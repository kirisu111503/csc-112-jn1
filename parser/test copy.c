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

// Symbol table structure
typedef struct vars
{
    char *id;
    int data_type;
    union
    {
        int val;       // for int/char
        char *str_val; // for string
    } data;
    int has_value; // Flag to indicate if variable has been assigned
    int reg_num;   // Register number assigned to this variable
    struct vars *next;
} vars;

// Error list structure
typedef struct errorList
{
    int line_error;
    char *error_type;
    char *line_content; // Store the problematic line
    struct errorList *next;
} errorList;

// History structure - stores all operations in order
typedef struct history
{
    int line_num;
    int operation_type; // OP_DECLARATION or OP_ASSIGNMENT
    char *variable_name;
    int data_type;         // TYPE_INT or TYPE_CHAR (for declarations)
    int has_value;         // Whether it has an initial/assigned value
    int value;             // The value (if has_value is true)
    char *source_variable; // NEW: for variable-to-variable assignments (e.g., y = x)
    char *original_line;   // Store the original line for reference
    struct history *next;
} history;

// Global symbol table, error list, and history
vars *symbol_table = NULL;
errorList *error_list_head = NULL;
history *history_head = NULL;
history *history_tail = NULL;
int next_register = 1; // Start from r1 (r0 is reserved for zero)

// Function prototypes
vars *find_variable(const char *id);
int add_variable(const char *id, int data_type, int line_num);
void set_variable_value(const char *id, int int_val, const char *str_val);
void add_error(int line_num, const char *error_type, const char *line_content);
void add_history_with_source(int line_num, int op_type, const char *var_name, int data_type, int has_val, int val, const char *source_var, const char *original_line);
void print_symbol_table();
void print_errors();
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

// --- NEW Function Prototypes for Expression Parser ---
int evaluate_expression(const char *expression_str, int line_num);
int parse_expression();
int parse_atom();

// Find a variable in the symbol table
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

// Add a new variable to the symbol table
// Returns 1 if successful, 0 if variable already exists
int add_variable(const char *id, int data_type, int line_num)
{
    // Check if variable already exists
    if (find_variable(id) != NULL)
    {
        add_error(line_num, ERROR_REDECLARATION, NULL);
        return 0;
    }

    // Create new variable node
    vars *new_var = (vars *)malloc(sizeof(vars));
    if (new_var == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return 0;
    }

    // Initialize all fields
    new_var->id = strdup(id);
    if (new_var->id == NULL)
    {
        free(new_var);
        return 0;
    }

    new_var->data_type = data_type;
    new_var->has_value = 0;
    new_var->data.val = 0;              // Initialize union to 0
    new_var->reg_num = next_register++; // Assign next available register
    new_var->next = symbol_table;
    symbol_table = new_var;

    return 1;
}

// Set the value of an existing variable
void set_variable_value(const char *id, int int_val, const char *str_val)
{
    vars *var = find_variable(id);
    if (var != NULL)
    {
        var->has_value = 1;
        if (var->data_type == TYPE_INT || var->data_type == TYPE_CHAR)
        {
            var->data.val = int_val;
        }
        else
        {
            // Only for string types (not used in current implementation)
            var->data.str_val = str_val ? strdup(str_val) : NULL;
        }
    }
}

// Add an error to the error list
void add_error(int line_num, const char *error_type, const char *line_content)
{
    errorList *new_error = (errorList *)malloc(sizeof(errorList));
    if (new_error == NULL)
    {
        fprintf(stderr, "Memory allocation failed for error\n");
        // We will not exit, just log the error
    }

    new_error->line_error = line_num;
    new_error->error_type = strdup(error_type);
    new_error->line_content = line_content ? strdup(line_content) : NULL;

    if (new_error->error_type == NULL)
    {
        if (new_error->line_content)
            free(new_error->line_content);
        free(new_error);
        fprintf(stderr, "Memory allocation failed for error\n");
    }

    new_error->next = error_list_head;
    error_list_head = new_error;

    // Display error immediately
    fprintf(stderr, "\n--- ERROR DETECTED ---\n");
    fprintf(stderr, "LINE %d: %s\n", new_error->line_error, new_error->error_type);
    if (new_error->line_content)
    {
        fprintf(stderr, "Content: %s\n", new_error->line_content);
    }
    fprintf(stderr, "----------------------\n");
}

