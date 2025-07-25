#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "variables.h"
#include "ast.h"

#define MAX_VARS 10000
#define MAX_VAR_NAME_LEN 1000
#define MAX_STRING_LEN 1000

typedef struct
{
    char name[MAX_VAR_NAME_LEN + 1];
    union
    {
        char *string_val;  // For string values
        ASTNode *list_val; // For list values
        ASTNode *dict_val; // For dict values
        ASTNode *stack_val; // For stack values
        ASTNode *queue_val; // For queue values
        ASTNode *linked_list_val; // For linked list values
        ASTNode *regex_val; // For regex values
    } value;
    int type; // 0=string, 1=list, 2=dict, 3=stack, 4=queue, 5=linked_list, 6=regex, 7=temporal
    TemporalVariable *temporal_val; // For temporal variables
} VarEntry;

static VarEntry vars[MAX_VARS];
static int var_count = 0;

static VarEntry *find_variable(const char *name)
{
    for (int i = 0; i < var_count; i++)
    {
        if (strcmp(vars[i].name, name) == 0)
        {
            return &vars[i];
        }
    }
    return NULL;
}

void set_variable(const char *name, const char *value)
{
    if (strlen(name) > MAX_VAR_NAME_LEN)
    {
        fprintf(stderr, "Variable name too long: %s\n", name);
        return;
    }

    VarEntry *entry = find_variable(name);
    if (entry)
    {
        // Existing variable
        if (entry->type == 1)
            ast_free(entry->value.list_val);
        else if (entry->type == 2)
            ast_free(entry->value.dict_val);
        else if (entry->type == 3)
            ast_free(entry->value.stack_val);
        else if (entry->type == 4)
            ast_free(entry->value.queue_val);
        else if (entry->type == 5)
            ast_free(entry->value.linked_list_val);
        else if (entry->type == 6)
            ast_free(entry->value.regex_val);
        else if (entry->type == 7)
        {
            // Free temporal variable history
            for (int i = 0; i < entry->temporal_val->count; i++)
            {
                free(entry->temporal_val->history[i].value);
            }
            free(entry->temporal_val);
        }
        else if (entry->type == 0)
            free(entry->value.string_val);

        entry->value.string_val = strdup(value);
        if (!entry->value.string_val)
        {
            perror("Failed to allocate string value");
            exit(EXIT_FAILURE);
        }
        entry->type = 0;
        return;
    }

    // New variable
    if (var_count >= MAX_VARS)
    {
        fprintf(stderr, "Maximum number of variables (%d) exceeded\n", MAX_VARS);
        exit(EXIT_FAILURE);
    }

    strncpy(vars[var_count].name, name, MAX_VAR_NAME_LEN);
    vars[var_count].name[MAX_VAR_NAME_LEN] = '\0';
    vars[var_count].value.string_val = strdup(value);
    if (!vars[var_count].value.string_val)
    {
        perror("Failed to allocate string value");
        exit(EXIT_FAILURE);
    }
    vars[var_count].type = 0;
    var_count++;
}

void set_list_variable(const char *name, ASTNode *list)
{
    if (strlen(name) > MAX_VAR_NAME_LEN)
    {
        fprintf(stderr, "Variable name too long: %s\n", name);
        return;
    }

    if (list->type != NODE_LIST)
    {
        fprintf(stderr, "Attempt to set non-list value as list variable\n");
        return;
    }

    VarEntry *entry = find_variable(name);
    if (entry)
    {
        // Existing variable
        if (entry->type == 0)
            free(entry->value.string_val);
        else if (entry->type == 1)
            ast_free(entry->value.list_val);
        else if (entry->type == 2)
            ast_free(entry->value.dict_val);
        else if (entry->type == 3)
            ast_free(entry->value.stack_val);
        else if (entry->type == 4)
            ast_free(entry->value.queue_val);
        else if (entry->type == 5)
            ast_free(entry->value.linked_list_val);
        else if (entry->type == 6)
            ast_free(entry->value.regex_val);
        entry->value.list_val = list;
        entry->type = 1;
        return;
    }

    // New variable
    if (var_count >= MAX_VARS)
    {
        fprintf(stderr, "Maximum number of variables (%d) exceeded\n", MAX_VARS);
        exit(EXIT_FAILURE);
    }

    strncpy(vars[var_count].name, name, MAX_VAR_NAME_LEN);
    vars[var_count].name[MAX_VAR_NAME_LEN] = '\0';
    vars[var_count].value.list_val = list;
    vars[var_count].type = 1;
    var_count++;
}

