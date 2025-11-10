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
    char *source_variable; // NEW: for variable-to-variable assignments (e.g., y = x)
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
void add_history_with_source(int line_num, int op_type, const char *var_name, int data_type, int has_val, int val, const char *source_var, const char *original_line);
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
int is_variable_name(const char *str);
void process_declaration(const char *declaration, int line_num);
void process_assignment(const char *assignment, int line_num);
int get_register_number(char *reg);
void convert_mips64_to_binhex(char *filename);


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
}

// Add an entry to the history with source variable tracking
void add_history_with_source(int line_num, int op_type, const char *var_name, int data_type, int has_val, int val, const char *source_var, const char *original_line) {
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
    new_entry->source_variable = source_var ? strdup(source_var) : NULL;
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

// Check if a string is a valid variable name (not a number or char literal)
int is_variable_name(const char *str) {
    if (str == NULL || strlen(str) == 0) return 0;
    
    // Check if it's a char literal ('a')
    if (str[0] == '\'') return 0;
    
    // Check if it's a number
    if (isdigit(str[0]) || (str[0] == '-' && strlen(str) > 1 && isdigit(str[1]))) {
        return 0;
    }
    
    // Check if first character is valid (letter or underscore)
    if (!isalpha(str[0]) && str[0] != '_') return 0;
    
    // Valid variable name
    return 1;
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
            char *source_var = NULL;
            
            // If has assignment, set the value
            if (value) {
                has_val = 1;
                
                // Check if value is a variable name
                if (is_variable_name(value)) {
                    vars *source = find_variable(value);
                    if (source != NULL) {
                        // Variable-to-variable assignment
                        source_var = strdup(value);
                        int_val = source->data.val;
                        set_variable_value(var_name, int_val, NULL);
                        printf("  -> Variable '%s' initialized from variable '%s' (value: %d)\n", 
                               var_name, value, int_val);
                    } else {
                        add_error(line_num, ERROR_UNDECLARED, declaration);
                        free(var_name);
                        free(data_type_str);
                        free(value);
                        return;
                    }
                } else {
                    // Direct value assignment
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
            }
            
            // Add to history with source variable tracking
            add_history_with_source(line_num, OP_DECLARATION, var_name, data_type, has_val, int_val, source_var, declaration);
            
            if (source_var) free(source_var);
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
        char *source_var = NULL;
        
        // Update the value
        if (value) {
            // Check if value is a variable name
            if (is_variable_name(value)) {
                vars *source = find_variable(value);
                if (source != NULL) {
                    // Variable-to-variable assignment
                    source_var = strdup(value);
                    int_val = source->data.val;
                    set_variable_value(var_name, int_val, NULL);
                    printf("  -> Variable '%s' updated from variable '%s' (value: %d)\n", 
                           var_name, value, int_val);
                } else {
                    add_error(line_num, ERROR_UNDECLARED, assignment);
                    free(var_name);
                    free(value);
                    return;
                }
            } else {
                // Direct value assignment
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
            }
            
            // Add to history with source variable tracking
            add_history_with_source(line_num, OP_ASSIGNMENT, var_name, var->data_type, 1, int_val, source_var, assignment);
            
            if (source_var) free(source_var);
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
        printf("$%-9d ", current->reg_num);
        
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
    printf("%-6s %-15s %-12s %-10s %-20s %-15s\n", "Line", "Operation", "Variable", "Type", "Value", "Source Var");
    printf("--------------------------------------------------------------------------------------------\n");
    
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
                printf("'%c' (%-16d)", (char)current->value, current->value);
            }
        } else {
            printf("%-20s", "(uninitialized)");
        }
        
        // Print source variable
        if (current->source_variable) {
            printf("%-15s", current->source_variable);
        } else {
            printf("%-15s", "-");
        }
        
        printf("\n");
        current = current->next;
    }
    printf("\n");
}

