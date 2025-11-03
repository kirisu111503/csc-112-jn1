// CSC 112 - C to MIPS64 Compiler
// Enhanced version with full compilation pipeline
// BOQUILON && ISIDERIO

#include <stdio.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_VARS 100
#define MAX_LINES 100
#define MAX_TOKENS 500
#define MAX_INSTR 1000

// ==================== DATA STRUCTURES ====================

typedef enum
{
    TOK_INT,
    TOK_CHAR,
    TOK_IDENT,
    TOK_NUMBER,
    TOK_CHAR_LIT,
    TOK_ASSIGN,
    TOK_PLUS,
    TOK_MINUS,
    TOK_MUL,
    TOK_DIV,
    TOK_SEMICOLON,
    TOK_COMMA,
    TOK_EOF
} TokenType;

typedef struct
{
    TokenType type;
    char lexeme[64];
    int value;
    int line_num;
} Token;

typedef struct
{
    char name[64];
    char type[16];
    int address;
    int defined;
} Symbol;

typedef struct
{
    char asm_code[128];
    unsigned int machine_code;
    int is_comment;
} Instruction;

// Global variables
Token tokens[MAX_TOKENS];
int token_count = 0;
Symbol symbol_table[MAX_VARS];
int symbol_count = 0;
int next_address = 0;
Instruction instructions[MAX_INSTR];
int instr_count = 0;
int has_error = 0;

// ==================== REGEX PATTERNS ====================

// Pattern for variable declaration with optional initialization
const char *decl_pattern =
    "^[[:space:]]*(int|char)[[:space:]]+[a-zA-Z_][a-zA-Z0-9_]*"
    "([[:space:]]*=[[:space:]]*[^;]+)?([[:space:]]*,[[:space:]]*[a-zA-Z_][a-zA-Z0-9_]*"
    "([[:space:]]*=[[:space:]]*[^;]+)?)*[[:space:]]*;[[:space:]]*$";

// Pattern for assignment
const char *assign_pattern =
    "^[[:space:]]*[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]*=[[:space:]]*[^;]+[[:space:]]*;[[:space:]]*$";

// ==================== UTILITY FUNCTIONS ====================

void trim(char *str)
{
    char *start = str;
    while (isspace(*start))
        start++;
    if (start != str)
        memmove(str, start, strlen(start) + 1);

    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end))
        end--;
    *(end + 1) = '\0';
}

int is_keyword(const char *word)
{
    return strcmp(word, "int") == 0 || strcmp(word, "char") == 0;
}

// ==================== SYMBOL TABLE ====================

int find_symbol(const char *name)
{
    for (int i = 0; i < symbol_count; i++)
    {
        if (strcmp(symbol_table[i].name, name) == 0)
            return i;
    }
    return -1;
}

void add_symbol(const char *name, const char *type)
{
    int idx = find_symbol(name);
    if (idx >= 0)
    {
        symbol_table[idx].defined = 1;
        return;
    }

    if (symbol_count >= MAX_VARS)
    {
        fprintf(stderr, "Error: Symbol table full\n");
        has_error = 1;
        return;
    }

    Symbol *s = &symbol_table[symbol_count++];
    strncpy(s->name, name, sizeof(s->name) - 1);
    strncpy(s->type, type, sizeof(s->type) - 1);
    s->address = next_address;
    s->defined = 1;

    if (strcmp(type, "char") == 0)
        next_address += 1;
    else
        next_address += 8; // 64-bit word
}

// ==================== TOKENIZER ====================

void add_token(TokenType type, const char *lexeme, int value, int line_num)
{
    if (token_count >= MAX_TOKENS)
        return;

    Token *t = &tokens[token_count++];
    t->type = type;
    t->value = value;
    t->line_num = line_num;
    strncpy(t->lexeme, lexeme, sizeof(t->lexeme) - 1);
    t->lexeme[sizeof(t->lexeme) - 1] = '\0';
}

