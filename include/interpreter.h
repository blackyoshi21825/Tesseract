#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "object.h"

ObjectInstance *object_new(const char *class_name);
void object_free(ObjectInstance *obj);
void register_class(const char *name, ASTNode *class_node);
ASTNode *instantiate_class(const char *name, ASTNode **args, int arg_count);
ASTNode *get_class(const char *name);
void interpret(ASTNode *root);

#endif
