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
    NODE_PATTERN_MATCH,
    NODE_FORMAT_STRING,
    NODE_NOP,
    NODE_CLASS_DEF,      // Class definition
    NODE_CLASS_INSTANCE, // Class instance (object)
    NODE_MEMBER_ACCESS,  // Accessing a member (field or method)
    NODE_METHOD_DEF,     // Method definition inside a class
    NODE_METHOD_CALL,    // Method call on an object
    NODE_MEMBER_ASSIGN,
    NODE_DICT,           // Dictionary literal
    NODE_DICT_GET,       // Dictionary get operation
    NODE_DICT_SET,       // Dictionary set operation
    NODE_DICT_KEYS,      // Dictionary keys operation
    NODE_DICT_VALUES,    // Dictionary values operation
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
            char varname[256];
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
        struct
        {
            ASTNode *pattern;
            ASTNode *noise;
        } pattern_match;
        struct
        {
            char format[256];
            ASTNode *args[4];
            int arg_count;
        } format_str;
        struct
        {
            char class_name[64];
            ASTNode *body; // Block of class body (fields, methods)
        } class_def;
        struct
        {
            char class_name[64];
            ASTNode *args[8]; // Arguments for constructor (if any)
            int arg_count;
        } class_instance;
        struct
        {
            ASTNode *object;
            char member_name[64];
        } member_access;
        struct
        {
            char method_name[64];
            char params[4][64];
            int param_count;
            ASTNode *body;
        } method_def;
        struct
        {
            ASTNode *object;
            char method_name[64];
            ASTNode **args;
            int arg_count;
        } method_call;
        struct
        {
            ASTNode *object;
            char member_name[64];
            ASTNode *value;
        } member_assign;
        struct
        {
            ASTNode **keys;
            ASTNode **values;
            int count;
        } dict;
        struct
        {
            ASTNode *dict;
            ASTNode *key;
        } dict_get;
        struct
        {
            ASTNode *dict;
            ASTNode *key;
            ASTNode *value;
        } dict_set;
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

ASTNode *ast_new_pattern_match(ASTNode *pattern, ASTNode *noise);

ASTNode *ast_new_format_string(const char *format, ASTNode **args, int arg_count);

ASTNode *ast_new_class_def(const char *class_name, ASTNode *body);
ASTNode *ast_new_class_instance(const char *class_name, ASTNode **args, int arg_count);
ASTNode *ast_new_member_access(ASTNode *object, const char *member_name);
ASTNode *ast_new_method_def(const char *method_name, char params[][64], int param_count, ASTNode *body);
ASTNode *ast_new_method_call(ASTNode *object, const char *method_name, ASTNode **args, int arg_count);

ASTNode *ast_new_dict();
ASTNode *ast_new_dict_get(ASTNode *dict, ASTNode *key);
ASTNode *ast_new_dict_set(ASTNode *dict, ASTNode *key, ASTNode *value);
ASTNode *ast_new_dict_keys(ASTNode *dict);
ASTNode *ast_new_dict_values(ASTNode *dict);
void ast_dict_add_pair(ASTNode *dict, ASTNode *key, ASTNode *value);

#endif