void tokenize_line(const char *line, int line_num)
{
    const char *p = line;

    while (*p)
    {
        // Skip whitespace
        while (isspace(*p))
            p++;
        if (*p == '\0')
            break;

        // Keywords or identifiers
        if (isalpha(*p) || *p == '_')
        {
            char buf[64];
            int i = 0;
            while ((isalnum(*p) || *p == '_') && i < 63)
            {
                buf[i++] = *p++;
            }
            buf[i] = '\0';

            if (strcmp(buf, "int") == 0)
                add_token(TOK_INT, buf, 0, line_num);
            else if (strcmp(buf, "char") == 0)
                add_token(TOK_CHAR, buf, 0, line_num);
            else
                add_token(TOK_IDENT, buf, 0, line_num);
            continue;
        }

        // Numbers
        if (isdigit(*p))
        {
            char buf[64];
            int i = 0, val = 0;
            while (isdigit(*p) && i < 63)
            {
                buf[i++] = *p;
                val = val * 10 + (*p++ - '0');
            }
            buf[i] = '\0';
            add_token(TOK_NUMBER, buf, val, line_num);
            continue;
        }

        // Character literals
        if (*p == '\'')
        {
            if (*(p + 1) && *(p + 2) == '\'')
            {
                char buf[4] = {'\'', *(p + 1), '\'', '\0'};
                add_token(TOK_CHAR_LIT, buf, (int)*(p + 1), line_num);
                p += 3;
                continue;
            }
        }

        // Operators and punctuation
        switch (*p)
        {
        case '=':
            add_token(TOK_ASSIGN, "=", 0, line_num);
            break;
        case '+':
            add_token(TOK_PLUS, "+", 0, line_num);
            break;
        case '-':
            add_token(TOK_MINUS, "-", 0, line_num);
            break;
        case '*':
            add_token(TOK_MUL, "*", 0, line_num);
            break;
        case '/':
            add_token(TOK_DIV, "/", 0, line_num);
            break;
        case ';':
            add_token(TOK_SEMICOLON, ";", 0, line_num);
            break;
        case ',':
            add_token(TOK_COMMA, ",", 0, line_num);
            break;
        }
        p++;
    }
}

// ==================== CODE GENERATION ====================

unsigned int encode_r_type(unsigned int opcode, unsigned int rs, unsigned int rt,
                           unsigned int rd, unsigned int shamt, unsigned int funct)
{
    return ((opcode & 0x3F) << 26) | ((rs & 0x1F) << 21) | ((rt & 0x1F) << 16) |
           ((rd & 0x1F) << 11) | ((shamt & 0x1F) << 6) | (funct & 0x3F);
}

unsigned int encode_i_type(unsigned int opcode, unsigned int rs,
                           unsigned int rt, unsigned short imm)
{
    return ((opcode & 0x3F) << 26) | ((rs & 0x1F) << 21) |
           ((rt & 0x1F) << 16) | (imm & 0xFFFF);
}

void emit(const char *asm_str, unsigned int code, int is_comment)
{
    if (instr_count >= MAX_INSTR)
        return;

    Instruction *i = &instructions[instr_count++];
    strncpy(i->asm_code, asm_str, sizeof(i->asm_code) - 1);
    i->asm_code[sizeof(i->asm_code) - 1] = '\0';
    i->machine_code = code;
    i->is_comment = is_comment;
}

void emit_daddiu(int rt, int rs, int imm)
{
    char buf[128];
    sprintf(buf, "DADDIU R%d, R%d, %d", rt, rs, imm);
    emit(buf, encode_i_type(0x19, rs, rt, (unsigned short)(imm & 0xFFFF)), 0);
}

void emit_daddu(int rd, int rs, int rt)
{
    char buf[128];
    sprintf(buf, "DADDU R%d, R%d, R%d", rd, rs, rt);
    emit(buf, encode_r_type(0, rs, rt, rd, 0, 0x2D), 0);
}

void emit_dsubu(int rd, int rs, int rt)
{
    char buf[128];
    sprintf(buf, "DSUBU R%d, R%d, R%d", rd, rs, rt);
    emit(buf, encode_r_type(0, rs, rt, rd, 0, 0x2F), 0);
}

void emit_dmultu(int rs, int rt)
{
    char buf[128];
    sprintf(buf, "DMULTU R%d, R%d", rs, rt);
    emit(buf, encode_r_type(0, rs, rt, 0, 0, 0x1C), 0);
}

void emit_ddivu(int rs, int rt)
{
    char buf[128];
    sprintf(buf, "DDIVU R%d, R%d", rs, rt);
    emit(buf, encode_r_type(0, rs, rt, 0, 0, 0x1F), 0);
}

void emit_mflo(int rd)
{
    char buf[128];
    sprintf(buf, "MFLO R%d", rd);
    emit(buf, encode_r_type(0, 0, 0, rd, 0, 0x12), 0);
}

void emit_ld(int rt, const char *label)
{
    char buf[128];
    int idx = find_symbol(label);
    int offset = (idx >= 0) ? symbol_table[idx].address : 0;
    sprintf(buf, "LD R%d, %s(R0)", rt, label);
    emit(buf, encode_i_type(0x37, 0, rt, (unsigned short)offset), 0);
}

void emit_sd(int rt, const char *label)
{
    char buf[128];
    int idx = find_symbol(label);
    int offset = (idx >= 0) ? symbol_table[idx].address : 0;
    sprintf(buf, "SD R%d, %s(R0)", rt, label);
    emit(buf, encode_i_type(0x3F, 0, rt, (unsigned short)offset), 0);
}

