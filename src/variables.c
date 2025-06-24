#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "variables.h"

#define MAX_VARS 1024

typedef struct {
    char name[64];
    char value[64];
} VarEntry;

static VarEntry vars[MAX_VARS];
static int var_count = 0;

void set_variable(const char* name, const char* value) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            strncpy(vars[i].value, value, sizeof(vars[i].value));
            vars[i].value[sizeof(vars[i].value)-1] = '\0';
            return;
        }
    }
    if (var_count < MAX_VARS) {
        strncpy(vars[var_count].name, name, sizeof(vars[var_count].name));
        vars[var_count].name[sizeof(vars[var_count].name)-1] = '\0';
        strncpy(vars[var_count].value, value, sizeof(vars[var_count].value));
        vars[var_count].value[sizeof(vars[var_count].value)-1] = '\0';
        var_count++;
    } else {
        fprintf(stderr, "Variable table full\n");
        exit(1);
    }
}

const char* get_variable(const char* name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            return vars[i].value;
        }
    }
    return NULL;
}
