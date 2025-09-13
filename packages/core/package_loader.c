#include "package_loader.h"
#include "../../include/ast.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    char *function_name;
    char *package_name;
} FunctionPackageMapping;

static PackageFunction package_functions[128];
static int function_count = 0;
static char imported_packages[32][64];
static int imported_count = 0;
static FunctionPackageMapping function_mappings[128];
static int mapping_count = 0;

void register_package_function(const char *name, ASTNode *(*func)(ASTNode **args, int arg_count)) {
    if (function_count < 128) {
        package_functions[function_count].name = malloc(strlen(name) + 1);
        strcpy(package_functions[function_count].name, name);
        package_functions[function_count].func = func;
        function_count++;
    }
}

void register_function_package_mapping(const char *function_name, const char *package_name) {
    if (mapping_count < 128) {
        function_mappings[mapping_count].function_name = malloc(strlen(function_name) + 1);
        strcpy(function_mappings[mapping_count].function_name, function_name);
        function_mappings[mapping_count].package_name = malloc(strlen(package_name) + 1);
        strcpy(function_mappings[mapping_count].package_name, package_name);
        mapping_count++;
    }
}

const char* get_function_package(const char *function_name) {
    for (int i = 0; i < mapping_count; i++) {
        if (strcmp(function_mappings[i].function_name, function_name) == 0) {
            return function_mappings[i].package_name;
        }
    }
    return NULL;
}

int is_package_imported(const char *package_name) {
    for (int i = 0; i < imported_count; i++) {
        if (strcmp(imported_packages[i], package_name) == 0) {
            return 1;
        }
    }
    return 0;
}

void import_package(const char *package_name) {
    if (imported_count < 32 && !is_package_imported(package_name)) {
        strcpy(imported_packages[imported_count], package_name);
        imported_count++;
    }
}

ASTNode *call_package_function(const char *func_name, ASTNode **args, int arg_count) {
    const char *required_package = get_function_package(func_name);
    if (required_package && !is_package_imported(required_package)) {
        return NULL;
    }
    
    for (int i = 0; i < function_count; i++) {
        if (strcmp(package_functions[i].name, func_name) == 0) {
            return package_functions[i].func(args, arg_count);
        }
    }
    return NULL;
}

int load_package(const char *package_name) {
    import_package(package_name);
    printf("Package %s imported\n", package_name);
    return 0;
}