void emit_syscall()
{
    emit("SYSCALL 0", encode_r_type(0, 0, 0, 0, 0, 0x0C), 0);
}

void emit_comment(const char *comment)
{
    emit(comment, 0, 1);
}

// ==================== EXPRESSION PARSER ====================

typedef struct ExprToken
{
    enum
    {
        EXPR_NUM,
        EXPR_VAR,
        EXPR_OP
    } type;
    char text[64];
    int value;
} ExprToken;

int parse_expression(const char *expr, int target_reg)
{
    char expr_copy[256];
    strncpy(expr_copy, expr, sizeof(expr_copy) - 1);
    trim(expr_copy);

    // Simple expression parser (handles basic arithmetic)
    char *tokens_str[50];
    int tok_count = 0;
    char *p = strtok(expr_copy, " ");
    while (p && tok_count < 50)
    {
        tokens_str[tok_count++] = p;
        p = strtok(NULL, " ");
    }

    if (tok_count == 0)
        return target_reg;

    // Single token (number or variable)
    if (tok_count == 1)
    {
        if (isdigit(tokens_str[0][0]))
        {
            emit_daddiu(target_reg, 0, atoi(tokens_str[0]));
        }
        else if (tokens_str[0][0] == '\'')
        {
            emit_daddiu(target_reg, 0, (int)tokens_str[0][1]);
        }
        else
        {
            emit_ld(target_reg, tokens_str[0]);
        }
        return target_reg;
    }

    // Expression with operators (simple left-to-right evaluation)
    // Load first operand
    if (isdigit(tokens_str[0][0]))
    {
        emit_daddiu(target_reg, 0, atoi(tokens_str[0]));
    }
    else if (tokens_str[0][0] == '\'')
    {
        emit_daddiu(target_reg, 0, (int)tokens_str[0][1]);
    }
    else
    {
        emit_ld(target_reg, tokens_str[0]);
    }

    // Process operator-operand pairs
    for (int i = 1; i < tok_count; i += 2)
    {
        if (i + 1 >= tok_count)
            break;

        char *op = tokens_str[i];
        char *operand = tokens_str[i + 1];
        int next_reg = target_reg + 1;

        // Load next operand
        if (isdigit(operand[0]))
        {
            emit_daddiu(next_reg, 0, atoi(operand));
        }
        else if (operand[0] == '\'')
        {
            emit_daddiu(next_reg, 0, (int)operand[1]);
        }
        else
        {
            emit_ld(next_reg, operand);
        }

        // Apply operator
        if (strcmp(op, "+") == 0)
        {
            emit_daddu(target_reg, target_reg, next_reg);
        }
        else if (strcmp(op, "-") == 0)
        {
            emit_dsubu(target_reg, target_reg, next_reg);
        }
        else if (strcmp(op, "*") == 0)
        {
            emit_dmultu(target_reg, next_reg);
            emit_mflo(target_reg);
        }
        else if (strcmp(op, "/") == 0)
        {
            emit_ddivu(target_reg, next_reg);
            emit_mflo(target_reg);
        }
    }

    return target_reg;
}

// ==================== STATEMENT PROCESSOR ====================

void process_declaration(const char *line, int line_num)
{
    char line_copy[256];
    strncpy(line_copy, line, sizeof(line_copy) - 1);

    // Extract type
    char *type = strtok(line_copy, " \t");
    if (!type)
        return;

    // Process each variable
    char *rest = strtok(NULL, ";");
    if (!rest)
        return;

    char *var_decl = strtok(rest, ",");
    while (var_decl)
    {
        trim(var_decl);

        // Split on '='
        char *eq = strchr(var_decl, '=');
        if (eq)
        {
            *eq = '\0';
            char *var_name = var_decl;
            char *expr = eq + 1;
            trim(var_name);
            trim(expr);

            // Add to symbol table
            add_symbol(var_name, type);

            // Generate code
            char comment[128];

            int reg = parse_expression(expr, 1);
            emit_sd(reg, var_name);
        }
        else
        {
            trim(var_decl);
            add_symbol(var_decl, type);

            char comment[128];
        }

        var_decl = strtok(NULL, ",");
    }
}

void process_assignment(const char *line, int line_num)
{
    char line_copy[256];
    strncpy(line_copy, line, sizeof(line_copy) - 1);

    char *eq = strchr(line_copy, '=');
    if (!eq)
        return;

    *eq = '\0';
    char *var_name = line_copy;
    char *expr = eq + 1;
    trim(var_name);
    trim(expr);

    // Remove semicolon
    char *semi = strchr(expr, ';');
    if (semi)
        *semi = '\0';
    trim(expr);

    // Check if variable exists
    if (find_symbol(var_name) < 0)
    {
        fprintf(stderr, "Error line %d: Variable '%s' not declared\n", line_num, var_name);
        has_error = 1;
        return;
    }

    // Generate code
    char comment[128];

    int reg = parse_expression(expr, 1);
    emit_sd(reg, var_name);
}