void set_dict_variable(const char *name, ASTNode *dict)
{
    if (strlen(name) > MAX_VAR_NAME_LEN)
    {
        fprintf(stderr, "Variable name too long: %s\n", name);
        return;
    }

    if (dict->type != NODE_DICT)
    {
        fprintf(stderr, "Attempt to set non-dict value as dict variable\n");
        return;
    }

    VarEntry *entry = find_variable(name);
    if (entry)
    {
        // Existing variable
        if (entry->type == 0)
            free(entry->value.string_val);
        else if (entry->type == 1)
            ast_free(entry->value.list_val);
        else if (entry->type == 2)
            ast_free(entry->value.dict_val);
        else if (entry->type == 3)
            ast_free(entry->value.stack_val);
        else if (entry->type == 4)
            ast_free(entry->value.queue_val);
        else if (entry->type == 5)
            ast_free(entry->value.linked_list_val);
        else if (entry->type == 6)
            ast_free(entry->value.regex_val);
        entry->value.dict_val = dict;
        entry->type = 2;
        return;
    }

    // New variable
    if (var_count >= MAX_VARS)
    {
        fprintf(stderr, "Maximum number of variables (%d) exceeded\n", MAX_VARS);
        exit(EXIT_FAILURE);
    }

    strncpy(vars[var_count].name, name, MAX_VAR_NAME_LEN);
    vars[var_count].name[MAX_VAR_NAME_LEN] = '\0';
    vars[var_count].value.dict_val = dict;
    vars[var_count].type = 2;
    var_count++;
}

const char *get_variable(const char *name)
{
    VarEntry *entry = find_variable(name);
    if (!entry)
    {
        fprintf(stderr, "Undefined variable: %s\n", name);
        return NULL;
    }
    if (entry->type != 0)
    {
        fprintf(stderr, "Variable %s is not a string\n", name);
        return NULL;
    }
    return entry->value.string_val;
}

ASTNode *get_list_variable(const char *name)
{
    VarEntry *entry = find_variable(name);
    if (!entry || entry->type != 1)
    {
        return NULL;
    }
    return entry->value.list_val;
}

ASTNode *get_dict_variable(const char *name)
{
    VarEntry *entry = find_variable(name);
    if (!entry || entry->type != 2)
    {
        return NULL;
    }
    return entry->value.dict_val;
}

void set_stack_variable(const char *name, ASTNode *stack)
{
    if (strlen(name) > MAX_VAR_NAME_LEN)
    {
        fprintf(stderr, "Variable name too long: %s\n", name);
        return;
    }

    if (stack->type != NODE_STACK)
    {
        fprintf(stderr, "Attempt to set non-stack value as stack variable\n");
        return;
    }

    VarEntry *entry = find_variable(name);
    if (entry)
    {
        // Existing variable
        if (entry->type == 0)
            free(entry->value.string_val);
        else if (entry->type == 1)
            ast_free(entry->value.list_val);
        else if (entry->type == 2)
            ast_free(entry->value.dict_val);
        else if (entry->type == 3)
            ast_free(entry->value.stack_val);
        else if (entry->type == 4)
            ast_free(entry->value.queue_val);
        else if (entry->type == 5)
            ast_free(entry->value.linked_list_val);
        else if (entry->type == 6)
            ast_free(entry->value.regex_val);
        entry->value.stack_val = stack;
        entry->type = 3;
        return;
    }

    // New variable
    if (var_count >= MAX_VARS)
    {
        fprintf(stderr, "Maximum number of variables (%d) exceeded\n", MAX_VARS);
        exit(EXIT_FAILURE);
    }

    strncpy(vars[var_count].name, name, MAX_VAR_NAME_LEN);
    vars[var_count].name[MAX_VAR_NAME_LEN] = '\0';
    vars[var_count].value.stack_val = stack;
    vars[var_count].type = 3;
    var_count++;
}

ASTNode *get_stack_variable(const char *name)
{
    VarEntry *entry = find_variable(name);
    if (!entry || entry->type != 3)
    {
        return NULL;
    }
    return entry->value.stack_val;
}