// Add an entry to the history with source variable tracking
void add_history_with_source(int line_num, int op_type, const char *var_name, int data_type, int has_val, int val, const char *source_var, const char *original_line)
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
    new_entry->has_value = has_val;
    new_entry->value = val;
    new_entry->source_variable = source_var ? strdup(source_var) : NULL;
    new_entry->original_line = original_line ? strdup(original_line) : NULL;
    new_entry->next = NULL;

    // Add to the end of the list (to maintain order)
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

// Extract variable name from declaration (still used by assignment)
char *extract_variable_name(const char *declaration)
{
    if (declaration == NULL)
        return NULL;

    char *temp = strdup(declaration);
    if (temp == NULL)
        return NULL;

    char *token;
    char *name = NULL;

    // Remove leading/trailing whitespace and semicolon
    char *start = temp;
    while (isspace(*start))
        start++;

    // Tokenize by space
    token = strtok(start, " \t\n;=,");

    // Skip data type if present (int or char)
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

// Extract data type from declaration
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

// Extract value from declaration (if has assignment)
char *extract_value(const char *declaration)
{
    if (declaration == NULL)
        return NULL;

    const char *equals = strchr(declaration, '=');
    if (equals == NULL)
    {
        return NULL;
    }

    equals++; // Move past '='
    while (isspace(*equals))
        equals++;

    // Extract until semicolon or comma
    const char *end = equals;
    // This is simplified: it will grab everything until the end
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

    // Trim trailing whitespace
    char *trim_end = value + strlen(value) - 1;
    while (trim_end > value && isspace(*trim_end))
    {
        *trim_end = '\0';
        trim_end--;
    }

    return value;
}

// Check if a line is a declaration (has data type) or just an assignment
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

// --- NEW: Expression Parser ---
// This parser evaluates expressions like "5 + y" and returns the final integer value.

// Global helper for the parser
static const char *g_expr_ptr; // Points to the current character in the expression
static int g_line_num;         // The current line number for error reporting

// Parses a single "atom": either a number or a variable name
int parse_atom()
{
    // Skip whitespace
    while (isspace(*g_expr_ptr))
        g_expr_ptr++;

    const char *start = g_expr_ptr;

    // Check for a number
    if (isdigit(*start) || (*start == '-' && isdigit(start[1])))
    {
        g_expr_ptr++;
        while (isdigit(*g_expr_ptr))
            g_expr_ptr++;

        char num_str[32];
        strncpy(num_str, start, g_expr_ptr - start);
        num_str[g_expr_ptr - start] = '\0';
        return atoi(num_str);
    }

    // Check for a char literal (e.g., 'A')
    if (*start == '\'')
    {
        g_expr_ptr++; // Skip '
        int char_val = *g_expr_ptr;
        g_expr_ptr++; // Skip character
        g_expr_ptr++; // Skip '
        return char_val;
    }

    // Check for a variable name
    if (isalpha(*start) || *start == '_')
    {
        g_expr_ptr++;
        while (isalnum(*g_expr_ptr) || *g_expr_ptr == '_')
            g_expr_ptr++;

        char var_name[256];
        strncpy(var_name, start, g_expr_ptr - start);
        var_name[g_expr_ptr - start] = '\0';

        // Look up the variable in the symbol table
        vars *var = find_variable(var_name);
        if (var == NULL)
        {
            // Error: variable not found
            add_error(g_line_num, ERROR_UNDECLARED, var_name);
            return 0; // Return 0 on error
        }
        if (!var->has_value)
        {
            // Warning (or error): using uninitialized variable
            printf("  -> WARNING (Line %d): Using uninitialized variable '%s'\n", g_line_num, var_name);
            // In a real compiler, this might be an error or just return 0
        }
        return var->data.val;
    }

    // If we get here, it's not a valid atom
    add_error(g_line_num, ERROR_SYNTAX, g_expr_ptr);
    return 0;
}

// Parses an expression (e.g., "atom + atom + atom")
int parse_expression()
{
    // Get the first value
    int left_value = parse_atom();

    // Loop for '+' or '-' (you can add subtraction here)
    while (1)
    {
        // Skip whitespace
        while (isspace(*g_expr_ptr))
            g_expr_ptr++;

        char op = *g_expr_ptr;

        if (op == '+')
        {
            g_expr_ptr++; // "Consume" the '+'
            int right_value = parse_atom();
            left_value = left_value + right_value;
        }
        // Example: How to add subtraction
        // else if (op == '-')
        // {
        //     g_expr_ptr++; // "Consume" the '-'
        //     int right_value = parse_atom();
        //     left_value = left_value - right_value;
        // }
        else
        {
            // Not an operator we handle, so we're done
            break;
        }
    }

    return left_value;
}

// Main entry point for the expression parser
int evaluate_expression(const char *expression_str, int line_num)
{
    if (expression_str == NULL)
        return 0;

    // Trim leading/trailing whitespace from the whole expression string
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

    int result = parse_expression();

    // After parsing, check if we consumed the whole string.
    // If not, there's trailing garbage.
    while (isspace(*g_expr_ptr))
        g_expr_ptr++;

    if (*g_expr_ptr != '\0')
    {
        // We stopped parsing but the string isn't over. Syntax error.
        add_error(line_num, ERROR_SYNTAX, g_expr_ptr);
    }

    free(temp_expr);
    return result;
}

// --- REWRITTEN: process_declaration ---
// Process a variable declaration (with data type)
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

    // Find the start of the variable list (after the data type)
    char *var_list_start = line_copy + strlen(data_type_str);
    while (isspace(*var_list_start))
        var_list_start++;

    // Remove the trailing semicolon
    char *semicolon = strrchr(var_list_start, ';');
    if (semicolon)
        *semicolon = '\0';

    // Tokenize the variable list by comma
    char *var_decl = strtok(var_list_start, ",");

    while (var_decl != NULL)
    {
        // Each token is "x" or "y = 10"
        char *var_name = var_decl;
        char *value_str = NULL;
        int has_val = 0;
        int int_val = 0;

        // Check for an assignment
        char *equals = strchr(var_name, '=');
        if (equals)
        {
            *equals = '\0'; // Split the string at '='
            value_str = equals + 1;
            has_val = 1;
        }

        // Trim whitespace from variable name
        char *name_end = var_name + strlen(var_name) - 1;
        while (name_end > var_name && isspace(*name_end))
        {
            *name_end = '\0';
            name_end--;
        }
        while (isspace(*var_name))
            var_name++;

        // Add variable to symbol table
        if (add_variable(var_name, data_type, line_num))
        {
            printf("  -> Variable '%s' declared as %s\n", var_name, data_type_str);

            if (has_val)
            {
                // --- THIS IS THE FIX ---
                // Use the new expression parser instead of atoi()
                int_val = evaluate_expression(value_str, line_num);
                // --- END OF FIX ---

                set_variable_value(var_name, int_val, NULL);
                printf("  -> Variable '%s' initialized with value %d\n", var_name, int_val);
            }

            // Add to history (note: source_variable is lost with this simple parser)
            add_history_with_source(line_num, OP_DECLARATION, var_name, data_type, has_val, int_val, NULL, declaration);
        }

        // Get the next variable in the list (e.g., "y = 10")
        var_decl = strtok(NULL, ",");
    }

    free(line_copy);
    free(data_type_str);
}