// Generate MIPS64 code from history and write to output.txt
void generate_mips64() {
    FILE *output_file = fopen("output.txt", "w");
    if (!output_file) {
        printf("Error: Could not create output.txt file\n");
        return;
    }

    if (!history_head) {
        fprintf(output_file, "# No operations to generate\n");
        fclose(output_file);
        return;
    }

    // --- DATA SECTION ---
    fprintf(output_file, ".data\n");

    // Walk history for declarations
    history *cur_hist = history_head;
    while (cur_hist) {
        if (cur_hist->operation_type == OP_DECLARATION) {
            vars *v = find_variable(cur_hist->variable_name);
            if (!v) { cur_hist = cur_hist->next; continue; }

            if (v->data_type == TYPE_INT) {
                if (cur_hist->has_value) {
                    // Use the initial value stored in history
                    fprintf(output_file, "%s: .word %d\n", v->id, cur_hist->value);
                } else {
                    fprintf(output_file, "%s: .space 8\n", v->id);
                }
            } else { // TYPE_CHAR
                if (cur_hist->has_value) {
                    fprintf(output_file, "%s: .byte %d\n", v->id, cur_hist->value & 0xFF);
                } else {
                    fprintf(output_file, "%s: .space 1\n", v->id);
                }
            }
        }
        cur_hist = cur_hist->next;
    }

    fprintf(output_file, "\n.text\n");
    fprintf(output_file, ".globl main\n");
    fprintf(output_file, "main:\n");

    // --- TEXT SECTION: use history to emit operations in order ---
    history *current = history_head;
    while (current) {
        vars *dst = find_variable(current->variable_name);
        if (!dst) {
            fprintf(output_file, "    # ERROR: Variable '%s' not found in symbol table\n\n",
                    current->variable_name);
            current = current->next;
            continue;
        }

        if (current->operation_type == OP_DECLARATION) {
            if (current->has_value) {
                // Load initial value into register, then store to memory
                fprintf(output_file, "    # Declaration: %s = %d\n", dst->id, current->value);
                fprintf(output_file, "    daddiu r%d, r0, %d      # Load value into register\n",
                        dst->reg_num, current->value);
                fprintf(output_file, "    dla r8, %s              # Load address of %s\n",
                        dst->id, dst->id);
                
                if (dst->data_type == TYPE_INT) {
                    fprintf(output_file, "    sd r%d, 0(r8)           # Store to memory\n",
                            dst->reg_num);
                } else { // TYPE_CHAR
                    fprintf(output_file, "    sb r%d, 0(r8)           # Store byte to memory\n",
                            dst->reg_num);
                }
            } else {
                fprintf(output_file, "    # Declaration: %s (uninitialized)\n", dst->id);
            }
        } else if (current->operation_type == OP_ASSIGNMENT) {
            if (current->source_variable) {
                // Variable-to-variable assignment: load from source, store to destination
                vars *src = find_variable(current->source_variable);
                if (src) {
                    fprintf(output_file, "    # Assignment: %s = %s\n", dst->id, src->id);
                    fprintf(output_file, "    dla r8, %s              # Load address of %s\n",
                            src->id, src->id);
                    
                    if (src->data_type == TYPE_INT) {
                        fprintf(output_file, "    ld r%d, 0(r8)           # Load value from memory\n",
                                dst->reg_num);
                    } else { // TYPE_CHAR
                        fprintf(output_file, "    lb r%d, 0(r8)           # Load byte from memory\n",
                                dst->reg_num);
                    }
                    
                    fprintf(output_file, "    dla r8, %s              # Load address of %s\n",
                            dst->id, dst->id);
                    
                    if (dst->data_type == TYPE_INT) {
                        fprintf(output_file, "    sd r%d, 0(r8)           # Store to memory\n",
                                dst->reg_num);
                    } else { // TYPE_CHAR
                        fprintf(output_file, "    sb r%d, 0(r8)           # Store byte to memory\n",
                                dst->reg_num);
                    }
                } else {
                    fprintf(output_file, "    # ERROR: Source variable '%s' not found\n",
                            current->source_variable);
                }
            } else {
                // Immediate value assignment
                fprintf(output_file, "    # Assignment: %s = %d\n", dst->id, current->value);
                fprintf(output_file, "    daddiu r%d, r0, %d      # Load immediate value\n",
                        dst->reg_num, current->value);
                fprintf(output_file, "    dla r8, %s              # Load address of %s\n",
                        dst->id, dst->id);
                
                if (dst->data_type == TYPE_INT) {
                    fprintf(output_file, "    sd r%d, 0(r8)           # Store to memory\n",
                            dst->reg_num);
                } else { // TYPE_CHAR
                    fprintf(output_file, "    sb r%d, 0(r8)           # Store byte to memory\n",
                            dst->reg_num);
                }
            }
        }

        fprintf(output_file, "\n");
        current = current->next;
    }

    // --- Exit: put syscall number into r31 then syscall ---
    fprintf(output_file, "    daddiu r31, r0, 10      # Exit syscall\n");
    fprintf(output_file, "    syscall\n");

    fclose(output_file);
    printf("\n=== MIPS64 Code Generated Successfully (with memory operations) ===\n");
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
        if (temp->source_variable != NULL) {
            free(temp->source_variable);
        }
        if (temp->original_line != NULL) {
            free(temp->original_line);
        }
        free(temp);
    }
    history_head = NULL;
    history_tail = NULL;
}


