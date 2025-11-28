#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>

typedef struct symbols
{
    char data_type;
    char *id;
    char *value;
    int line_declared;
    struct symbols *next;
} symbols;

typedef struct error
{
    int line;
    char *error_type;
    char *err_msg;
    struct error *next;
} error;

typedef struct history
{
    int line;
    char data_type; // 'i' for int, 'c' for char, '\0' for assignment
    char *variable;
    char *value;
    char *action; // "declare", "assign", "declare_assign"
    int is_valid; // 1 if valid, 0 if error
    struct history *next;
} history;

// Global Initialization
struct error *list_error_head = NULL;
struct error *list_error_tail = NULL;
struct symbols *list_var_head = NULL;
struct symbols *list_var_tail = NULL;
struct history *list_hist_head = NULL;
struct history *list_hist_tail = NULL;

// Forward declaration
void parse_single_statement(char *input, int line);

// Somewhat CFG but it's not. But still do the work :)
int is_valid_syntax(const char *line)
{
    if (!line || strlen(line) == 0)
        return 1; // Empty line is valid

    // Patterns for allowed syntax
    const char *pattern[] = {
        "([[:space:]]*(;)*)*",
        // Declaration: int x; int x = 5; int x, y = x + 10;
        "^([[:space:]]*(;)*)*(([[:space:]]*(int|char)[[:space:]]+[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]*)(=[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]')([[:space:]]*(\\+|\\-|\\*|\\/)[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]'))*)?[[:space:]]*;([[:space:]]*(;)*)*)*$",
        // Assignment or complex declaration: x = 5 + y; int a = x * 2, b;
        "^([[:space:]]*(;)*)*[[:space:]]*((int|char)[[:space:]]+)?[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]*((=[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]')+([[:space:]]*)*)?)*(=[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]')([[:space:]]*(\\+|\\-|\\*|\\/)[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]'))*)?(,[[:space:]]*[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]*(=[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]')([[:space:]]*(\\+|\\-|\\*|\\/)[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]'))*)?[[:space:]]*)*;([[:space:]]*(;)*)*$",
        // Pure expression: 2 + 3 * 4;
        "^([[:space:]]*(;)*)*[[:space:]]*[^;']+;([[:space:]]*(;)*)*$",
        NULL // Sentinel
    };

    // Count patterns (using sentinel)
    int count = 0;
    while (pattern[count] != NULL)
        count++;

    // Check each pattern
    regex_t regex;
    int valid = 0;

    for (int i = 0; i < count; i++)
    {
        // Compile
        if (regcomp(&regex, pattern[i], REG_EXTENDED | REG_NOSUB) != 0)
        {
            fprintf(stderr, "Regex compilation failed for pattern %d\n", i);
            continue; // Skip bad pattern
        }

        // Execute
        if (regexec(&regex, line, 0, NULL, 0) == 0)
        {
            valid = 1; // Match found
        }

        // Always free regex
        regfree(&regex);

        if (valid)
            break; // No need to check more patterns
    }

    return valid;
}

// Helper function to trim whitespace
char *trim(char *str)
{
    char *end;
    while (isspace((unsigned char)*str))
        str++;
    if (*str == 0)
        return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;
    end[1] = '\0';
    return str;
}

// Add entry to history (records everything chronologically)
void add_to_history(int line, char data_type, const char *variable, const char *value, const char *action, int is_valid)
{
    history *new_hist = malloc(sizeof(history));
    if (!new_hist)
    {
        fprintf(stderr, "Failed to allocate memory for history\n");
        return;
    }

    new_hist->line = line;
    new_hist->data_type = data_type;
    new_hist->variable = strdup(variable);
    new_hist->value = value ? strdup(value) : strdup("NULL");
    new_hist->action = strdup(action);
    new_hist->is_valid = is_valid;
    new_hist->next = NULL;

    if (!list_hist_head)
    {
        list_hist_head = list_hist_tail = new_hist;
    }
    else
    {
        list_hist_tail->next = new_hist;
        list_hist_tail = new_hist;
    }
}

// Check if variable was declared in history (for validation)
history *find_in_history(const char *variable)
{
    history *curr = list_hist_head;
    history *last_valid = NULL;

    while (curr)
    {
        if (strcmp(curr->variable, variable) == 0 && curr->is_valid)
        {
            // Keep track of the most recent valid entry
            last_valid = curr;
        }
        curr = curr->next;
    }

    return last_valid;
}