// --- REWRITTEN: process_assignment ---
// Process a variable assignment (without data type - reassignment)
void process_assignment(const char *assignment, int line_num)
{
    if (assignment == NULL)
        return;

    char *var_name = extract_variable_name(assignment); // This should still work
    char *value = extract_value(assignment);            // This should still work

    if (var_name)
    {
        // Check if variable exists
        vars *var = find_variable(var_name);
        if (var == NULL)
        {
            add_error(line_num, ERROR_UNDECLARED, assignment);
            free(var_name);
            if (value)
                free(value);
            return;
        }

        printf("  -> Reassigning variable '%s'\n", var_name);

        int int_val = 0;

        // Update the value
        if (value)
        {
            // --- THIS IS THE FIX ---
            // Use the new expression parser instead of the old logic
            int_val = evaluate_expression(value, line_num);
            // --- END OF FIX ---

            set_variable_value(var_name, int_val, NULL);
            printf("  -> Variable '%s' updated with value %d\n", var_name, int_val);

            // Add to history (note: source_variable is lost with this simple parser)
            add_history_with_source(line_num, OP_ASSIGNMENT, var_name, var->data_type, 1, int_val, NULL, assignment);
        }
    }

    if (var_name)
        free(var_name);
    if (value)
        free(value);
}

