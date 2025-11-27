#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>
#include <stdint.h>

// Data type constants
#define TYPE_INT 1
#define TYPE_CHAR 2

// Operation type constants
#define OP_DECLARATION 1
#define OP_ASSIGNMENT 2

// Error type constants
#define ERROR_REDECLARATION "Variable redeclaration"
#define ERROR_SYNTAX "Syntax error"
#define ERROR_UNDECLARED "Undeclared variable"

// --- Abstract Syntax Tree (AST) Nodes ---
typedef enum
{
    NODE_NUMBER,
    NODE_VARIABLE,
    NODE_BINARY_OP
} NodeType;

// The structure for our AST
typedef struct AstNode
{
    NodeType type;
    union
    {
        // For NODE_NUMBER
        int value;

        // For NODE_VARIABLE
        char *var_name;

        // For NODE_BINARY_OP
        struct
        {
            char op;
            struct AstNode *left;
            struct AstNode *right;
        } op_details;
    };
} AstNode;

// Symbol table structure
typedef struct vars
{
    char *id;
    int data_type;
    union
    {
        int val;
        char *str_val;
    } data;
    int has_value;
    int reg_num; //*destination* register
    struct vars *next;
} vars;

// Error list structure
typedef struct errorList
{
    int line_error;
    char *error_type;
    char *line_content;
    struct errorList *next;
} errorList;

// --- History Structure ---
typedef struct history
{
    int line_num;
    int operation_type;
    char *variable_name;
    int data_type;

    // stores blueprint of the expression
    AstNode *expression_tree;

    char *original_line;
    struct history *next;
} history;

// global symbol table, error list, and history
vars *symbol_table = NULL;
errorList *error_list_head = NULL;
history *history_head = NULL;
history *history_tail = NULL;
int next_register = 1;      // start from r1 (r0 is reserved)
int next_temp_register = 8; // start using r8 for temp calculations

// --- Function Prototypes for AST ---
AstNode *create_number_node(int value);
AstNode *create_variable_node(char *var_name);
AstNode *create_binary_op_node(char op, AstNode *left, AstNode *right);
void free_ast(AstNode *node);
int generate_mips_for_ast(FILE *output_file, AstNode *node);

// Function prototypes
vars *find_variable(const char *id);
int add_variable(const char *id, int data_type, int line_num);
void set_variable_value_in_table(const char *id, int int_val);
void add_error(int line_num, const char *error_type, const char *line_content);
void add_history_entry(int line_num, int op_type, const char *var_name, int data_type, AstNode *tree, const char *original_line);
void print_symbol_table();
void print_errors();
void print_history_ast(AstNode *node);
void print_history();
void generate_mips64();
void free_symbol_table();
void free_error_list();
void free_history();
char *extract_variable_name(const char *declaration);
char *extract_data_type(const char *declaration);
char *extract_value(const char *declaration);
int is_declaration(const char *line);
void process_declaration(const char *declaration, int line_num);
void process_assignment(const char *assignment, int line_num);
int get_register_number(char *reg);
void convert_mips64_to_binhex(char *filename);

// --- PEMDAS-COMPLIANT PARSER PROTOTYPES ---
AstNode *parse_expression_to_ast(const char *expression_str, int line_num);
AstNode *parse_expression(); // Handles + and -
AstNode *parse_term();       // Handles * and /
AstNode *parse_atom();       // Handles numbers, vars, (and parentheses)

// --- AST Helper Functions ---

// create a simple number node
AstNode *create_number_node(int value)
{
    AstNode *node = (AstNode *)malloc(sizeof(AstNode));
    node->type = NODE_NUMBER;
    node->value = value;
    return node;
}

// create node for a variable name
AstNode *create_variable_node(char *var_name)
{
    AstNode *node = (AstNode *)malloc(sizeof(AstNode));
    node->type = NODE_VARIABLE;
    node->var_name = strdup(var_name);
    return node;
}

// create binary operation node (the "blueprint")
AstNode *create_binary_op_node(char op, AstNode *left, AstNode *right)
{
    AstNode *node = (AstNode *)malloc(sizeof(AstNode));
    node->type = NODE_BINARY_OP;
    node->op_details.op = op;
    node->op_details.left = left;
    node->op_details.right = right;
    return node;
}

// recursively frees all memory associated with an AST
void free_ast(AstNode *node)
{
    if (!node)
        return;

    if (node->type == NODE_BINARY_OP)
    {
        free_ast(node->op_details.left);
        free_ast(node->op_details.right);
    }
    else if (node->type == NODE_VARIABLE)
    {
        free(node->var_name);
    }
    free(node);
}

// --- End of AST Helper Functions ---