// Check if variable is already declared (for redeclaration check)
int is_variable_declared_in_history(const char *variable)
{
    history *curr = list_hist_head;

    while (curr)
    {
        if (strcmp(curr->variable, variable) == 0 &&
            (strcmp(curr->action, "declare") == 0 || strcmp(curr->action, "declare_assign") == 0) &&
            curr->is_valid)
        {
            return 1; // Already declared
        }
        curr = curr->next;
    }

    return 0;
}

// Helper function to check if variable exists in symbol table
symbols *find_variable(const char *id)
{
    symbols *curr = list_var_head;
    while (curr)
    {
        if (strcmp(curr->id, id) == 0)
            return curr;
        curr = curr->next;
    }
    return NULL;
}

// Helper function to check if a string is a valid identifier (variable name)
int is_identifier(const char *str)
{
    if (!str || strlen(str) == 0)
        return 0;

    // Must start with letter or underscore
    if (!isalpha(str[0]) && str[0] != '_')
        return 0;

    // Rest must be alphanumeric or underscore
    for (int i = 1; str[i]; i++)
    {
        if (!isalnum(str[i]) && str[i] != '_')
            return 0;
    }

    return 1;
}

// Helper function to check if a string is a number
int is_number(const char *str)
{
    if (!str || strlen(str) == 0)
        return 0;

    for (int i = 0; str[i]; i++)
    {
        if (!isdigit(str[i]))
            return 0;
    }
    return 1;
}

// Helper function to check if a string is a character literal
int is_char_literal(const char *str)
{
    if (!str || strlen(str) < 3)
        return 0;

    return (str[0] == '\'' && str[strlen(str) - 1] == '\'');
}

// Extract all identifiers from an expression (e.g., "x + y * 2" -> ["x", "y"])
void extract_identifiers(const char *expr, char identifiers[][64], int *count)
{
    *count = 0;
    char *expr_copy = strdup(expr);
    char *token;
    char delimiters[] = " \t\n+-*/=(),;";

    token = strtok(expr_copy, delimiters);
    while (token && *count < 100)
    {
        token = trim(token);

        // Only add if it's an identifier (not a number or char literal)
        if (is_identifier(token) && !is_number(token) && !is_char_literal(token))
        {
            // Check for duplicates
            int duplicate = 0;
            for (int i = 0; i < *count; i++)
            {
                if (strcmp(identifiers[i], token) == 0)
                {
                    duplicate = 1;
                    break;
                }
            }

            if (!duplicate)
            {
                strncpy(identifiers[*count], token, 63);
                identifiers[*count][63] = '\0';
                (*count)++;
            }
        }

        token = strtok(NULL, delimiters);
    }

    free(expr_copy);
}

// Add VALID variable to symbol table only
void add_to_symbol_table(char data_type, const char *id, const char *value, int line)
{
    symbols *existing = find_variable(id);

    if (existing)
    {
        // Update existing variable value only
        if (value)
        {
            free(existing->value);
            existing->value = strdup(value);
        }
    }
    else
    {
        // Create new variable
        symbols *new_var = malloc(sizeof(symbols));
        if (!new_var)
        {
            fprintf(stderr, "Failed to allocate memory for symbol\n");
            return;
        }

        new_var->data_type = data_type;
        new_var->id = strdup(id);
        new_var->value = value ? strdup(value) : strdup("NULL");
        new_var->line_declared = line;
        new_var->next = NULL;

        if (!list_var_head)
        {
            list_var_head = list_var_tail = new_var;
        }
        else
        {
            list_var_tail->next = new_var;
            list_var_tail = new_var;
        }
    }
}

// Create error log
void create_error_log(char *error_type, char *error_msgs, int line)
{
    error *new_error = malloc(sizeof(error));
    if (!new_error)
    {
        printf("Failed to create error NODE.\n");
        return;
    }
    new_error->line = line;
    new_error->error_type = strdup(error_type);
    new_error->err_msg = strdup(error_msgs);
    new_error->next = NULL;

    if (!list_error_head)
    {
        list_error_head = list_error_tail = new_error;
    }
    else
    {
        list_error_tail->next = new_error;
        list_error_tail = new_error;
    }
}