void set_queue_variable(const char *name, ASTNode *queue)
{
    if (strlen(name) > MAX_VAR_NAME_LEN)
    {
        fprintf(stderr, "Variable name too long: %s\n", name);
        return;
    }

    if (queue->type != NODE_QUEUE)
    {
        fprintf(stderr, "Attempt to set non-queue value as queue variable\n");
        return;
    }

    VarEntry *entry = find_variable(name);
    if (entry)
    {
        if (entry->type == 0)
            free(entry->value.string_val);
        else if (entry->type == 1)
            ast_free(entry->value.list_val);
        else if (entry->type == 2)
            ast_free(entry->value.dict_val);
        else if (entry->type == 3)
            ast_free(entry->value.stack_val);
        else if (entry->type == 4)
            ast_free(entry->value.queue_val);
        else if (entry->type == 5)
            ast_free(entry->value.linked_list_val);
        else if (entry->type == 6)
            ast_free(entry->value.regex_val);
        entry->value.queue_val = queue;
        entry->type = 4;
        return;
    }

    if (var_count >= MAX_VARS)
    {
        fprintf(stderr, "Maximum number of variables (%d) exceeded\n", MAX_VARS);
        exit(EXIT_FAILURE);
    }

    strncpy(vars[var_count].name, name, MAX_VAR_NAME_LEN);
    vars[var_count].name[MAX_VAR_NAME_LEN] = '\0';
    vars[var_count].value.queue_val = queue;
    vars[var_count].type = 4;
    var_count++;
}

ASTNode *get_queue_variable(const char *name)
{
    VarEntry *entry = find_variable(name);
    if (!entry || entry->type != 4)
    {
        return NULL;
    }
    return entry->value.queue_val;
}

void set_linked_list_variable(const char *name, ASTNode *list)
{
    if (strlen(name) > MAX_VAR_NAME_LEN)
    {
        fprintf(stderr, "Variable name too long: %s\n", name);
        return;
    }

    if (list->type != NODE_LINKED_LIST)
    {
        fprintf(stderr, "Attempt to set non-linked-list value as linked list variable\n");
        return;
    }

    VarEntry *entry = find_variable(name);
    if (entry)
    {
        if (entry->type == 0)
            free(entry->value.string_val);
        else if (entry->type == 1)
            ast_free(entry->value.list_val);
        else if (entry->type == 2)
            ast_free(entry->value.dict_val);
        else if (entry->type == 3)
            ast_free(entry->value.stack_val);
        else if (entry->type == 4)
            ast_free(entry->value.queue_val);
        else if (entry->type == 5)
            ast_free(entry->value.linked_list_val);
        entry->value.linked_list_val = list;
        entry->type = 5;
        return;
    }

    if (var_count >= MAX_VARS)
    {
        fprintf(stderr, "Maximum number of variables (%d) exceeded\n", MAX_VARS);
        exit(EXIT_FAILURE);
    }

    strncpy(vars[var_count].name, name, MAX_VAR_NAME_LEN);
    vars[var_count].name[MAX_VAR_NAME_LEN] = '\0';
    vars[var_count].value.linked_list_val = list;
    vars[var_count].type = 5;
    var_count++;
}

ASTNode *get_linked_list_variable(const char *name)
{
    VarEntry *entry = find_variable(name);
    if (!entry || entry->type != 5)
    {
        return NULL;
    }
    return entry->value.linked_list_val;
}

void set_regex_variable(const char *name, ASTNode *regex)
{
    if (strlen(name) > MAX_VAR_NAME_LEN)
    {
        fprintf(stderr, "Variable name too long: %s\n", name);
        return;
    }

    if (regex->type != NODE_REGEX)
    {
        fprintf(stderr, "Attempt to set non-regex value as regex variable\n");
        return;
    }

    VarEntry *entry = find_variable(name);
    if (entry)
    {
        if (entry->type == 0)
            free(entry->value.string_val);
        else if (entry->type == 1)
            ast_free(entry->value.list_val);
        else if (entry->type == 2)
            ast_free(entry->value.dict_val);
        else if (entry->type == 3)
            ast_free(entry->value.stack_val);
        else if (entry->type == 4)
            ast_free(entry->value.queue_val);
        else if (entry->type == 5)
            ast_free(entry->value.linked_list_val);
        else if (entry->type == 6)
            ast_free(entry->value.regex_val);
        entry->value.regex_val = regex;
        entry->type = 6;
        return;
    }

    if (var_count >= MAX_VARS)
    {
        fprintf(stderr, "Maximum number of variables (%d) exceeded\n", MAX_VARS);
        exit(EXIT_FAILURE);
    }

    strncpy(vars[var_count].name, name, MAX_VAR_NAME_LEN);
    vars[var_count].name[MAX_VAR_NAME_LEN] = '\0';
    vars[var_count].value.regex_val = regex;
    vars[var_count].type = 6;
    var_count++;
}

