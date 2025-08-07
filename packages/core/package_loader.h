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

#endif