// Helper function to parse a single statement
void parse_single_statement(char *input, int line)
{
    if (!input || strlen(input) == 0)
        return;

    // Check for data type declaration
    char data_type = '\0';
    char *start = input;

    if (strncmp(input, "int ", 4) == 0)
    {
        data_type = 'i';
        start = input + 4;
    }
    else if (strncmp(input, "char ", 5) == 0)
    {
        data_type = 'c';
        start = input + 5;
    }

    // Track variables declared in THIS statement (for same-line references)
    char current_stmt_vars[100][64];
    int current_stmt_count = 0;

    // Parse variable declarations/assignments separated by commas
    char *start_copy = strdup(start);
    char *saveptr;
    char *token = strtok_r(start_copy, ",", &saveptr);
    int var_position = 0; // Track position of variable in declaration

    while (token)
    {
        token = trim(token);
        var_position++;

        // Check for assignment operator
        char *eq = strchr(token, '=');

        if (eq)
        {
            // Split into variable name and value
            *eq = '\0';
            char *var_name = trim(token);
            char *value = trim(eq + 1);
            int is_valid = 1;

            // RULE 1: All variables used in initialization must be declared
            // For first variable: can only use previously declared variables
            // For subsequent variables: can use previous vars + vars declared earlier in this statement
            if (data_type != '\0')
            {
                char identifiers[100][64];
                int id_count;
                extract_identifiers(value, identifiers, &id_count);

                for (int i = 0; i < id_count; i++)
                {
                    int found_in_history = find_in_history(identifiers[i]) != NULL;

                    // Check if variable was declared earlier in this statement
                    int found_in_current_stmt = 0;
                    for (int j = 0; j < current_stmt_count; j++)
                    {
                        if (strcmp(current_stmt_vars[j], identifiers[i]) == 0)
                        {
                            found_in_current_stmt = 1;
                            break;
                        }
                    }

                    if (!found_in_history && !found_in_current_stmt)
                    {
                        char err_msg[256];
                        snprintf(err_msg, sizeof(err_msg),
                                 "Variable '%s' used in initialization of '%s' is not declared",
                                 identifiers[i], var_name);
                        create_error_log("Undeclared Variable", err_msg, line);
                        is_valid = 0;
                    }
                }
            }

            // RULE 2: Check for redeclaration
            if (data_type != '\0')
            {
                if (is_variable_declared_in_history(var_name))
                {
                    history *prev = find_in_history(var_name);
                    char err_msg[256];
                    snprintf(err_msg, sizeof(err_msg),
                             "Variable '%s' already declared on line %d",
                             var_name, prev->line);
                    create_error_log("Redeclaration Error", err_msg, line);
                    is_valid = 0;
                }
            }

            // RULE 3: Assignment without data type - variable must be declared first
            if (data_type == '\0')
            {
                history *existing = find_in_history(var_name);
                if (!existing)
                {
                    char err_msg[256];
                    snprintf(err_msg, sizeof(err_msg),
                             "Variable '%s' used without declaration", var_name);
                    create_error_log("Undeclared Variable", err_msg, line);
                    is_valid = 0;
                }

                // Check all variables in the expression are declared
                char identifiers[100][64];
                int id_count;
                extract_identifiers(value, identifiers, &id_count);

                for (int i = 0; i < id_count; i++)
                {
                    history *found = find_in_history(identifiers[i]);
                    if (!found)
                    {
                        char err_msg[256];
                        snprintf(err_msg, sizeof(err_msg),
                                 "Variable '%s' used without declaration", identifiers[i]);
                        create_error_log("Undeclared Variable", err_msg, line);
                        is_valid = 0;
                    }
                }

                // Add to history and symbol table only if valid
                if (is_valid && existing)
                {
                    add_to_history(line, existing->data_type, var_name, value, "assign", 1);
                    add_to_symbol_table(existing->data_type, var_name, value, line);
                }
                else if (!is_valid)
                {
                    add_to_history(line, '\0', var_name, value, "assign", 0);
                }
            }
            else
            {
                // Add to history (always, even if invalid)
                add_to_history(line, data_type, var_name, value, "declare_assign", is_valid);

                // Add to symbol table only if valid
                if (is_valid)
                {
                    add_to_symbol_table(data_type, var_name, value, line);
                    // Track this variable for same-line references
                    strncpy(current_stmt_vars[current_stmt_count], var_name, 63);
                    current_stmt_vars[current_stmt_count][63] = '\0';
                    current_stmt_count++;
                }
            }
        }
        else
        {
            // Just a declaration without initialization
            char *var_name = trim(token);
            int is_valid = 1;

            if (data_type != '\0')
            {
                // Check if variable is already declared using history
                if (is_variable_declared_in_history(var_name))
                {
                    history *prev = find_in_history(var_name);
                    char err_msg[256];
                    snprintf(err_msg, sizeof(err_msg),
                             "Variable '%s' already declared on line %d",
                             var_name, prev->line);
                    create_error_log("Redeclaration Error", err_msg, line);
                    is_valid = 0;
                }

                // Add to history (always)
                add_to_history(line, data_type, var_name, NULL, "declare", is_valid);

                // Add to symbol table only if valid
                if (is_valid)
                {
                    add_to_symbol_table(data_type, var_name, NULL, line);
                    // Track this variable for same-line references
                    strncpy(current_stmt_vars[current_stmt_count], var_name, 63);
                    current_stmt_vars[current_stmt_count][63] = '\0';
                    current_stmt_count++;
                }
            }
            else
            {
                // Pure expression or variable reference without assignment
                // Check all variables in the expression exist
                char identifiers[100][64];
                int id_count;
                extract_identifiers(var_name, identifiers, &id_count);

                int all_vars_valid = 1;
                for (int i = 0; i < id_count; i++)
                {
                    if (!find_in_history(identifiers[i]))
                    {
                        char err_msg[256];
                        snprintf(err_msg, sizeof(err_msg),
                                 "Variable '%s' used without declaration", identifiers[i]);
                        create_error_log("Undeclared Variable", err_msg, line);
                        all_vars_valid = 0;
                    }
                }

                // Add pure expression to history
                // Use "expression" as action type
                add_to_history(line, '\0', var_name, NULL, "expression", all_vars_valid);
            }
        }

        token = strtok_r(NULL, ",", &saveptr);
    }

    free(start_copy);
}

