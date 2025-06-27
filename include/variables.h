#ifndef VARIABLES_H
#define VARIABLES_H
#include "ast.h"

void set_variable(const char *name, const char *value);
void set_list_variable(const char *name, ASTNode *list);
const char *get_variable(const char *name);
ASTNode *get_list_variable(const char *name);

#endif