// Print symbol table
void print_symbol_table()
{
    if (symbol_table == NULL)
    {
        printf("\n=== Symbol Table ===\n");
        printf("(empty)\n\n");
        return;
    }

    printf("\n=== Symbol Table ===\n");
    printf("%-15s %-10s %-10s %-15s\n", "Variable", "Type", "Register", "Value");
    printf("--------------------------------------------------------\n");

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

// Print error list
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

// Print history
void print_history()
{
    if (history_head == NULL)
    {
        printf("\n=== Operation History ===\n");
        printf("(empty)\n\n");
        return;
    }

    printf("\n=== Operation History (for MIPS64 Generation) ===\n");
    printf("%-6s %-15s %-12s %-10s %-20s %-15s\n", "Line", "Operation", "Variable", "Type", "Value", "Source Var");
    printf("--------------------------------------------------------------------------------------------\n");

    history *current = history_head;
    while (current != NULL)
    {
        printf("%-6d ", current->line_num);

        // Print operation type
        if (current->operation_type == OP_DECLARATION)
        {
            printf("%-15s ", "DECLARE");
        }
        else
        {
            printf("%-15s ", "ASSIGN");
        }

        // Print variable name
        printf("%-12s ", current->variable_name);

        // Print data type
        printf("%-10s ", current->data_type == TYPE_INT ? "int" : "char");

        // Print value
        if (current->has_value)
        {
            if (current->data_type == TYPE_INT)
            {
                printf("%-20d", current->value);
            }
            else
            {
                printf("'%c' (%-16d)", (char)current->value, current->value);
            }
        }
        else
        {
            printf("%-20s", "(uninitialized)");
        }

        // Print source variable
        if (current->source_variable)
        {
            printf("%-15s", current->source_variable);
        }
        else
        {
            printf("%-15s", "-");
        }

        printf("\n");
        current = current->next;
    }
    printf("\n");
}

// --- EDITED FOR EDUMIPS64 ---
// Generate MIPS64 code from history and write to output.txt
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
        // fprintf(output_file, "# No operations to generate\n"); // Removed comment
        fclose(output_file);
        return;
    }

    // --- DATA SECTION ---
    fprintf(output_file, ".data\n");

    // Walk symbol table for all declarations, not history
    vars *cur_var = symbol_table;
    while (cur_var)
    {
        if (cur_var->data_type == TYPE_INT)
        {
            fprintf(output_file, "%s: .space 8\n", cur_var->id);
        }
        else
        { // TYPE_CHAR
            fprintf(output_file, "%s: .space 1\n", cur_var->id);
        }
        cur_var = cur_var->next;
    }

    fprintf(output_file, "\n.text\n");
    // fprintf(output_file, ".globl main\n"); // Removed for EduMIPS64
    fprintf(output_file, "main:\n");

    // --- TEXT SECTION: use history to emit operations in order ---
    history *current = history_head;
    while (current)
    {
        vars *dst = find_variable(current->variable_name);
        if (!dst)
        {
            // fprintf(output_file, "    # ERROR: Variable '%s' not found in symbol table\n\n", // Removed comment
            //         current->variable_name);
            current = current->next;
            continue;
        }

        // fprintf(output_file, "    # Operation: %s %s (Line %d)\n", // Removed comment
        //     (current->operation_type == OP_DECLARATION) ? "DECLARE" : "ASSIGN",
        //     current->variable_name,
        //     current->line_num
        // );

        // Both DECLARATION with value and ASSIGNMENT are handled the same way:
        // Load an immediate value into a register and store it.
        if (current->has_value)
        {
            if (current->source_variable)
            {
                // This logic is now bypassed by the new parser, but we'll leave it.
                // A better parser would set this field.
                vars *src = find_variable(current->source_variable);
                if (src)
                {
                    // fprintf(output_file, "    dla r8, %s              \n", // Removed
                    //         src->id);

                    if (src->data_type == TYPE_INT)
                    {
                        fprintf(output_file, "    ld r%d, %s(r0)         \n", // Changed
                                dst->reg_num, src->id);
                    }
                    else
                    {                                                         // TYPE_CHAR
                        fprintf(output_file, "    lb r%d, %s(r0)         \n", // Changed
                                dst->reg_num, src->id);
                    }

                    // fprintf(output_file, "    dla r8, %s              \n", // Removed
                    //         dst->id);

                    if (dst->data_type == TYPE_INT)
                    {
                        fprintf(output_file, "    sd r%d, %s(r0)          \n", // Changed
                                dst->reg_num, dst->id);
                    }
                    else
                    {                                                          // TYPE_CHAR
                        fprintf(output_file, "    sb r%d, %s(r0)          \n", // Changed
                                dst->reg_num, dst->id);
                    }
                }
            }
            else
            {
                // Immediate value assignment (from constant folding)

                fprintf(output_file, "    daddiu r%d, r0, %d      \n",
                        dst->reg_num, current->value);
                // fprintf(output_file, "    dla r8, %s            \n", // Removed
                //         dst->id);

                if (dst->data_type == TYPE_INT)
                {
                    fprintf(output_file, "    sd r%d, %s(r0)          \n", // Changed
                            dst->reg_num, dst->id);
                }
                else
                {                                                        // TYPE_CHAR
                    fprintf(output_file, "    sb r%d, %s(r0)        \n", // Changed
                            dst->reg_num, dst->id);
                }
            }
        }
        else
        {
            // fprintf(output_file, "    # (no value assigned)\n"); // Removed comment
        }

        fprintf(output_file, "\n");
        current = current->next;
    }

    // --- Exit: removed for EduMIPS64 ---
    // fprintf(output_file, "    daddiu r31, r0, 10     \n");
    // fprintf(output_file, "    syscall\n");

    fclose(output_file);
    printf("\n=== MIPS64 Code Generated Successfully (EduMIPS64 compatible) ===\n");
}