int get_register_number(char *reg) {
    if (reg[0] == 'r' || reg[0] == 'R') return atoi(reg + 1);
    return -1;
}

void convert_mips64_to_binhex(char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open %s\n", filename);
        return;
    }

    char line[256];
    int instr_count = 1;
    uint32_t instructions[1024]; // Store all instructions
    int total_instrs = 0;

    printf("+----+------------------------+---------------------------------+----------+\n");
    printf("| No | Instruction            | 32-bit Binary                   | Hex      |\n");
    printf("+----+------------------------+---------------------------------+----------+\n");

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;

        // Skip empty lines, comments, directives, labels
        if (line[0] == '\0' || line[0] == '#' || line[0] == '.' || 
            (strlen(line) > 0 && line[strlen(line)-1] == ':'))
            continue;

        // Trim leading whitespace
        char *trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
        if (*trimmed == '\0') continue;
        
        // Skip lines containing .word or other data declarations
        if (strstr(trimmed, ".word") != NULL || strstr(trimmed, ":") != NULL) {
            continue;
        }

        char instr[16], rd[8], rs[8], rt[8];
        int imm;
        uint32_t binary = 0;
        char bin_str[36];

        // daddiu rD, rS, IMM
        if (sscanf(trimmed, " %15s %7[^,], %7[^,], %d", instr, rd, rs, &imm) == 4) {
            // Remove any whitespace from register names
            char *rd_clean = rd;
            while (*rd_clean == ' ') rd_clean++;
            char *rs_clean = rs;
            while (*rs_clean == ' ') rs_clean++;
            
            int rd_num = get_register_number(rd_clean);
            int rs_num = get_register_number(rs_clean);
            
            if (strcmp(instr, "daddiu") == 0) {
                int opcode = 0x19; // daddiu opcode (25 decimal)
                binary = (opcode << 26) | (rs_num << 21) | (rd_num << 16) | (imm & 0xFFFF);
            }
        }
        // syscall
        else if (strstr(trimmed, "syscall") != NULL) {
            binary = 0x0000000C; // syscall
        }
        else {
            printf("| %-2d | %-22s | %-31s | %-8s |\n",
                   instr_count,
                   trimmed,
                   "UNKNOWN",
                   "UNKNOWN");
            instr_count++;
            continue;
        }

        // Convert to binary string with spaces every 8 bits
        int bin_idx = 0;
        for (int i = 31; i >= 0; i--) {
            bin_str[bin_idx++] = (binary & (1 << i)) ? '1' : '0';
            if (i % 8 == 0 && i != 0) {
                bin_str[bin_idx++] = ' ';
            }
        }
        bin_str[bin_idx] = '\0';

        printf("| %-2d | %-22s | %-31s | 0x%08X |\n",
               instr_count,
               trimmed,
               bin_str,
               binary);
        
        // Store instruction for merged output
        instructions[total_instrs++] = binary;
        instr_count++;
    }

    printf("+----+------------------------+---------------------------------+----------+\n");
    
    // Print merged binary and hex
    if (total_instrs > 0) {
        printf("\n=== Merged Binary (Execution Order) ===\n");
        for (int i = 0; i < total_instrs; i++) {
            for (int bit = 31; bit >= 0; bit--) {
                printf("%c", (instructions[i] & (1 << bit)) ? '1' : '0');
            }
        }
        printf("\n\n=== Merged Hexadecimal ===\n");
        for (int i = 0; i < total_instrs; i++) {
            printf("%08X", instructions[i]);
        }
        printf("\n");
    }
    
    fclose(file);
}

int main() {
    const char *pattern[] = {
        "^(([[:space:]]*(int|char)[[:space:]]+[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]*)(=[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]')([[:space:]]*(\\+|\\-|\\*|\\/)[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]'))*)?[[:space:]]*;[[:space:]]*)+$",
        "^[[:space:]]*((int|char)[[:space:]]+)?[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]*((=[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]')+([[:space:]]*)*)?)*(=[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]')([[:space:]]*(\\+|\\-|\\*|\\/)[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]'))*)?[[:space:]]*(,[[:space:]]*[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]*(=[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]')([[:space:]]*(\\+|\\-|\\*|\\/)[[:space:]]*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+|'[a-zA-Z0-9_]'))*)?[[:space:]]*)*;[[:space:]]*$",
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
        convert_mips64_to_binhex("output.txt");
    }
    
    print_errors();
    
    // Cleanup
    free_symbol_table();
    free_error_list();
    free_history();
    
    return 0;
}