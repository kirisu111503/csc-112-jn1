#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>

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
typedef struct vars{
    char *id;
    int data_type;
    union {
        int val;      // for int/char
        char *str_val; // for string
    } data;
    int has_value;    // Flag to indicate if variable has been assigned
    int reg_num;      // Register number assigned to this variable
    struct vars *next;
} vars;

// Error list structure
typedef struct errorList{
   int line_error;
   char *error_type;
   char *line_content;  // Store the problematic line
   struct errorList *next;
} errorList;

// History structure - stores all operations in order
typedef struct history{
    int line_num;
    int operation_type;  // OP_DECLARATION or OP_ASSIGNMENT
    char *variable_name;
    int data_type;       // TYPE_INT or TYPE_CHAR (for declarations)
    int has_value;       // Whether it has an initial/assigned value
    int value;           // The value (if has_value is true)
    char *original_line; // Store the original line for reference
    struct history *next;
} history;

// Global symbol table, error list, and history
vars *symbol_table = NULL;
errorList *error_list_head = NULL;
history *history_head = NULL;
history *history_tail = NULL;
int next_register = 1;  // Start from r1 (r0 is reserved for zero)

// Function prototypes
vars* find_variable(const char *id);
int add_variable(const char *id, int data_type, int line_num);
void set_variable_value(const char *id, int int_val, const char *str_val);
void add_error(int line_num, const char *error_type, const char *line_content);
void add_history(int line_num, int op_type, const char *var_name, int data_type, int has_val, int val, const char *original_line);
void print_symbol_table();
void print_errors();
void print_history();
void generate_mips64();
void free_symbol_table();
void free_error_list();
void free_history();
char* extract_variable_name(const char *declaration);
char* extract_data_type(const char *declaration);
char* extract_value(const char *declaration);
int is_declaration(const char *line);
void process_declaration(const char *declaration, int line_num);
void process_assignment(const char *assignment, int line_num);