// Free symbol table memory
void free_symbol_table()
{
    vars *current = symbol_table;
    while (current != NULL)
    {
        vars *temp = current;
        current = current->next;

        // Free the id string
        if (temp->id != NULL)
        {
            free(temp->id);
        }

        // Free the node itself
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
        if (temp->source_variable != NULL)
        {
            free(temp->source_variable);
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
        // I-type: opcode(6) | rs(5) | rt(5) | immediate(16)
        uint32_t opcode = (binary >> 26) & 0x3F;
        uint32_t rs = (binary >> 21) & 0x1F;
        uint32_t rt = (binary >> 16) & 0x1F;
        uint32_t imm = binary & 0xFFFF;

        // Print opcode (6 bits)
        for (int i = 5; i >= 0; i--)
        {
            output[idx++] = (opcode & (1 << i)) ? '1' : '0';
        }
        output[idx++] = ' ';

        // Print rs (5 bits)
        for (int i = 4; i >= 0; i--)
        {
            output[idx++] = (rs & (1 << i)) ? '1' : '0';
        }
        output[idx++] = ' ';

        // Print rt (5 bits)
        for (int i = 4; i >= 0; i--)
        {
            output[idx++] = (rt & (1 << i)) ? '1' : '0';
        }
        output[idx++] = ' ';

        // Print immediate (16 bits)
        for (int i = 15; i >= 0; i--)
        {
            output[idx++] = (imm & (1 << i)) ? '1' : '0';
        }
    }
    else if (strcmp(format_type, "R-type") == 0)
    {
        // R-type: opcode(6) | rs(5) | rt(5) | rd(5) | shamt(5) | funct(6)
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
        // Default: 8-bit groups
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

    printf("\n=== MIPS64 Code with Instruction Field Format ===\n");
    printf("+----+------------------------+-------------------------------------------+----------+\n");
    printf("| No | Instruction            | Binary Fields                             | Hex      |\n");
    printf("+----+------------------------+-------------------------------------------+----------+\n");

    while (fgets(line, sizeof(line), file))
    {
        line[strcspn(line, "\n")] = 0;

        if (line[0] == '\0' || line[0] == '#' || line[0] == '.' ||
            (strlen(line) > 0 && line[strlen(line) - 1] == ':'))
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

        // Handle load/store instructions (I-type)
        // --- EDITED FOR EDUMIPS64 ---
        // This parser now needs to understand "sd r1, x(r0)"
        if (strcmp(instr_only, "ld") == 0 || strcmp(instr_only, "sd") == 0 ||
            strcmp(instr_only, "lb") == 0 || strcmp(instr_only, "sb") == 0)
        {
            // Try parsing the EduMIPS64 format: "sd rt, label(rs)"
            // Note: For labels, sscanf is tricky. We'll parse manually.
            char *rt_str = strtok(trimmed + strlen(instr_only), " \t,");
            char *addr_str = strtok(NULL, ""); // Get the rest of the string

            if (rt_str && addr_str && strchr(addr_str, '('))
            {
                char *label_str = strtok(addr_str, "(");
                char *rs_str = strtok(NULL, ")");

                if (label_str && rs_str)
                {
                    int rt_num = get_register_number(rt_str);
                    int rs_num = get_register_number(rs_str);

                    // In our case, offset (the label) isn't an integer.
                    // The binhex converter can't resolve labels, so it will be 0
                    offset = 0;

                    if (strcmp(instr_only, "ld") == 0)
                    {
                        int opcode = 0x37;
                        binary = (opcode << 26) | (rs_num << 21) | (rt_num << 16) | (offset & 0xFFFF);
                        strcpy(format_type, "I-type");
                        parsed = 1;
                    }
                    else if (strcmp(instr_only, "sd") == 0)
                    {
                        int opcode = 0x3F;
                        binary = (opcode << 26) | (rs_num << 21) | (rt_num << 16) | (offset & 0xFFFF);
                        strcpy(format_type, "I-type");
                        parsed = 1;
                    }
                    else if (strcmp(instr_only, "lb") == 0)
                    {
                        int opcode = 0x20;
                        binary = (opcode << 26) | (rs_num << 21) | (rt_num << 16) | (offset & 0xFFFF);
                        strcpy(format_type, "I-type");
                        parsed = 1;
                    }
                    else if (strcmp(instr_only, "sb") == 0)
                    {
                        int opcode = 0x28;
                        binary = (opcode << 26) | (rs_num << 21) | (rt_num << 16) | (offset & 0xFFFF);
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
                char *rt_clean = rt;
                while (*rt_clean == ' ')
                    rt_clean++;
                char *rs_clean = rs;
                while (*rs_clean == ' ')
                    rs_clean++;

                int rt_num = get_register_number(rt_clean);
                int rs_num = get_register_number(rs_clean);

                int opcode = 0x19;
                binary = (opcode << 26) | (rs_num << 21) | (rt_num << 16) | (imm & 0xFFFF);
                strcpy(format_type, "I-type");
                parsed = 1;
            }
        }
        // dla (pseudo-instruction, treated as lui which is I-type)
        else if (strcmp(instr_only, "dla") == 0)
        {
            // This case should no longer be hit, but we leave it
            if (sscanf(trimmed, "%15s %7[^,], %63s", instr, rd, label) == 3)
            {
                char *rd_clean = rd;
                while (*rd_clean == ' ')
                    rd_clean++;
                int rd_num = get_register_number(rd_clean);

                int opcode = 0x0F;
                binary = (opcode << 26) | (rd_num << 16) | (0x1000);
                strcpy(format_type, "I-type");
                parsed = 1;
            }
        }
        // syscall (R-type)
        else if (strstr(trimmed, "syscall") != NULL)
        {
            binary = 0x0000000C;
            strcpy(format_type, "R-type");
            parsed = 1;
        }

        if (!parsed)
        {
            printf("| %-2d | %-22s | %-41s | %-8s |\n",
                   instr_count, trimmed, "UNKNOWN", "UNKNOWN");
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

    // Print field format legend
    printf("\n=== Instruction Format Legend ===\n");
    printf("I-type: opcode(6) | rs(5) | rt(5) | immediate(16)\n");
    printf("R-type: opcode(6) | rs(5) | rt(5) | rd(5) | shamt(5) | funct(6)\n");

    if (total_instrs > 0)
    {
        printf("\n=== Merged Binary (Execution Order) ===\n");
        for (int i = 0; i < total_instrs; i++)
        {
            for (int bit = 31; bit >= 0; bit--)
            {
                printf("%c", (instructions[i] & (1 << bit)) ? '1' : '0');
            }
        }
        printf("\n\n=== Merged Hexadecimal ===\n");
        for (int i = 0; i < total_instrs; i++)
        {
            printf("%08X", instructions[i]);
        }
        printf("\n");
    }

    fclose(file);
}

int main()
{
    const char *pattern[] = {
        "^(([[:space:]]*(int|char)[[:space:]]+[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]*)(=[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]')([[:space:]]*(\\+|\\-|\\*|\\/)[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]'))*)?[[:space:]]*;[[:space:]]*)+$",
        "^[[:space:]]*((int|char)[[:space:]]+)?[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]*((=[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]')+([[:space:]]*)*)?)*(=[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]')([[:space:]]*(\\+|\\-|\\*|\\/)[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]'))*)?[[:space:]]*(,[[:space:]]*[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]*(=[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]')([[:space:]]*(\\+|\\-|\\*|\\/)[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]'))*)?[[:space:]]*)*;[[:space:]]*$",
    };

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

    printf("=== Processing Input File ===\n");

    // Read file line by line
    while (fgets(line, sizeof(line), file) != NULL)
    {
        // Remove trailing newline
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
        }

        // Skip empty lines
        if (strlen(line) == 0)
        {
            line_num++;
            continue;
        }

        printf("Line %d: %s\n", line_num, line);

        int matched = 0;

        // Try matching with both patterns
        for (int p = 0; p < 2; p++)
        {
            if (regcomp(&regex, pattern[p], REG_EXTENDED) == 0)
            {
                if (regexec(&regex, line, 0, NULL, 0) == 0)
                {
                    matched = 1;
                    printf("  -> Valid syntax, processing...\n");

                    // Check if it's a declaration or assignment
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
            printf("  -> Syntax error detected\n");
            add_error(line_num, ERROR_SYNTAX, line);
        }

        line_num++;
    }

    fclose(file);

    // Display results
    print_symbol_table();
    print_history();

    // Generate MIPS64 code only if no errors
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