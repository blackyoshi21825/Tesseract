#ifndef AST_H
#define AST_H

#include "lexer.h"

typedef enum
{
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
    NODE_LIST,
    NODE_LIST_ACCESS,
    NODE_LIST_LEN,
    NODE_LIST_APPEND,
    NODE_LIST_PREPEND,
    NODE_LIST_POP,
    NODE_LIST_INSERT,
    NODE_LIST_REMOVE,
    NODE_AND,
    NODE_OR,
    NODE_NOT,
    NODE_BITWISE_AND,
    NODE_BITWISE_OR,
    NODE_BITWISE_XOR,
    NODE_BITWISE_NOT,
    NODE_NOP
} NodeType;

typedef struct ASTNode ASTNode;

struct ASTNode
{
    NodeType type;
    union
    {
        double number; // Directly store the number here
        char string[64];
        char varname[64];
        struct
        {
            ASTNode *left;
            ASTNode *right;
            TokenType op;
        } binop;
        struct
        {
            ASTNode *operand;
        } unop;
        struct
        {
            char varname[64];
            ASTNode *value;
        } assign;
        struct
        {
            ASTNode *condition;
            ASTNode *then_branch;
            ASTNode *elseif_branch;
            ASTNode *else_branch;
        } if_stmt;
        struct
        {
            char varname[64];
            ASTNode *start;
            ASTNode *end;
            ASTNode *body;
        } loop_stmt;
        struct
        {
            ASTNode **statements;
            int count;
        } block;
        struct
        {
            char name[64];
            char params[4][64];
            int param_count;
            ASTNode *body;
        } func_def;
        struct
        {
            char name[64];
            ASTNode *args[4];
            int arg_count;
        } func_call;
        struct
        {
            ASTNode **elements;
            int count;
        } list;
        struct
        {
            ASTNode *list;
            ASTNode *index;
        } list_access;
        struct
        {
            ASTNode *list;
            ASTNode *start;
            ASTNode *end;
        } list_slice;
    };
};

ASTNode *ast_new_number(double value);
ASTNode *ast_new_string(const char *str);
ASTNode *ast_new_var(const char *name);
ASTNode *ast_new_binop(ASTNode *left, ASTNode *right, TokenType op);
ASTNode *ast_new_assign(const char *name, ASTNode *value);
ASTNode *ast_new_if(ASTNode *cond, ASTNode *then_branch, ASTNode *elseif_branch, ASTNode *else_branch);
ASTNode *ast_new_loop(const char *varname, ASTNode *start, ASTNode *end, ASTNode *body);
ASTNode *ast_new_print(ASTNode *expr);
ASTNode *ast_new_block();
ASTNode *ast_new_import(const char *filename);
ASTNode *ast_new_func_def(const char *name, char params[][64], int param_count, ASTNode *body);
ASTNode *ast_new_func_call(const char *name, ASTNode **args, int arg_count);
ASTNode *ast_new_list();
ASTNode *ast_new_list_len(ASTNode *list);
ASTNode *ast_new_list_append(ASTNode *list, ASTNode *value);
ASTNode *ast_new_list_prepend(ASTNode *list, ASTNode *value);
ASTNode *ast_new_list_pop(ASTNode *list);
ASTNode *ast_new_list_insert(ASTNode *list, ASTNode *index, ASTNode *value);
ASTNode *ast_new_list_remove(ASTNode *list, ASTNode *value);
void ast_list_add_element(ASTNode *list, ASTNode *element);
ASTNode *ast_new_list_access(ASTNode *list, ASTNode *index);
void ast_block_add_statement(ASTNode *block, ASTNode *statement);
void ast_free(ASTNode *node);

ASTNode *ast_new_and(ASTNode *left, ASTNode *right);
ASTNode *ast_new_or(ASTNode *left, ASTNode *right);
ASTNode *ast_new_not(ASTNode *operand);

ASTNode *ast_new_bitwise_and(ASTNode *left, ASTNode *right);
ASTNode *ast_new_bitwise_or(ASTNode *left, ASTNode *right);
ASTNode *ast_new_bitwise_xor(ASTNode *left, ASTNode *right);
ASTNode *ast_new_bitwise_not(ASTNode *operand);

#endif