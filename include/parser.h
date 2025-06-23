#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"

void parser_init(const char* source);
ASTNode* parse_program();

#endif
