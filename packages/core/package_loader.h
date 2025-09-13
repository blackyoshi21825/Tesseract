#ifndef PACKAGE_LOADER_H
#define PACKAGE_LOADER_H

#include "../include/ast.h"

typedef struct {
    char *name;
    ASTNode *(*func)(ASTNode **args, int arg_count);
} PackageFunction;

typedef struct {
    char *name;
    PackageFunction *functions;
    int function_count;
} LoadedPackage;

int load_package(const char *package_name);
ASTNode *call_package_function(const char *func_name, ASTNode **args, int arg_count);
void register_package_function(const char *name, ASTNode *(*func)(ASTNode **args, int arg_count));
void import_package(const char *package_name);
int is_package_imported(const char *package_name);
void register_function_package_mapping(const char *function_name, const char *package_name);
const char* get_function_package(const char *function_name);

#endif