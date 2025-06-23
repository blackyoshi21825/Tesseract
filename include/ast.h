#ifndef AST_H
#define AST_H

#include "lexer.h"

typedef enum {
    NODE_NUMBER,
    NODE_STRING,
    NODE_VAR,
    NODE_BINOP,
    NODE_ASSIGN,
    NODE_IF,
    NODE_LOOP,
    NODE_IMPORT,
    NODE_PRINT,
    NODE_BLOCK,
    NODE_FUNC_DEF,
    NODE_FUNC_CALL,
    NODE_NOP
} NodeType;

typedef struct ASTNode ASTNode;

struct ASTNode {
    NodeType type;
    union {
        double number;
        char string[64];
        char varname[64];
        struct {
            ASTNode *left;
            ASTNode *right;
            TokenType op;
        } binop;
        struct {
            char varname[64];
            ASTNode *value;
        } assign;
        struct {
            ASTNode *condition;
            ASTNode *then_branch;
            ASTNode *elseif_branch;
            ASTNode *else_branch;
        } if_stmt;
        struct {
            char varname[64];
            ASTNode *start;
            ASTNode *end;
            ASTNode *body;
        } loop_stmt;
        struct {
            ASTNode **statements;
            int count;
        } block;
        struct {
            char name[64];
            char params[4][64];
            int param_count;
            ASTNode* body;
        } func_def;
        struct {
            char name[64];
            ASTNode* args[4];
            int arg_count;
        } func_call;
        struct {
            ASTNode** elements;
            int count;
        } list;
        struct {
            ASTNode* list;
            ASTNode* index;
        } list_access;
        struct {
            ASTNode* list;
            ASTNode* start;
            ASTNode* end;
        } list_slice;
    };
};

ASTNode* ast_new_number(double value);
ASTNode* ast_new_string(const char* str);
ASTNode* ast_new_var(const char* name);
ASTNode* ast_new_binop(ASTNode* left, ASTNode* right, TokenType op);
ASTNode* ast_new_assign(const char* name, ASTNode* value);
ASTNode* ast_new_if(ASTNode* cond, ASTNode* then_branch, ASTNode* elseif_branch, ASTNode* else_branch);
ASTNode* ast_new_loop(const char* varname, ASTNode* start, ASTNode* end, ASTNode* body);
ASTNode* ast_new_print(ASTNode* expr);
ASTNode* ast_new_block();
ASTNode* ast_new_import(const char* filename);
ASTNode* ast_new_func_def(const char* name, char params[][64], int param_count, ASTNode* body);
ASTNode* ast_new_func_call(const char* name, ASTNode** args, int arg_count);
void ast_block_add_statement(ASTNode* block, ASTNode* statement);
void ast_free(ASTNode* node);

#endif
