#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"

typedef struct
{
    const char *name;
    ASTNode *(*func)(ASTNode **args, int arg_count);
} LibraryFunction;

void interpret(ASTNode *root);
void register_library(const char *name, LibraryFunction *functions, int count);

#endif