// Parse syntax and extract variables with proper C semantics
void parse_syntax(char *raw_input, int line)
{
    if (!raw_input || strlen(raw_input) == 0)
        return;

    if (!is_valid_syntax(raw_input))
    {
        create_error_log("Syntax Error", "Invalid statement", line);
        return;
    }

    // Create a copy to work with
    char *input_copy = strdup(raw_input);
    char *ptr = input_copy;

    // Split by semicolons and process each statement
    char *statement_start = ptr;

    while (*ptr)
    {
        if (*ptr == ';')
        {
            *ptr = '\0'; // Terminate the current statement

            char *statement = trim(statement_start);

            if (strlen(statement) > 0)
            {
                // Process this statement
                parse_single_statement(statement, line);
            }

            // Move to next statement
            statement_start = ptr + 1;
        }
        ptr++;
    }

    // Process last statement if there's anything left
    char *statement = trim(statement_start);
    if (strlen(statement) > 0)
    {
        parse_single_statement(statement, line);
    }

    free(input_copy);
}

// Check each code if it's valid (optional additional validations)
void check_code_validity()
{
}

// Helper function to print history
void print_history()
{
    printf("\n=== History (Chronological Order) ===\n");
    history *curr = list_hist_head;
    if (!curr)
    {
        printf("No history recorded.\n");
    }
    while (curr)
    {
        printf("Line %d [%s] %s: Variable '%s', Value: %s, DataType: %c, Valid: %s\n",
               curr->line, curr->action,
               curr->is_valid ? "✓" : "✗",
               curr->variable, curr->value,
               curr->data_type ? curr->data_type : '-',
               curr->is_valid ? "Yes" : "No");
        curr = curr->next;
    }
}

// Helper function to print symbol table
void print_symbol_table()
{
    printf("\n=== Symbol Table (Valid Variables Only) ===\n");
    symbols *curr = list_var_head;
    if (!curr)
    {
        printf("No variables declared.\n");
    }
    while (curr)
    {
        printf("Line %d - Type: %c, ID: %s, Value: %s\n",
               curr->line_declared, curr->data_type, curr->id, curr->value);
        curr = curr->next;
    }
}

// Helper function to print errors
void print_errors()
{
    printf("\n=== Errors ===\n");
    error *curr = list_error_head;
    if (!curr)
    {
        printf("No errors found.\n");
    }
    while (curr)
    {
        printf("Line %d [%s]: %s\n",
               curr->line, curr->error_type, curr->err_msg);
        curr = curr->next;
    }
}

// Example usage
int main()
{
    printf("=== Test Cases ===\n");
    int line = 1;
    FILE *input = fopen("input.txt", "r");
    // parse_syntax("2 + 1;", 2);
    if (!input)
    {
        printf("input file doesn't exist!\n");
        return -1;
    }

    char buffer[255];
    while (fgets(buffer, sizeof(buffer), input) != NULL)
    {
        parse_syntax(buffer, line);
        line += 1;
    }
    print_history();
    print_symbol_table();

    // Pass 0 to disable warnings about uninitialized variables
    // Pass 1 to enable warnings
    check_code_validity(0);

    print_errors();

    fclose(input);
    return 0;
}