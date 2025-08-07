#include "package_loader.h"
#include "../../include/ast.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static PackageFunction package_functions[32];
static int function_count = 0;

void register_package_function(const char *name, ASTNode *(*func)(ASTNode **args, int arg_count)) {
    if (function_count < 32) {
        package_functions[function_count].name = malloc(strlen(name) + 1);
        strcpy(package_functions[function_count].name, name);
        package_functions[function_count].func = func;
        function_count++;
    }
}

ASTNode *call_package_function(const char *func_name, ASTNode **args, int arg_count) {
    for (int i = 0; i < function_count; i++) {
        if (strcmp(package_functions[i].name, func_name) == 0) {
            return package_functions[i].func(args, arg_count);
        }
    }
    return NULL;
}

int load_package(const char *package_name) {
    printf("Package %s loaded\n", package_name);
    return 0;
}