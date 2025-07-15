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
    } value;
    int type; // 0=string, 1=list, 2=dict, 3=stack
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