// ==================== MAIN FUNCTION ====================

void print_binary(unsigned int v, char *out, size_t len)
{
    int pos = 0;
    for (int i = 31; i >= 0; i--)
    {
        if (pos >= len - 1)
            break;
        out[pos++] = ((v >> i) & 1) ? '1' : '0';
        if (i % 4 == 0 && i > 0 && pos < len - 1)
        {
            out[pos++] = ' ';
        }
    }
    out[pos] = '\0';
}

int main()
{
    FILE *file;
    char line[256];
    char source[4096] = "";
    int line_num = 1;

    file = fopen("input.txt", "r");
    if (file == NULL)
    {
        printf("Error: Can't open input.txt\n");
        return 1;
    }

    // Compile regex patterns
    regex_t decl_regex, assign_regex;
    if (regcomp(&decl_regex, decl_pattern, REG_EXTENDED) != 0)
    {
        printf("Error: Invalid declaration pattern\n");
        fclose(file);
        return 1;
    }
    if (regcomp(&assign_regex, assign_pattern, REG_EXTENDED) != 0)
    {
        printf("Error: Invalid assignment pattern\n");
        regfree(&decl_regex);
        fclose(file);
        return 1;
    }

    printf("======================================\n");
    printf("LEXICAL ANALYSIS (REGEX-BASED)\n");
    printf("======================================\n");

    // Phase 1: Lexical Analysis
    while (fgets(line, sizeof(line), file))
    {
        line[strcspn(line, "\n")] = '\0';
        strncat(source, line, sizeof(source) - strlen(source) - 2);
        strcat(source, "\n");

        if (strlen(line) == 0 || line[0] == '\n')
        {
            line_num++;
            continue;
        }

        int matched = 0;

        // Check declaration pattern
        if (regexec(&decl_regex, line, 0, NULL, 0) == 0)
        {
            printf("Line %d: Declaration - Valid\n", line_num);
            tokenize_line(line, line_num);
            matched = 1;
        }
        // Check assignment pattern
        else if (regexec(&assign_regex, line, 0, NULL, 0) == 0)
        {
            printf("Line %d: Assignment - Valid\n", line_num);
            tokenize_line(line, line_num);
            matched = 1;
        }

        if (!matched)
        {
            printf("Line %d: ERROR - Invalid syntax\n", line_num);
            has_error = 1;
        }

        line_num++;
    }

    fclose(file);
    regfree(&decl_regex);
    regfree(&assign_regex);

    if (has_error)
    {
        printf("\n======================================\n");
        printf("COMPILATION FAILED\n");
        printf("======================================\n");
        return 1;
    }

    // Phase 2: Parse and generate code
    file = fopen("input.txt", "r");
    line_num = 1;

    while (fgets(line, sizeof(line), file))
    {
        line[strcspn(line, "\n")] = '\0';

        if (strlen(line) == 0)
        {
            line_num++;
            continue;
        }

        trim(line);

        // Check if declaration or assignment
        if (strstr(line, "int ") == line || strstr(line, "char ") == line)
        {
            process_declaration(line, line_num);
        }
        else if (strchr(line, '=') != NULL)
        {
            process_assignment(line, line_num);
        }

        line_num++;
    }

    fclose(file);
    emit_syscall();

    // Phase 3: Output
    printf("\n======================================\n");
    printf("SOURCE CODE\n");
    printf("======================================\n");
    printf("%s\n", source);

    printf("\n======================================\n");
    printf("ASSEMBLY CODE\n");
    printf("======================================\n");
    printf(".data\n");
    for (int i = 0; i < symbol_count; i++)
    {
        printf("%-15s %s 0\n",
               symbol_table[i].name,
               strcmp(symbol_table[i].type, "char") == 0 ? ".byte" : ".word64");
    }

    printf("\n.code\n");
    for (int i = 0; i < instr_count; i++)
    {
        printf("    %s\n", instructions[i].asm_code);
    }

    printf("\n======================================\n");
    printf("MACHINE CODE\n");
    printf("======================================\n");
    printf("%-20s %-40s\n", "(HEX)", "(BINARY)");
    printf("--------------------------------------------------------------\n");
    for (int i = 0; i < instr_count; i++)
    {
        if (instructions[i].is_comment)
        {
            printf("%-60s %s\n", "", instructions[i].asm_code);
        }
        else
        {
            char bin[128];
            print_binary(instructions[i].machine_code, bin, sizeof(bin));
            printf("0x%08X          %s\n", instructions[i].machine_code, bin);
        }
    }

    return 0;
}