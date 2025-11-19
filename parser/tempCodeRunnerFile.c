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

// --- MODIFIED: Add History Entry ---
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

// (Helper functions for string extraction are unchanged)
char *extract_variable_name(const char *declaration)
{
    if (declaration == NULL)
        return NULL;
    char *temp = strdup(declaration);
    if (temp == NULL)