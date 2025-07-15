#ifndef VARIABLES_H
#define VARIABLES_H
#include "ast.h"

void set_variable(const char *name, const char *value);
void set_list_variable(const char *name, ASTNode *list);
void set_dict_variable(const char *name, ASTNode *dict);
void set_stack_variable(const char *name, ASTNode *stack);
const char *get_variable(const char *name);
ASTNode *get_list_variable(const char *name);
ASTNode *get_dict_variable(const char *name);
ASTNode *get_stack_variable(const char *name);

#endif