// Find a variable in the symbol table
vars* find_variable(const char *id) {
    vars *current = symbol_table;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Add a new variable to the symbol table
// Returns 1 if successful, 0 if variable already exists
int add_variable(const char *id, int data_type, int line_num) {
    // Check if variable already exists
    if (find_variable(id) != NULL) {
        add_error(line_num, ERROR_REDECLARATION, NULL);
        return 0;
    }
    
    // Create new variable node
    vars *new_var = (vars*)malloc(sizeof(vars));
    if (new_var == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 0;
    }
    
    // Initialize all fields
    new_var->id = strdup(id);
    if (new_var->id == NULL) {
        free(new_var);
        return 0;
    }
    
    new_var->data_type = data_type;
    new_var->has_value = 0;
    new_var->data.val = 0;  // Initialize union to 0
    new_var->reg_num = next_register++;  // Assign next available register
    new_var->next = symbol_table;
    symbol_table = new_var;
    
    return 1;
}

// Set the value of an existing variable
void set_variable_value(const char *id, int int_val, const char *str_val) {
    vars *var = find_variable(id);
    if (var != NULL) {
        var->has_value = 1;
        if (var->data_type == TYPE_INT || var->data_type == TYPE_CHAR) {
            var->data.val = int_val;
        } else {
            // Only for string types (not used in current implementation)
            var->data.str_val = str_val ? strdup(str_val) : NULL;
        }
    }
}

// Add an error to the error list and terminate
void add_error(int line_num, const char *error_type, const char *line_content) {
    errorList *new_error = (errorList*)malloc(sizeof(errorList));
    if (new_error == NULL) {
        fprintf(stderr, "Memory allocation failed for error\n");
        exit(EXIT_FAILURE);
    }
    
    new_error->line_error = line_num;
    new_error->error_type = strdup(error_type);
    new_error->line_content = line_content ? strdup(line_content) : NULL;
    
    if (new_error->error_type == NULL) {
        if (new_error->line_content) free(new_error->line_content);
        free(new_error);
        fprintf(stderr, "Memory allocation failed for error\n");
        exit(EXIT_FAILURE);
    }
    
    new_error->next = error_list_head;
    error_list_head = new_error;
    
    // Display error and terminate
    fprintf(stderr, "\n=== ERROR DETECTED ===\n");
    fprintf(stderr, "LINE %d: %s\n", new_error->line_error, new_error->error_type);
    if (new_error->line_content) {
        fprintf(stderr, "Content: %s\n", new_error->line_content);
    }
    fprintf(stderr, "======================\n");
    fprintf(stderr, "Program terminated due to error.\n\n");
    
    // Clean up before exit
    free_symbol_table();
    free_error_list();
    free_history();
    
    exit(EXIT_FAILURE);
}

// Add an entry to the history
void add_history(int line_num, int op_type, const char *var_name, int data_type, int has_val, int val, const char *original_line) {
    history *new_entry = (history*)malloc(sizeof(history));
    if (new_entry == NULL) {
        fprintf(stderr, "Memory allocation failed for history\n");
        return;
    }
    
    new_entry->line_num = line_num;
    new_entry->operation_type = op_type;
    new_entry->variable_name = strdup(var_name);
    new_entry->data_type = data_type;
    new_entry->has_value = has_val;
    new_entry->value = val;
    new_entry->original_line = original_line ? strdup(original_line) : NULL;
    new_entry->next = NULL;
    
    // Add to the end of the list (to maintain order)
    if (history_tail == NULL) {
        history_head = new_entry;
        history_tail = new_entry;
    } else {
        history_tail->next = new_entry;
        history_tail = new_entry;
    }
}

// Extract variable name from declaration
char* extract_variable_name(const char *declaration) {
    if (declaration == NULL) return NULL;
    
    char *temp = strdup(declaration);
    if (temp == NULL) return NULL;
    
    char *token;
    char *name = NULL;
    
    // Remove leading/trailing whitespace and semicolon
    char *start = temp;
    while (isspace(*start)) start++;
    
    // Tokenize by space
    token = strtok(start, " \t\n;=,");
    
    // Skip data type if present (int or char)
    if (token && (strcmp(token, "int") == 0 || strcmp(token, "char") == 0)) {
        token = strtok(NULL, " \t\n;=,");
    }
    
    if (token) {
        name = strdup(token);
    }
    
    free(temp);
    return name;
}

// Extract data type from declaration
char* extract_data_type(const char *declaration) {
    if (declaration == NULL) return NULL;
    
    char *temp = strdup(declaration);
    if (temp == NULL) return NULL;
    
    char *token;
    char *type = NULL;
    
    char *start = temp;
    while (isspace(*start)) start++;
    
    token = strtok(start, " \t\n");
    
    if (token && (strcmp(token, "int") == 0 || strcmp(token, "char") == 0)) {
        type = strdup(token);
    }
    
    free(temp);
    return type;
}

// Extract value from declaration (if has assignment)
char* extract_value(const char *declaration) {
    if (declaration == NULL) return NULL;
    
    const char *equals = strchr(declaration, '=');
    if (equals == NULL) {
        return NULL;
    }
    
    equals++; // Move past '='
    while (isspace(*equals)) equals++;
    
    // Extract until semicolon or comma
    const char *end = equals;
    while (*end && *end != ';' && *end != ',') end++;
    
    size_t len = end - equals;
    if (len == 0) return NULL;
    
    char *value = (char*)malloc(len + 1);
    if (value == NULL) return NULL;
    
    strncpy(value, equals, len);
    value[len] = '\0';
    
    // Trim trailing whitespace
    char *trim_end = value + strlen(value) - 1;
    while (trim_end > value && isspace(*trim_end)) {
        *trim_end = '\0';
        trim_end--;
    }
    
    return value;
}

// Check if a line is a declaration (has data type) or just an assignment
int is_declaration(const char *line) {
    if (line == NULL) return 0;
    
    char *temp = strdup(line);
    if (temp == NULL) return 0;
    
    char *start = temp;
    while (isspace(*start)) start++;
    
    char *token = strtok(start, " \t\n");
    int result = 0;
    
    if (token && (strcmp(token, "int") == 0 || strcmp(token, "char") == 0)) {
        result = 1;
    }
    
    free(temp);
    return result;
}

// Process a variable declaration (with data type)
void process_declaration(const char *declaration, int line_num) {
    if (declaration == NULL) return;
    
    char *var_name = extract_variable_name(declaration);
    char *data_type_str = extract_data_type(declaration);
    char *value = extract_value(declaration);
    
    if (var_name && data_type_str) {
        int data_type = (strcmp(data_type_str, "int") == 0) ? TYPE_INT : TYPE_CHAR;
        
        // Try to add variable to symbol table
        if (add_variable(var_name, data_type, line_num)) {
            printf("  -> Variable '%s' declared as %s\n", var_name, data_type_str);
            
            int has_val = 0;
            int int_val = 0;
            
            // If has assignment, set the value
            if (value) {
                has_val = 1;
                // Parse the value (simplified - you can enhance this)
                if (data_type == TYPE_INT) {
                    int_val = atoi(value);
                    set_variable_value(var_name, int_val, NULL);
                    printf("  -> Variable '%s' initialized with value %d\n", var_name, int_val);
                } else if (data_type == TYPE_CHAR) {
                    // Handle char value (e.g., 'A' or 65)
                    if (value[0] == '\'') {
                        int_val = value[1];
                        set_variable_value(var_name, int_val, NULL);
                        printf("  -> Variable '%s' initialized with value '%c'\n", var_name, (char)int_val);
                    } else {
                        int_val = atoi(value);
                        set_variable_value(var_name, int_val, NULL);
                        printf("  -> Variable '%s' initialized with value %d\n", var_name, int_val);
                    }
                }
            }
            
            // Add to history
            add_history(line_num, OP_DECLARATION, var_name, data_type, has_val, int_val, declaration);
        }
        // If add_variable returns 0, error has already been logged and program terminated
    }
    
    if (var_name) free(var_name);
    if (data_type_str) free(data_type_str);
    if (value) free(value);
}

// Process a variable assignment (without data type - reassignment)
void process_assignment(const char *assignment, int line_num) {
    if (assignment == NULL) return;
    
    char *var_name = extract_variable_name(assignment);
    char *value = extract_value(assignment);
    
    if (var_name) {
        // Check if variable exists
        vars *var = find_variable(var_name);
        if (var == NULL) {
            add_error(line_num, ERROR_UNDECLARED, assignment);
            free(var_name);
            if (value) free(value);
            return;
        }
        
        printf("  -> Reassigning variable '%s'\n", var_name);
        
        int int_val = 0;
        
        // Update the value
        if (value) {
            if (var->data_type == TYPE_INT) {
                int_val = atoi(value);
                set_variable_value(var_name, int_val, NULL);
                printf("  -> Variable '%s' updated with value %d\n", var_name, int_val);
            } else if (var->data_type == TYPE_CHAR) {
                // Handle char value (e.g., 'A' or 65)
                if (value[0] == '\'') {
                    int_val = value[1];
                    set_variable_value(var_name, int_val, NULL);
                    printf("  -> Variable '%s' updated with value '%c'\n", var_name, (char)int_val);
                } else {
                    int_val = atoi(value);
                    set_variable_value(var_name, int_val, NULL);
                    printf("  -> Variable '%s' updated with value %d\n", var_name, int_val);
                }
            }
            
            // Add to history
            add_history(line_num, OP_ASSIGNMENT, var_name, var->data_type, 1, int_val, assignment);
        }
    }
    
    if (var_name) free(var_name);
    if (value) free(value);
}

// Print symbol table
void print_symbol_table() {
    if (symbol_table == NULL) {
        printf("\n=== Symbol Table ===\n");
        printf("(empty)\n\n");
        return;
    }
    
    printf("\n=== Symbol Table ===\n");
    printf("%-15s %-10s %-10s %-15s\n", "Variable", "Type", "Register", "Value");
    printf("--------------------------------------------------------\n");
    
    vars *current = symbol_table;
    while (current != NULL) {
        printf("%-15s ", current->id);
        printf("%-10s ", current->data_type == TYPE_INT ? "int" : "char");
        printf("r%-9d ", current->reg_num);
        
        if (current->has_value) {
            if (current->data_type == TYPE_INT) {
                printf("%-15d", current->data.val);
            } else if (current->data_type == TYPE_CHAR) {
                printf("'%c' (%d)", (char)current->data.val, current->data.val);
            }
        } else {
            printf("%-15s", "(uninitialized)");
        }
        printf("\n");
        
        current = current->next;
    }
    printf("\n");
}

// Print error list
void print_errors() {
    if (error_list_head == NULL) {
        printf("\n=== No Errors Found ===\n\n");
        return;
    }
    
    printf("\n=== Error List ===\n");
    printf("%-10s %-30s %s\n", "Line", "Error Type", "Details");
    printf("-------------------------------------------------------------------------\n");
    
    errorList *current = error_list_head;
    while (current != NULL) {
        printf("%-10d %-30s", current->line_error, current->error_type);
        if (current->line_content) {
            printf(" %s", current->line_content);
        }
        printf("\n");
        current = current->next;
    }
    printf("\n");
}

// Print history
void print_history() {
    if (history_head == NULL) {
        printf("\n=== Operation History ===\n");
        printf("(empty)\n\n");
        return;
    }
    
    printf("\n=== Operation History (for MIPS64 Generation) ===\n");
    printf("%-6s %-15s %-12s %-10s %-20s\n", "Line", "Operation", "Variable", "Type", "Value");
    printf("--------------------------------------------------------------------------------\n");
    
    history *current = history_head;
    while (current != NULL) {
        printf("%-6d ", current->line_num);
        
        // Print operation type
        if (current->operation_type == OP_DECLARATION) {
            printf("%-15s ", "DECLARE");
        } else {
            printf("%-15s ", "ASSIGN");
        }
        
        // Print variable name
        printf("%-12s ", current->variable_name);
        
        // Print data type
        printf("%-10s ", current->data_type == TYPE_INT ? "int" : "char");
        
        // Print value
        if (current->has_value) {
            if (current->data_type == TYPE_INT) {
                printf("%-20d", current->value);
            } else {
                printf("'%c' (%d)", (char)current->value, current->value);
            }
        } else {
            printf("%-20s", "(uninitialized)");
        }
        
        printf("\n");
        current = current->next;
    }
    printf("\n");
}

// Generate MIPS64 code from history
void generate_mips64() {
    if (history_head == NULL) {
        printf("\n=== MIPS64 Code ===\n");
        printf("# No operations to generate\n\n");
        return;
    }
    
    printf("\n=== MIPS64 Code (64-bit instruction format) ===\n");
    printf("# Generated from operation history\n");
    printf("# Register r0 is always 0\n");
    printf("# Variables are allocated to registers r1-r31\n\n");
    
    printf(".data\n");
    printf("# Data section (if needed for future extensions)\n\n");
    
    printf(".text\n");
    printf(".globl main\n");
    printf("main:\n");
    
    history *current = history_head;
    while (current != NULL) {
        // Add comment with original source line
        printf("    # Line %d: %s\n", current->line_num, 
               current->original_line ? current->original_line : "");
        
        // Find the variable to get its register
        vars *var = find_variable(current->variable_name);
        if (var == NULL) {
            printf("    # ERROR: Variable '%s' not found in symbol table\n", 
                   current->variable_name);
            current = current->next;
            continue;
        }
        
        if (current->operation_type == OP_DECLARATION) {
            if (current->has_value) {
                // Declaration with initialization
                // Load immediate value into register
                printf("    daddi r%d, r0, %d    # %s %s = %d\n", 
                       var->reg_num, current->value, 
                       current->data_type == TYPE_INT ? "int" : "char",
                       current->variable_name, current->value);
            } else {
                // Declaration without initialization (just a comment)
                printf("    # %s %s declared (uninitialized) -> r%d\n",
                       current->data_type == TYPE_INT ? "int" : "char",
                       current->variable_name, var->reg_num);
            }
        } else if (current->operation_type == OP_ASSIGNMENT) {
            // Assignment (reassignment of value)
            printf("    daddi r%d, r0, %d    # %s = %d\n", 
                   var->reg_num, current->value,
                   current->variable_name, current->value);
        }
        
        printf("\n");
        current = current->next;
    }
    
    // Exit syscall
    printf("    # Program exit\n");
    printf("    daddi r2, r0, 10     # syscall code 10 (exit)\n");
    printf("    syscall              # exit program\n");
    printf("\n");
    
    // Print register allocation summary
    printf("# Register Allocation Summary:\n");
    printf("# r0  : constant 0 (hardwired)\n");
    vars *v = symbol_table;
    while (v != NULL) {
        printf("# r%-2d : %s (%s)\n", v->reg_num, v->id, 
               v->data_type == TYPE_INT ? "int" : "char");
        v = v->next;
    }
    printf("\n");
}

// Free symbol table memory
void free_symbol_table() {
    vars *current = symbol_table;
    while (current != NULL) {
        vars *temp = current;
        current = current->next;
        
        // Free the id string
        if (temp->id != NULL) {
            free(temp->id);
        }
        
        // Free the node itself
        free(temp);
    }
    symbol_table = NULL;
}

// Free error list memory
void free_error_list() {
    errorList *current = error_list_head;
    while (current != NULL) {
        errorList *temp = current;
        current = current->next;
        
        if (temp->error_type != NULL) {
            free(temp->error_type);
        }
        if (temp->line_content != NULL) {
            free(temp->line_content);
        }
        free(temp);
    }
    error_list_head = NULL;
}

// Free history memory
void free_history() {
    history *current = history_head;
    while (current != NULL) {
        history *temp = current;
        current = current->next;
        
        if (temp->variable_name != NULL) {
            free(temp->variable_name);
        }
        if (temp->original_line != NULL) {
            free(temp->original_line);
        }
        free(temp);
    }
    history_head = NULL;
    history_tail = NULL;
}

int main() {
    const char *pattern[] = {
        "^(([[:space:]]*(int|char)[[:space:]]+[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]*)(=[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]')([[:space:]]*(\\+|\\-|\\*|\\/)[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]'))*)?[[:space:]]*;[[:space:]]*)+$",
        "^[[:space:]]*((int|char)[[:space:]]+)?[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]*((=[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]')+([[:space:]]*)*)?)*(=[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]')([[:space:]]*(\\+|\\-|\\*|\\/)[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]'))*)?(,[[:space:]]*[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]*(=[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]')([[:space:]]*(\\+|\\-|\\*|\\/)[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]'))*)?[[:space:]]*)*;[[:space:]]*$",
    };
    
    // Open input.txt file
    FILE *file = fopen("input.txt", "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open input.txt\n");
        return 1;
    }
    
    char line[1024];
    int line_num = 1;
    regex_t regex;
    
    printf("=== Processing Input File ===\n");
    
    // Read file line by line
    while (fgets(line, sizeof(line), file) != NULL) {
        // Remove trailing newline
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        
        // Skip empty lines
        if (strlen(line) == 0) {
            line_num++;
            continue;
        }
        
        printf("Line %d: %s\n", line_num, line);
        
        int matched = 0;
        
        // Try matching with both patterns
        for (int p = 0; p < 2; p++) {
            if (regcomp(&regex, pattern[p], REG_EXTENDED) == 0) {
                if (regexec(&regex, line, 0, NULL, 0) == 0) {
                    matched = 1;
                    printf("  -> Valid syntax, processing...\n");
                    
                    // Check if it's a declaration or assignment
                    if (is_declaration(line)) {
                        process_declaration(line, line_num);
                    } else {
                        process_assignment(line, line_num);
                    }
                }
                regfree(&regex);
                if (matched) break;
            }
        }
        
        if (!matched) {
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
    if (error_list_head == NULL) {
        generate_mips64();
    }
    
    print_errors();
    
    // Cleanup
    free_symbol_table();
    free_error_list();
    free_history();
    
    return 0;
}