ASTNode *get_regex_variable(const char *name)
{
    VarEntry *entry = find_variable(name);
    if (!entry || entry->type != 6)
    {
        return NULL;
    }
    return entry->value.regex_val;
}

void set_temporal_variable(const char *name, const char *value, int max_history)
{
    if (strlen(name) > MAX_VAR_NAME_LEN)
    {
        fprintf(stderr, "Variable name too long: %s\n", name);
        return;
    }

    if (max_history > MAX_TEMPORAL_HISTORY)
        max_history = MAX_TEMPORAL_HISTORY;

    VarEntry *entry = find_variable(name);
    if (entry && entry->type == 7)
    {
        // Existing temporal variable - add new value to history
        TemporalVariable *temp_var = entry->temporal_val;
        
        // Shift history if at capacity
        if (temp_var->count >= temp_var->max_history)
        {
            free(temp_var->history[0].value);
            for (int i = 0; i < temp_var->count - 1; i++)
            {
                temp_var->history[i] = temp_var->history[i + 1];
            }
            temp_var->count--;
        }
        
        // Add new value
        temp_var->history[temp_var->count].value = strdup(value);
        temp_var->history[temp_var->count].timestamp = temp_var->count;
        temp_var->count++;
        return;
    }
    
    if (entry)
    {
        // Convert existing variable to temporal
        if (entry->type == 0)
            free(entry->value.string_val);
        else if (entry->type == 1)
            ast_free(entry->value.list_val);
        else if (entry->type == 2)
            ast_free(entry->value.dict_val);
        else if (entry->type == 3)
            ast_free(entry->value.stack_val);
        else if (entry->type == 4)
            ast_free(entry->value.queue_val);
        else if (entry->type == 5)
            ast_free(entry->value.linked_list_val);
        else if (entry->type == 6)
            ast_free(entry->value.regex_val);
    }
    else
    {
        // New variable
        if (var_count >= MAX_VARS)
        {
            fprintf(stderr, "Maximum number of variables (%d) exceeded\n", MAX_VARS);
            exit(EXIT_FAILURE);
        }
        entry = &vars[var_count++];
        strncpy(entry->name, name, MAX_VAR_NAME_LEN);
        entry->name[MAX_VAR_NAME_LEN] = '\0';
    }
    
    // Initialize temporal variable
    entry->temporal_val = malloc(sizeof(TemporalVariable));
    entry->temporal_val->max_history = max_history;
    entry->temporal_val->count = 1;
    entry->temporal_val->current_index = 0;
    entry->temporal_val->history[0].value = strdup(value);
    entry->temporal_val->history[0].timestamp = 0;
    entry->type = 7;
}

const char *get_temporal_variable(const char *name, int time_offset)
{
    VarEntry *entry = find_variable(name);
    if (!entry || entry->type != 7)
    {
        fprintf(stderr, "Variable %s is not a temporal variable\n", name);
        return NULL;
    }
    
    TemporalVariable *temp_var = entry->temporal_val;
    
    // For time_offset = 0, return current value (most recent)
    // For time_offset = 1, return previous value, etc.
    int index = temp_var->count - 1 - time_offset;
    
    if (index < 0 || index >= temp_var->count)
    {
        fprintf(stderr, "Temporal offset %d out of range for variable %s (count: %d)\n", 
                time_offset, name, temp_var->count);
        return NULL;
    }
    
    return temp_var->history[index].value;
}

int get_temporal_variable_count(const char *name)
{
    VarEntry *entry = find_variable(name);
    if (!entry || entry->type != 7)
    {
        return 0;
    }
    return entry->temporal_val->count;
}

TemporalVariable *get_temporal_var_struct(const char *name)
{
    VarEntry *entry = find_variable(name);
    if (!entry || entry->type != 7)
    {
        return NULL;
    }
    return entry->temporal_val;
}