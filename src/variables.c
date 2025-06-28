#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "variables.h"
#include "ast.h"

#define MAX_VARS 1024
#define MAX_VAR_NAME_LEN 63
#define MAX_STRING_LEN 1023

typedef struct
{
    char name[MAX_VAR_NAME_LEN + 1];
    union
    {
        char *string_val;  // For string values
        ASTNode *list_val; // For list values
    } value;
    int is_list; // Flag to indicate if this is a list
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
        if (entry->is_list)
        {
            ast_free(entry->value.list_val);
            entry->is_list = 0;
        }
        free(entry->value.string_val); // Free old string if it exists

        entry->value.string_val = strdup(value);
        if (!entry->value.string_val)
        {
            perror("Failed to allocate string value");
            exit(EXIT_FAILURE);
        }
        entry->is_list = 0;
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
    vars[var_count].is_list = 0;
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
        if (!entry->is_list)
        {
            free(entry->value.string_val);
        }
        else
        {
            ast_free(entry->value.list_val);
        }
        entry->value.list_val = list;
        entry->is_list = 1;
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
    vars[var_count].is_list = 1;
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
    if (entry->is_list)
    {
        fprintf(stderr, "Variable %s is a list, not a string\n", name);
        return NULL;
    }
    return entry->value.string_val;
}

ASTNode *get_list_variable(const char *name)
{
    printf("Debug: Attempting to get list variable: %s\n", name);
    VarEntry *entry = find_variable(name);
    if (!entry)
    {
        fprintf(stderr, "Debug: Undefined variable: %s\n", name);
        return NULL;
    }
    if (!entry->is_list)
    {
        fprintf(stderr, "Debug: Variable %s is not a list\n", name);
        return NULL;
    }
    printf("Debug: Successfully retrieved list variable: %s\n", name);
    return entry->value.list_val;
}