// find a variable in the symbol table
vars *find_variable(const char *id)
{
    vars *current = symbol_table;
    while (current != NULL)
    {
        if (strcmp(current->id, id) == 0)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// add a new variable to the symbol table
int add_variable(const char *id, int data_type, int line_num)
{
    if (find_variable(id) != NULL)
    {
        add_error(line_num, ERROR_REDECLARATION, NULL);
        return 0;
    }
    vars *new_var = (vars *)malloc(sizeof(vars));
    if (new_var == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return 0;
    }
    new_var->id = strdup(id);
    if (new_var->id == NULL)
    {
        free(new_var);
        return 0;
    }
    new_var->data_type = data_type;
    new_var->has_value = 0;
    new_var->data.val = 0;
    new_var->reg_num = next_register++;
    new_var->next = symbol_table;
    symbol_table = new_var;
    return 1;
}

// set the value of an existing variable
// NOTE: This is only for the *compiler's* tracking (constant folding).
void set_variable_value_in_table(const char *id, int int_val)
{
    vars *var = find_variable(id);
    if (var != NULL)
    {
        var->has_value = 1;
        if (var->data_type == TYPE_INT || var->data_type == TYPE_CHAR)
        {
            var->data.val = int_val;
        }
    }
}

// add an error to the error list
void add_error(int line_num, const char *error_type, const char *line_content)
{
    errorList *new_error = (errorList *)malloc(sizeof(errorList));
    if (new_error == NULL)
    {
        fprintf(stderr, "Memory allocation failed for error\n");
    }
    new_error->line_error = line_num;
    new_error->error_type = strdup(error_type);
    new_error->line_content = line_content ? strdup(line_content) : NULL;
    new_error->next = error_list_head;
    error_list_head = new_error;
    fprintf(stderr, "\n--- ERROR DETECTED ---\n");
    fprintf(stderr, "LINE %d: %s\n", new_error->line_error, new_error->error_type);
    if (new_error->line_content)
    {
        fprintf(stderr, "Content: %s\n", new_error->line_content);
    }
    fprintf(stderr, "----------------------\n");
}

// --- History Entry ---
void add_history_entry(int line_num, int op_type, const char *var_name, int data_type, AstNode *tree, const char *original_line)
{
    history *new_entry = (history *)malloc(sizeof(history));
    if (new_entry == NULL)
    {
        fprintf(stderr, "Memory allocation failed for history\n");
        return;
    }
    new_entry->line_num = line_num;
    new_entry->operation_type = op_type;
    new_entry->variable_name = strdup(var_name);
    new_entry->data_type = data_type;
    new_entry->expression_tree = tree; // Store the AST
    new_entry->original_line = original_line ? strdup(original_line) : NULL;
    new_entry->next = NULL;

    if (history_tail == NULL)
    {
        history_head = new_entry;
        history_tail = new_entry;
    }
    else
    {
        history_tail->next = new_entry;
        history_tail = new_entry;
    }
}

//(Helper functions for string extraction
char *extract_variable_name(const char *declaration)
{
    if (declaration == NULL)
        return NULL;
    char *temp = strdup(declaration);
    if (temp == NULL)
        return NULL;
    char *token;
    char *name = NULL;
    char *start = temp;
    while (isspace(*start))
        start++;
    token = strtok(start, " \t\n;=,");
    if (token && (strcmp(token, "int") == 0 || strcmp(token, "char") == 0))
    {
        token = strtok(NULL, " \t\n;=,");
    }
    if (token)
    {
        name = strdup(token);
    }
    free(temp);
    return name;
}

char *extract_data_type(const char *declaration)
{
    if (declaration == NULL)
        return NULL;
    char *temp = strdup(declaration);
    if (temp == NULL)
        return NULL;
    char *token;
    char *type = NULL;
    char *start = temp;
    while (isspace(*start))
        start++;
    token = strtok(start, " \t\n");
    if (token && (strcmp(token, "int") == 0 || strcmp(token, "char") == 0))
    {
        type = strdup(token);
    }
    free(temp);
    return type;
}

char *extract_value(const char *declaration)
{
    if (declaration == NULL)
        return NULL;
    const char *equals = strchr(declaration, '=');
    if (equals == NULL)
        return NULL;
    equals++;
    while (isspace(*equals))
        equals++;
    const char *end = equals;
    while (*end && *end != ';')
        end++;
    size_t len = end - equals;
    if (len == 0)
        return NULL;
    char *value = (char *)malloc(len + 1);
    if (value == NULL)
        return NULL;
    strncpy(value, equals, len);
    value[len] = '\0';
    char *trim_end = value + strlen(value) - 1;
    while (trim_end > value && isspace(*trim_end))
    {
        *trim_end = '\0';
        trim_end--;
    }
    return value;
}

int is_declaration(const char *line)
{
    if (line == NULL)
        return 0;
    char *temp = strdup(line);
    if (temp == NULL)
        return 0;
    char *start = temp;
    while (isspace(*start))
        start++;
    char *token = strtok(start, " \t\n");
    int result = 0;
    if (token && (strcmp(token, "int") == 0 || strcmp(token, "char") == 0))
    {
        result = 1;
    }
    free(temp);
    return result;
}

// --- PEMDAS-COMPLIANT PARSER (NOW BUILDS AN AST) ---

// Global helper for the parser
static const char *g_expr_ptr; // points to the current character in the expression
static int g_line_num;         // current line number for error reporting

// parses an "atom": number, variable, or (parentheses)
AstNode *parse_atom()
{
    while (isspace(*g_expr_ptr))
        g_expr_ptr++;
    const char *start = g_expr_ptr;

    // atom: Number (e.g., 5, -10)
    if (isdigit(*start) || (*start == '-' && isdigit(start[1])))
    {
        g_expr_ptr++;
        while (isdigit(*g_expr_ptr))
            g_expr_ptr++;
        char num_str[32];
        strncpy(num_str, start, g_expr_ptr - start);
        num_str[g_expr_ptr - start] = '\0';
        return create_number_node(atoi(num_str));
    }

    // atom: Char (e.g., 'A')
    if (*start == '\'')
    {
        g_expr_ptr++;
        int char_val = *g_expr_ptr;
        g_expr_ptr++;
        g_expr_ptr++; // Skip closing '
        return create_number_node(char_val);
    }

    // atom: Variable (e.g., y)
    if (isalpha(*start) || *start == '_')
    {
        g_expr_ptr++;
        while (isalnum(*g_expr_ptr) || *g_expr_ptr == '_')
            g_expr_ptr++;
        char var_name[256];
        strncpy(var_name, start, g_expr_ptr - start);
        var_name[g_expr_ptr - start] = '\0';

        if (find_variable(var_name) == NULL)
        {
            add_error(g_line_num, ERROR_UNDECLARED, var_name);
            return create_number_node(0); // Return 0 on error
        }
        return create_variable_node(var_name);
    }

    // atom: Parentheses (e.g., (5 + y))
    if (*start == '(')
    {
        g_expr_ptr++;                       // Consume '('
        AstNode *node = parse_expression(); // Recursively call the top-level parser
        if (*g_expr_ptr != ')')
        {
            add_error(g_line_num, ERROR_SYNTAX, "Missing ')'");
        }
        g_expr_ptr++; // Consume ')'
        return node;
    }

    add_error(g_line_num, ERROR_SYNTAX, g_expr_ptr);
    return create_number_node(0);
}

// handles * and / high Precedence)
AstNode *parse_term()
{
    AstNode *left_node = parse_atom();

    while (1)
    {
        while (isspace(*g_expr_ptr))
            g_expr_ptr++;
        char op = *g_expr_ptr;

        if (op == '*' || op == '/')
        {
            g_expr_ptr++;
            AstNode *right_node = parse_atom();
            left_node = create_binary_op_node(op, left_node, right_node);
        }
        else
        {
            break;
        }
    }
    return left_node;
}

// handles + and - lower Precedence
AstNode *parse_expression()
{
    AstNode *left_node = parse_term();

    while (1)
    {
        while (isspace(*g_expr_ptr))
            g_expr_ptr++;
        char op = *g_expr_ptr;

        if (op == '+' || op == '-')
        {
            g_expr_ptr++;
            AstNode *right_node = parse_term();
            left_node = create_binary_op_node(op, left_node, right_node);
        }
        else
        {
            break;
        }
    }
    return left_node;
}

// main entry point for the expression parser
AstNode *parse_expression_to_ast(const char *expression_str, int line_num)
{
    if (expression_str == NULL)
        return NULL;

    char *temp_expr = strdup(expression_str);
    char *start = temp_expr;
    while (isspace(*start))
        start++;
    char *end = start + strlen(start) - 1;
    while (end > start && isspace(*end))
    {
        *end = '\0';
        end--;
    }

    g_expr_ptr = start;
    g_line_num = line_num;
    AstNode *tree = parse_expression();

    while (isspace(*g_expr_ptr))
        g_expr_ptr++;
    if (*g_expr_ptr != '\0')
    {
        add_error(line_num, ERROR_SYNTAX, g_expr_ptr);
    }

    free(temp_expr);
    return tree;
}

// ---  process_declaration ---
void process_declaration(const char *declaration, int line_num)
{
    if (declaration == NULL)
        return;

    char *line_copy = strdup(declaration);
    char *data_type_str = extract_data_type(line_copy);
    if (!data_type_str)
    {
        add_error(line_num, ERROR_SYNTAX, "Missing data type");
        free(line_copy);
        return;
    }
    int data_type = (strcmp(data_type_str, "int") == 0) ? TYPE_INT : TYPE_CHAR;
    char *var_list_start = line_copy + strlen(data_type_str);
    while (isspace(*var_list_start))
        var_list_start++;
    char *semicolon = strrchr(var_list_start, ';');
    if (semicolon)
        *semicolon = '\0';
    char *var_decl = strtok(var_list_start, ",");

    while (var_decl != NULL)
    {
        char *var_name = var_decl;
        char *value_str = NULL;
        AstNode *tree = NULL;

        char *equals = strchr(var_name, '=');
        if (equals)
        {
            *equals = '\0';
            value_str = equals + 1;
        }

        char *name_end = var_name + strlen(var_name) - 1;
        while (name_end > var_name && isspace(*name_end))
        {
            *name_end = '\0';
            name_end--;
        }
        while (isspace(*var_name))
            var_name++;

        if (add_variable(var_name, data_type, line_num))
        {
            // printf("  -> Variable '%s' declared as %s\n", var_name, data_type_str);
            if (value_str)
            {
                // Parse the expression into an AST
                tree = parse_expression_to_ast(value_str, line_num);
                // printf("  -> Parsed expression for '%s'\n", var_name);
            }
            add_history_entry(line_num, OP_DECLARATION, var_name, data_type, tree, declaration);
        }
        var_decl = strtok(NULL, ",");
    }
    free(line_copy);
    free(data_type_str);
}

// --- process_assignment ---
void process_assignment(const char *assignment, int line_num)
{
    if (assignment == NULL)
        return;
    char *var_name = extract_variable_name(assignment);
    char *value = extract_value(assignment);
    AstNode *tree = NULL;

    if (var_name)
    {
        vars *var = find_variable(var_name);
        if (var == NULL)
        {
            add_error(line_num, ERROR_UNDECLARED, assignment);
            free(var_name);
            if (value)
                free(value);
            return;
        }
        // printf("  -> Reassigning variable '%s'\n", var_name);
        if (value)
        {
            // Parse the expression into an AST
            tree = parse_expression_to_ast(value, line_num);
            // printf("  -> Parsed expression for '%s'\n", var_name);
        }
        add_history_entry(line_num, OP_ASSIGNMENT, var_name, var->data_type, tree, assignment);
    }
    if (var_name)
        free(var_name);
    if (value)
        free(value);
}

// PRINT FUnctIONS
void print_symbol_table()
{
    if (symbol_table == NULL)
    {
        printf("\n=== Symbol Table ===\n(empty)\n\n");
        return;
    }
    printf("\n=== Symbol Table ===\n");
    printf("%-15s %-10s %-10s %-15s\n", "Variable", "Type", "Register", "Value (Known by C)");
    printf("------------------------------------------------------------\n");
    vars *current = symbol_table;
    while (current != NULL)
    {
        printf("%-15s ", current->id);
        printf("%-10s ", current->data_type == TYPE_INT ? "int" : "char");
        printf("$%-9d ", current->reg_num);
        if (current->has_value)
        {
            if (current->data_type == TYPE_INT)
            {
                printf("%-15d", current->data.val);
            }
            else if (current->data_type == TYPE_CHAR)
            {
                printf("'%c' (%d)", (char)current->data.val, current->data.val);
            }
        }
        else
        {
            printf("%-15s", "(uninitialized)");
        }
        printf("\n");
        current = current->next;
    }
    printf("\n");
}

void print_errors()
{
    if (error_list_head == NULL)
    {
        printf("\n=== No Errors Found ===\n\n");
        return;
    }
    printf("\n=== Error List ===\n");
    printf("%-10s %-30s %s\n", "Line", "Error Type", "Details");
    printf("-------------------------------------------------------------------------\n");
    errorList *current = error_list_head;
    while (current != NULL)
    {
        printf("%-10d %-30s", current->line_error, current->error_type);
        if (current->line_content)
        {
            printf(" %s", current->line_content);
        }
        printf("\n");
        current = current->next;
    }
    printf("\n");
}

// Helper to print the AST for debugging
void print_history_ast(AstNode *node)
{
    if (!node)
    {
        printf("(uninitialized)");
        return;
    }
    switch (node->type)
    {
    case NODE_NUMBER:
        printf("%d", node->value);
        break;
    case NODE_VARIABLE:
        printf("%s", node->var_name);
        break;
    case NODE_BINARY_OP:
        printf("(");
        print_history_ast(node->op_details.left);
        printf(" %c ", node->op_details.op);
        print_history_ast(node->op_details.right);
        printf(")");
        break;
    }
}

void print_history()
{
    if (history_head == NULL)
    {
        printf("\n=== Operation History ===\n(empty)\n\n");
        return;
    }
    printf("\n=== Operation History (AST View) ===\n");
    printf("%-6s %-15s %-12s %-10s %-20s\n", "Line", "Operation", "Variable", "Type", "Expression Blueprint");
    printf("--------------------------------------------------------------------------\n");
    history *current = history_head;
    while (current != NULL)
    {
        printf("%-6d ", current->line_num);
        printf("%-15s ", (current->operation_type == OP_DECLARATION) ? "DECLARE" : "ASSIGN");
        printf("%-12s ", current->variable_name);
        printf("%-10s ", current->data_type == TYPE_INT ? "int" : "char");
        print_history_ast(current->expression_tree);
        printf("\n");
        current = current->next;
    }
    printf("\n");
}

// ---  generate_mips64 ---

// recursive function walks the AST and generates MIPS code THEN returns the temporary register number that holds the final result.
int generate_mips_for_ast(FILE *output_file, AstNode *node)
{
    if (!node)
        return 0; // Should not happen (error)

    int reg_num;
    switch (node->type)
    {
    case NODE_NUMBER:
        // Load an immediate value into a new temporary register
        reg_num = next_temp_register++;
        fprintf(output_file, "    daddiu r%d, r0, %d\n", reg_num, node->value);
        return reg_num;

    case NODE_VARIABLE:
        // Load the variable's value from memory into a new temporary register
        reg_num = next_temp_register++;
        vars *var = find_variable(node->var_name);
        if (var->data_type == TYPE_INT)
        {
            fprintf(output_file, "    ld r%d, %s(r0)\n", reg_num, var->id);
        }
        else
        {
            fprintf(output_file, "    lb r%d, %s(r0)\n", reg_num, var->id);
        }
        return reg_num;

    case NODE_BINARY_OP:
    {
        // 1. generate code for the left side
        int left_reg = generate_mips_for_ast(output_file, node->op_details.left);
        // 2. generate code for the right side
        int right_reg = generate_mips_for_ast(output_file, node->op_details.right);

        // 3. `left_reg` now holds the result of the left side.
        //    `right_reg` holds the result of the right side.
        //    re-use `left_reg` for the final result.

        switch (node->op_details.op)
        {
        case '+':
            // DADDU rd, rs, rt (rd = rs + rt)
            fprintf(output_file, "    daddu r%d, r%d, r%d\n", left_reg, left_reg, right_reg);
            break;
        case '-':
            // DSUBU rd, rs, rt (rd = rs - rt)
            fprintf(output_file, "    dsubu r%d, r%d, r%d\n", left_reg, left_reg, right_reg);
            break;
        case '*':
            // use HI/LO registers for dmul
            fprintf(output_file, "    dmult r%d, r%d\n", left_reg, right_reg);
            fprintf(output_file, "    mflo r%d\n", left_reg); // Move result from LO to left_reg
            break;
        case '/':
            // Use  HI/LO registers for ddiv
            fprintf(output_file, "    ddiv r%d, r%d\n", left_reg, right_reg);
            fprintf(output_file, "    mflo r%d\n", left_reg); // Move quotient from LO to left_reg
            break;
        }

        // result is now in left_reg.  free right_reg for later use.
        next_temp_register--; // Frees right_reg
        return left_reg;      // Return the register that holds the result
    }
    }
    return 0;
}

void generate_mips64()
{
    FILE *output_file = fopen("output.txt", "w");
    if (!output_file)
    {
        printf("Error: Could not create output.txt file\n");
        return;
    }
    if (!history_head)
    {
        fclose(output_file);
        return;
    }

    printf("\n=== Generate Assembly Code === \n");

    fprintf(output_file, ".data\n");
    printf(".data\n");

    vars *cur_var = symbol_table;
    while (cur_var)
    {
        if (cur_var->data_type == TYPE_INT)
        {
            fprintf(output_file, "%s: .space 8\n", cur_var->id);
            printf("%s: .space 8\n", cur_var->id);
        }
        else
        {
            fprintf(output_file, "%s: .space 1\n", cur_var->id);
            printf("%s: .space 1\n", cur_var->id);
        }
        cur_var = cur_var->next;
    }

    fprintf(output_file, "\n.text\n");
    printf("\n.text\n");
    fprintf(output_file, "main:\n");
    printf("main:\n");

    history *current = history_head;
    while (current)
    {
        vars *dst = find_variable(current->variable_name);
        if (!dst)
        {
            current = current->next;
            continue;
        }

        // Only generate code if there is an expression
        if (current->expression_tree)
        {
            // Reset the temporary register counter for each new statement
            next_temp_register = 8;

            // Generate all the MIPS for the expression
            // The final result will be in the register returned by this call
            int final_result_reg = generate_mips_for_ast(output_file, current->expression_tree);

            // store final result from the temp reg into the variable's memory
            if (dst->data_type == TYPE_INT)
            {
                fprintf(output_file, "    sd r%d, %s(r0)\n", final_result_reg, dst->id);
                printf("    sd r%d, %s(r0)\n", final_result_reg, dst->id);
            }
            else
            {
                fprintf(output_file, "    sb r%d, %s(r0)\n", final_result_reg, dst->id);
                printf("    sb r%d, %s(r0)\n", final_result_reg, dst->id);
            }
        }
        fprintf(output_file, "\n");
        current = current->next;
    }

    // --- Exit ---
    // fprintf(output_file, "    daddiu r31, r0, 10     \n");
    fprintf(output_file, "    syscall 0\n");

    fclose(output_file);
}

// Free symbol table memory
void free_symbol_table()
{
    vars *current = symbol_table;
    while (current != NULL)
    {
        vars *temp = current;
        current = current->next;
        if (temp->id != NULL)
        {
            free(temp->id);
        }
        free(temp);
    }
    symbol_table = NULL;
}

// Free error list memory
void free_error_list()
{
    errorList *current = error_list_head;
    while (current != NULL)
    {
        errorList *temp = current;
        current = current->next;
        if (temp->error_type != NULL)
        {
            free(temp->error_type);
        }
        if (temp->line_content != NULL)
        {
            free(temp->line_content);
        }
        free(temp);
    }
    error_list_head = NULL;
}

// Free history memory
void free_history()
{
    history *current = history_head;
    while (current != NULL)
    {
        history *temp = current;
        current = current->next;

        if (temp->variable_name != NULL)
        {
            free(temp->variable_name);
        }
        // --- NEW: Free the AST ---
        if (temp->expression_tree != NULL)
        {
            free_ast(temp->expression_tree);
        }
        if (temp->original_line != NULL)
        {
            free(temp->original_line);
        }
        free(temp);
    }
    history_head = NULL;
    history_tail = NULL;
}

// (binhex converter functions are unchanged)
int get_register_number(char *reg)
{
    if (reg[0] == 'r' || reg[0] == 'R')
        return atoi(reg + 1);
    return -1;
}

void print_binary_fields(uint32_t binary, const char *format_type)
{
    char output[100];
    int idx = 0;

    if (strcmp(format_type, "I-type") == 0)
    {
        uint32_t opcode = (binary >> 26) & 0x3F;
        uint32_t rs = (binary >> 21) & 0x1F;
        uint32_t rt = (binary >> 16) & 0x1F;
        uint32_t imm = binary & 0xFFFF;
        for (int i = 5; i >= 0; i--)
        {
            output[idx++] = (opcode & (1 << i)) ? '1' : '0';
        }
        output[idx++] = ' ';
        for (int i = 4; i >= 0; i--)
        {
            output[idx++] = (rs & (1 << i)) ? '1' : '0';
        }
        output[idx++] = ' ';
        for (int i = 4; i >= 0; i--)
        {
            output[idx++] = (rt & (1 << i)) ? '1' : '0';
        }
        output[idx++] = ' ';
        for (int i = 15; i >= 0; i--)
        {
            output[idx++] = (imm & (1 << i)) ? '1' : '0';
        }
    }
    else if (strcmp(format_type, "R-type") == 0)
    {
        uint32_t opcode = (binary >> 26) & 0x3F;
        uint32_t rs = (binary >> 21) & 0x1F;
        uint32_t rt = (binary >> 16) & 0x1F;
        uint32_t rd = (binary >> 11) & 0x1F;
        uint32_t shamt = (binary >> 6) & 0x1F;
        uint32_t funct = binary & 0x3F;
        for (int i = 5; i >= 0; i--)
            output[idx++] = (opcode & (1 << i)) ? '1' : '0';
        output[idx++] = ' ';
        for (int i = 4; i >= 0; i--)
            output[idx++] = (rs & (1 << i)) ? '1' : '0';
        output[idx++] = ' ';
        for (int i = 4; i >= 0; i--)
            output[idx++] = (rt & (1 << i)) ? '1' : '0';
        output[idx++] = ' ';
        for (int i = 4; i >= 0; i--)
            output[idx++] = (rd & (1 << i)) ? '1' : '0';
        output[idx++] = ' ';
        for (int i = 4; i >= 0; i--)
            output[idx++] = (shamt & (1 << i)) ? '1' : '0';
        output[idx++] = ' ';
        for (int i = 5; i >= 0; i--)
            output[idx++] = (funct & (1 << i)) ? '1' : '0';
    }
    else
    {
        for (int i = 31; i >= 0; i--)
        {
            output[idx++] = (binary & (1 << i)) ? '1' : '0';
            if (i % 8 == 0 && i != 0)
            {
                output[idx++] = ' ';
            }
        }
    }
    output[idx] = '\0';
    printf("%s", output);
}

void convert_mips64_to_binhex(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        printf("Error: Cannot open %s\n", filename);
        return;
    }
    char line[256];
    int instr_count = 1;
    uint32_t instructions[1024];
    char formats[1024][16];
    int total_instrs = 0;
    printf("\n\n=== MIPS64 Code with Instruction Field Format ===\n");
    printf("+----+------------------------+-------------------------------------------+----------+\n");
    printf("| No | Instruction            | Binary Fields                             | Hex      |\n");
    printf("+----+------------------------+-------------------------------------------+----------+\n");
    while (fgets(line, sizeof(line), file))
    {
        line[strcspn(line, "\n")] = 0;
        if (line[0] == '\0' || line[0] == '#' || line[0] == '.' || (strlen(line) > 0 && line[strlen(line) - 1] == ':'))
            continue;
        char *trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t')
            trimmed++;
        if (*trimmed == '\0')
            continue;
        if (strstr(trimmed, ".word") != NULL || strstr(trimmed, ":") != NULL)
        {
            continue;
        }
        char instr[16], rd[8], rs[8], rt[8], label[64];
        int imm, offset;
        uint32_t binary = 0;
        int parsed = 0;
        char format_type[16] = "default";
        char instr_only[16];
        sscanf(trimmed, "%15s", instr_only);

        if (strcmp(instr_only, "ld") == 0 || strcmp(instr_only, "sd") == 0 ||
            strcmp(instr_only, "lb") == 0 || strcmp(instr_only, "sb") == 0)
        {
            char *rt_str = strtok(trimmed + strlen(instr_only), " \t,");
            char *addr_str = strtok(NULL, "");
            if (rt_str && addr_str && strchr(addr_str, '('))
            {
                char *label_str = strtok(addr_str, "(");
                char *rs_str = strtok(NULL, ")");
                if (label_str && rs_str)
                {
                    int rt_num = get_register_number(rt_str);
                    int rs_num = get_register_number(rs_str);
                    offset = 0;
                    if (strcmp(instr_only, "ld") == 0)
                    {
                        binary = (0x37 << 26) | (rs_num << 21) | (rt_num << 16) | (offset & 0xFFFF);
                        strcpy(format_type, "I-type");
                        parsed = 1;
                    }
                    else if (strcmp(instr_only, "sd") == 0)
                    {
                        binary = (0x3F << 26) | (rs_num << 21) | (rt_num << 16) | (offset & 0xFFFF);
                        strcpy(format_type, "I-type");
                        parsed = 1;
                    }
                    else if (strcmp(instr_only, "lb") == 0)
                    {
                        binary = (0x20 << 26) | (rs_num << 21) | (rt_num << 16) | (offset & 0xFFFF);
                        strcpy(format_type, "I-type");
                        parsed = 1;
                    }
                    else if (strcmp(instr_only, "sb") == 0)
                    {
                        binary = (0x28 << 26) | (rs_num << 21) | (rt_num << 16) | (offset & 0xFFFF);
                        strcpy(format_type, "I-type");
                        parsed = 1;
                    }
                }
            }
        }
        // daddiu (I-type)
        else if (strcmp(instr_only, "daddiu") == 0)
        {
            if (sscanf(trimmed, "%15s %7[^,], %7[^,], %d", instr, rt, rs, &imm) == 4)
            {
                int rt_num = get_register_number(rt);
                int rs_num = get_register_number(rs);
                binary = (0x19 << 26) | (rs_num << 21) | (rt_num << 16) | (imm & 0xFFFF);
                strcpy(format_type, "I-type");
                parsed = 1;
            }
        }
        else if (strcmp(instr_only, "daddu") == 0 || strcmp(instr_only, "dsubu") == 0)
        {
            if (sscanf(trimmed, "%15s %7[^,], %7[^,], %7s", instr, rd, rs, rt) == 4)
            {
                int rd_num = get_register_number(rd);
                int rs_num = get_register_number(rs);
                int rt_num = get_register_number(rt);
                int funct = (strcmp(instr_only, "daddu") == 0) ? 0x2D : 0x2F;
                binary = (0x00 << 26) | (rs_num << 21) | (rt_num << 16) | (rd_num << 11) | (0x00 << 6) | funct;
                strcpy(format_type, "R-type");
                parsed = 1;
            }
        }
        else if (strcmp(instr_only, "dmult") == 0 || strcmp(instr_only, "ddiv") == 0)
        {
            if (sscanf(trimmed, "%15s %7[^,], %7s", instr, rs, rt) == 3)
            {
                char *rs_clean = rs;
                while (*rs_clean == ' ')
                    rs_clean++;
                char *rt_clean = rt;
                while (*rt_clean == ' ')
                    rt_clean++;

                int rs_num = get_register_number(rs_clean);
                int rt_num = get_register_number(rt_clean);

                int funct = (strcmp(instr_only, "dmult") == 0) ? 0x1C : 0x1E;
                binary = (0x00 << 26) | (rs_num << 21) | (rt_num << 16) | (0x00 << 11) | (0x00 << 6) | funct;
                strcpy(format_type, "R-type");
                parsed = 1;
            }
        }
        else if (strcmp(instr_only, "mflo") == 0)
        {
            if (sscanf(trimmed, "%15s %7s", instr, rd) == 2)
            {
                char *rd_clean = rd;
                while (*rd_clean == ' ')
                    rd_clean++;

                int rd_num = get_register_number(rd_clean);
                int funct = 0x12; // MFLO
                binary = (0x00 << 26) | (0x00 << 21) | (0x00 << 16) | (rd_num << 11) | (0x00 << 6) | funct;
                strcpy(format_type, "R-type");
                parsed = 1;
            }
        }
        else if (strstr(trimmed, "syscall") != NULL)
        {
            binary = 0x0000000C;
            strcpy(format_type, "R-type");
            parsed = 1;
        }

        if (!parsed)
        {
            printf("| %-2d | %-22s | %-41s | %-8s |\n", instr_count, trimmed, "UNKNOWN", "UNKNOWN");
            instr_count++;
            continue;
        }
        printf("| %-2d | %-22s | ", instr_count, trimmed);
        print_binary_fields(binary, format_type);
        printf(" | 0x%08X |\n", binary);
        instructions[total_instrs] = binary;
        strcpy(formats[total_instrs], format_type);
        total_instrs++;
        instr_count++;
    }
    printf("+----+------------------------+-------------------------------------------+----------+\n");

    fclose(file);
}

int main()
{
    const char *pattern[] = {
        "^[[:space:]]*(int|char)[[:space:]]+[a-zA-Z_][a-zA-Z0-9_]*([[:space:]]*,[[:space:]]*[a-zA-Z_][a-zA-Z0-9_]*)*([[:space:]]*=[^;]+)?[[:space:]]*;[[:space:]]*$",
        "^[[:space:]]*[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]*=[^;]+;[[:space:]]*$"};

    // Open input.txt file
    FILE *file = fopen("input.txt", "r");
    if (file == NULL)
    {
        fprintf(stderr, "Error: Could not open input.txt\n");
        return 1;
    }

    char line[1024];
    int line_num = 1;
    regex_t regex;

    // printf("=== Processing Input File ===\n");

    // Read file line by line
    while (fgets(line, sizeof(line), file) != NULL)
    {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
        }
        if (strlen(line) == 0)
        {
            line_num++;
            continue;
        }
        // printf("Line %d: %s\n", line_num, line);
        int matched = 0;

        // try match with both patterns
        for (int p = 0; p < 2; p++)
        {
            if (regcomp(&regex, pattern[p], REG_EXTENDED) == 0)
            {
                if (regexec(&regex, line, 0, NULL, 0) == 0)
                {
                    matched = 1;
                    // printf("  -> Valid syntax, processing...\n");
                    if (is_declaration(line))
                    {
                        process_declaration(line, line_num);
                    }
                    else
                    {
                        process_assignment(line, line_num);
                    }
                }
                regfree(&regex);
                if (matched)
                    break;
            }
        }
        if (!matched)
        {
            // fallback for simple declarations that the complex regex might miss
            if (regcomp(&regex, "^[[:space:]]*(int|char)[[:space:]]+[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]*;[[:space:]]*$", REG_EXTENDED) == 0)
            {
                if (regexec(&regex, line, 0, NULL, 0) == 0)
                {
                    matched = 1;
                    // printf("  -> Valid syntax (simple decl), processing...\n");
                    process_declaration(line, line_num);
                }
                regfree(&regex);
            }
        }
        if (!matched)
        {
            // printf("  -> Syntax error detected\n");
            add_error(line_num, ERROR_SYNTAX, line);
        }
        line_num++;
    }

    fclose(file);

    // display results
    print_symbol_table();
    print_history();

    // generate MIPS64 code only if no errors
    if (error_list_head == NULL)
    {
        generate_mips64();
        convert_mips64_to_binhex("output.txt");
    }

    print_errors();

    // Cleanup
    free_symbol_table();
    free_error_list();
    free_history();

    return 0;
}