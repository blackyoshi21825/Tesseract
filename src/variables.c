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
        ASTNode *tree_val; // For tree values
        ASTNode *graph_val; // For graph values
    } value;
    int type; // 0=string, 1=list, 2=dict, 3=stack, 4=queue, 5=linked_list, 6=regex, 7=temporal, 8=set, 9=undef, 10=iterator, 11=tree, 12=graph
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
        // Auto-create UNDEF variable
        set_undef_variable(name);
        return NULL;
    }
    if (entry->type == 9) // UNDEF type
    {
        return NULL;
    }
    if (entry->type != 0)
    {
        // Don't print error for non-string variables, just return NULL
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

void set_set_variable(const char *name, ASTNode *set)
{
    if (strlen(name) > MAX_VAR_NAME_LEN)
    {
        fprintf(stderr, "Variable name too long: %s\n", name);
        return;
    }

    if (set->type != NODE_SET)
    {
        fprintf(stderr, "Attempt to set non-set value as set variable\n");
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
        entry->value.list_val = set; // Reuse list_val for set
        entry->type = 8; // New type for sets
        return;
    }

    if (var_count >= MAX_VARS)
    {
        fprintf(stderr, "Maximum number of variables (%d) exceeded\n", MAX_VARS);
        exit(EXIT_FAILURE);
    }

    strncpy(vars[var_count].name, name, MAX_VAR_NAME_LEN);
    vars[var_count].name[MAX_VAR_NAME_LEN] = '\0';
    vars[var_count].value.list_val = set;
    vars[var_count].type = 8;
    var_count++;
}

ASTNode *get_set_variable(const char *name)
{
    VarEntry *entry = find_variable(name);
    if (!entry || entry->type != 8)
    {
        return NULL;
    }
    return entry->value.list_val;
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

void set_undef_variable(const char *name)
{
    if (strlen(name) > MAX_VAR_NAME_LEN)
    {
        fprintf(stderr, "Variable name too long: %s\n", name);
        return;
    }

    VarEntry *entry = find_variable(name);
    if (entry)
    {
        // Free existing value
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
        else if (entry->type == 7)
        {
            for (int i = 0; i < entry->temporal_val->count; i++)
            {
                free(entry->temporal_val->history[i].value);
            }
            free(entry->temporal_val);
        }
        
        entry->type = 9; // UNDEF type
        entry->value.string_val = NULL;
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
    vars[var_count].value.string_val = NULL;
    vars[var_count].type = 9; // UNDEF type
    var_count++;
}

int is_undef_variable(const char *name)
{
    VarEntry *entry = find_variable(name);
    if (!entry)
    {
        // Auto-create UNDEF variable
        set_undef_variable(name);
        return 1;
    }
    return entry->type == 9;
}

// Generator and iterator implementation
#define MAX_GENERATORS 1000
static Generator generators[MAX_GENERATORS];
static int generator_count = 0;

void register_generator(const char *name, char params[][64], int param_count, ASTNode *body)
{
    if (generator_count >= MAX_GENERATORS)
    {
        fprintf(stderr, "Maximum number of generators (%d) exceeded\n", MAX_GENERATORS);
        exit(EXIT_FAILURE);
    }
    
    strncpy(generators[generator_count].name, name, sizeof(generators[generator_count].name));
    generators[generator_count].name[sizeof(generators[generator_count].name) - 1] = '\0';
    generators[generator_count].body = body;
    generators[generator_count].param_count = param_count;
    
    for (int i = 0; i < param_count && i < 4; i++)
    {
        strncpy(generators[generator_count].params[i], params[i], sizeof(generators[generator_count].params[i]));
        generators[generator_count].params[i][sizeof(generators[generator_count].params[i]) - 1] = '\0';
    }
    
    generator_count++;
}

Generator *find_generator(const char *name)
{
    for (int i = 0; i < generator_count; i++)
    {
        if (strcmp(generators[i].name, name) == 0)
        {
            return &generators[i];
        }
    }
    return NULL;
}

void set_iterator_variable(const char *name, Iterator *iterator)
{
    if (strlen(name) > MAX_VAR_NAME_LEN)
    {
        fprintf(stderr, "Variable name too long: %s\n", name);
        return;
    }

    VarEntry *entry = find_variable(name);
    if (entry)
    {
        // Free existing value
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
        else if (entry->type == 7)
        {
            for (int i = 0; i < entry->temporal_val->count; i++)
            {
                free(entry->temporal_val->history[i].value);
            }
            free(entry->temporal_val);
        }
        else if (entry->type == 10) // Iterator type
        {
            free_iterator((Iterator*)entry->value.string_val);
        }
        
        entry->type = 10; // Iterator type
        entry->value.string_val = (char*)iterator; // Store iterator pointer
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
    vars[var_count].value.string_val = (char*)iterator;
    vars[var_count].type = 10; // Iterator type
    var_count++;
}

Iterator *get_iterator_variable(const char *name)
{
    VarEntry *entry = find_variable(name);
    if (!entry || entry->type != 10)
    {
        return NULL;
    }
    return (Iterator*)entry->value.string_val;
}

Iterator *create_iterator(Generator *gen, ASTNode **args, int arg_count)
{
    if (!gen)
        return NULL;
        
    Iterator *iter = malloc(sizeof(Iterator));
    if (!iter)
    {
        fprintf(stderr, "Failed to allocate iterator\n");
        return NULL;
    }
    
    iter->generator = gen;
    iter->arg_count = arg_count;
    iter->current_position = 0;
    iter->is_exhausted = 0;
    iter->current_yield_value = NULL;
    
    // Copy argument values
    if (arg_count > 0)
    {
        iter->arg_values = malloc(sizeof(ASTNode*) * arg_count);
        for (int i = 0; i < arg_count; i++)
        {
            iter->arg_values[i] = args[i];
        }
    }
    else
    {
        iter->arg_values = NULL;
    }
    
    return iter;
}

void free_iterator(Iterator *iter)
{
    if (!iter)
        return;
        
    if (iter->arg_values)
    {
        free(iter->arg_values);
    }
    
    if (iter->current_yield_value)
    {
        ast_free(iter->current_yield_value);
    }
    
    free(iter);
}

// Simple iterator implementation - executes generator body and captures yield values
ASTNode *iterator_next(Iterator *iter)
{
    if (!iter || iter->is_exhausted)
        return NULL;
        
    // For this simple implementation, we'll create a basic range generator
    // In a full implementation, this would execute the generator body and handle yield statements
    
    // Simple example: if generator is "range", generate numbers
    if (strcmp(iter->generator->name, "range") == 0)
    {
        if (iter->current_position >= 10) // Simple limit
        {
            iter->is_exhausted = 1;
            return NULL;
        }
        
        ASTNode *result = ast_new_number(iter->current_position);
        iter->current_position++;
        return result;
    }
    
    // For other generators, mark as exhausted for now
    iter->is_exhausted = 1;
    return NULL;
}

void set_tree_variable(const char *name, ASTNode *tree)
{
    if (strlen(name) > MAX_VAR_NAME_LEN)
    {
        fprintf(stderr, "Variable name too long: %s\n", name);
        return;
    }

    if (tree->type != NODE_TREE)
    {
        fprintf(stderr, "Attempt to set non-tree value as tree variable\n");
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
        else if (entry->type == 11)
            ast_free(entry->value.tree_val);
        else if (entry->type == 12)
            ast_free(entry->value.graph_val);
        entry->value.tree_val = tree;
        entry->type = 11;
        return;
    }

    if (var_count >= MAX_VARS)
    {
        fprintf(stderr, "Maximum number of variables (%d) exceeded\n", MAX_VARS);
        exit(EXIT_FAILURE);
    }

    strncpy(vars[var_count].name, name, MAX_VAR_NAME_LEN);
    vars[var_count].name[MAX_VAR_NAME_LEN] = '\0';
    vars[var_count].value.tree_val = tree;
    vars[var_count].type = 11;
    var_count++;
}

ASTNode *get_tree_variable(const char *name)
{
    VarEntry *entry = find_variable(name);
    if (!entry || entry->type != 11)
    {
        return NULL;
    }
    return entry->value.tree_val;
}

void set_graph_variable(const char *name, ASTNode *graph)
{
    if (strlen(name) > MAX_VAR_NAME_LEN)
    {
        fprintf(stderr, "Variable name too long: %s\n", name);
        return;
    }

    if (graph->type != NODE_GRAPH)
    {
        fprintf(stderr, "Attempt to set non-graph value as graph variable\n");
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
        else if (entry->type == 11)
            ast_free(entry->value.tree_val);
        else if (entry->type == 12)
            ast_free(entry->value.graph_val);
        entry->value.graph_val = graph;
        entry->type = 12;
        return;
    }

    if (var_count >= MAX_VARS)
    {
        fprintf(stderr, "Maximum number of variables (%d) exceeded\n", MAX_VARS);
        exit(EXIT_FAILURE);
    }

    strncpy(vars[var_count].name, name, MAX_VAR_NAME_LEN);
    vars[var_count].name[MAX_VAR_NAME_LEN] = '\0';
    vars[var_count].value.graph_val = graph;
    vars[var_count].type = 12;
    var_count++;
}

ASTNode *get_graph_variable(const char *name)
{
    VarEntry *entry = find_variable(name);
    if (!entry || entry->type != 12)
    {
        return NULL;
    }
    return entry->value.graph_val;
}