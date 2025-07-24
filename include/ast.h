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
    NODE_WHILE,
    NODE_SWITCH,
    NODE_CASE,
    NODE_IMPORT,
    NODE_PRINT,
    NODE_INPUT,
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
    NODE_DICT,                // Dictionary literal
    NODE_DICT_GET,            // Dictionary get operation
    NODE_DICT_SET,            // Dictionary set operation
    NODE_DICT_KEYS,           // Dictionary keys operation
    NODE_DICT_VALUES,         // Dictionary values operation
    NODE_STACK,               // Stack literal
    NODE_STACK_PUSH,          // Stack push operation
    NODE_STACK_POP,           // Stack pop operation
    NODE_STACK_PEEK,          // Stack peek operation
    NODE_STACK_SIZE,          // Stack size operation
    NODE_STACK_EMPTY,         // Stack empty check
    NODE_QUEUE,               // Queue literal
    NODE_QUEUE_ENQUEUE,       // Queue enqueue operation
    NODE_QUEUE_DEQUEUE,       // Queue dequeue operation
    NODE_QUEUE_FRONT,         // Queue front operation
    NODE_QUEUE_BACK,          // Queue back operation
    NODE_QUEUE_ISEMPTY,       // Queue isEmpty check
    NODE_QUEUE_SIZE,          // Queue size operation
    NODE_LINKED_LIST,         // Linked list literal
    NODE_LINKED_LIST_ADD,     // Add to linked list
    NODE_LINKED_LIST_REMOVE,  // Remove from linked list
    NODE_LINKED_LIST_GET,     // Get element from linked list
    NODE_LINKED_LIST_SIZE,    // Get size of linked list
    NODE_LINKED_LIST_ISEMPTY, // Check if linked list is empty
    NODE_FILE_OPEN,
    NODE_FILE_READ,
    NODE_FILE_WRITE,
    NODE_FILE_CLOSE,
    NODE_TO_STR,
    NODE_TO_INT,
    NODE_HTTP_GET,       // HTTP GET request
    NODE_HTTP_POST,      // HTTP POST request
    NODE_HTTP_PUT,       // HTTP PUT request
    NODE_HTTP_DELETE,    // HTTP DELETE request
    NODE_REGEX,          // Regex literal
    NODE_REGEX_MATCH,    // Regex match operation
    NODE_REGEX_REPLACE,  // Regex replace operation
    NODE_REGEX_FIND_ALL, // Regex find all operation
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
            ASTNode *condition;
            ASTNode *body;
        } while_stmt;
        struct
        {
            ASTNode *expression;
            ASTNode **cases;
            int case_count;
            ASTNode *default_case;
        } switch_stmt;
        struct
        {
            ASTNode *value;
            ASTNode *body;
        } case_stmt;
        struct
        {
            ASTNode *prompt;
        } input_stmt;
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
        struct
        {
            ASTNode **elements;
            int count;
        } stack;
        struct
        {
            ASTNode *stack;
            ASTNode *value;
        } stack_push;
        struct
        {
            ASTNode *stack;
        } stack_op;
        struct
        {
            ASTNode **elements;
            int count;
        } queue;
        struct
        {
            ASTNode *queue;
            ASTNode *value;
        } queue_enqueue;
        struct
        {
            ASTNode *queue;
        } queue_op;
        struct
        {
            ASTNode **elements;
            int count;
        } linked_list;
        struct
        {
            ASTNode *list;
            ASTNode *value;
        } linked_list_op;
        struct
        {
            ASTNode *list;
            ASTNode *index;
        } linked_list_get;
        struct
        {
            ASTNode *filename;
            ASTNode *mode;
        } file_open_stmt;
        struct
        {
            ASTNode *file_handle;
        } file_read_stmt;
        struct
        {
            ASTNode *file_handle;
            ASTNode *content;
        } file_write_stmt;
        struct
        {
            ASTNode *file_handle;
        } file_close_stmt;
        struct
        {
            ASTNode *url;
            ASTNode *headers; // Optional dictionary of headers
        } http_get;
        struct
        {
            ASTNode *url;
            ASTNode *data;
            ASTNode *headers; // Optional dictionary of headers
        } http_post;
        struct
        {
            ASTNode *url;
            ASTNode *data;
            ASTNode *headers; // Optional dictionary of headers
        } http_put;
        struct
        {
            ASTNode *url;
            ASTNode *headers; // Optional dictionary of headers
        } http_delete;
        struct
        {
            char pattern[256];
            char flags[16];
        } regex;
        struct
        {
            ASTNode *regex;
            ASTNode *text;
        } regex_match;
        struct
        {
            ASTNode *regex;
            ASTNode *text;
            ASTNode *replacement;
        } regex_replace;
        struct
        {
            ASTNode *regex;
            ASTNode *text;
        } regex_find_all;
    };
};

ASTNode *ast_new_number(double value);
ASTNode *ast_new_string(const char *str);
ASTNode *ast_new_var(const char *name);
ASTNode *ast_new_binop(ASTNode *left, ASTNode *right, TokenType op);
ASTNode *ast_new_assign(const char *name, ASTNode *value);
ASTNode *ast_new_if(ASTNode *cond, ASTNode *then_branch, ASTNode *elseif_branch, ASTNode *else_branch);
ASTNode *ast_new_loop(const char *varname, ASTNode *start, ASTNode *end, ASTNode *body);
ASTNode *ast_new_while(ASTNode *condition, ASTNode *body);
ASTNode *ast_new_switch(ASTNode *expression);
ASTNode *ast_new_case(ASTNode *value, ASTNode *body);
void ast_switch_add_case(ASTNode *switch_node, ASTNode *case_node);
void ast_switch_set_default(ASTNode *switch_node, ASTNode *default_body);
ASTNode *ast_new_print(ASTNode *expr);
ASTNode *ast_new_input(ASTNode *prompt);
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

ASTNode *ast_new_stack();
ASTNode *ast_new_stack_push(ASTNode *stack, ASTNode *value);
ASTNode *ast_new_stack_pop(ASTNode *stack);
ASTNode *ast_new_stack_peek(ASTNode *stack);
ASTNode *ast_new_stack_size(ASTNode *stack);
ASTNode *ast_new_stack_empty(ASTNode *stack);
void ast_stack_add_element(ASTNode *stack, ASTNode *element);

ASTNode *ast_new_queue();
ASTNode *ast_new_queue_enqueue(ASTNode *queue, ASTNode *value);
ASTNode *ast_new_queue_dequeue(ASTNode *queue);
ASTNode *ast_new_queue_front(ASTNode *queue);
ASTNode *ast_new_queue_back(ASTNode *queue);
ASTNode *ast_new_queue_isempty(ASTNode *queue);
ASTNode *ast_new_queue_size(ASTNode *queue);
void ast_queue_add_element(ASTNode *queue, ASTNode *element);

ASTNode *ast_new_linked_list();
ASTNode *ast_new_linked_list_add(ASTNode *list, ASTNode *value);
ASTNode *ast_new_linked_list_remove(ASTNode *list, ASTNode *value);
ASTNode *ast_new_linked_list_get(ASTNode *list, ASTNode *index);
ASTNode *ast_new_linked_list_size(ASTNode *list);
ASTNode *ast_new_linked_list_isempty(ASTNode *list);
void ast_linked_list_add_element(ASTNode *list, ASTNode *element);

ASTNode *ast_new_file_open(ASTNode *filename, ASTNode *mode);
ASTNode *ast_new_file_read(ASTNode *file_handle);
ASTNode *ast_new_file_write(ASTNode *file_handle, ASTNode *content);
ASTNode *ast_new_file_close(ASTNode *file_handle);

ASTNode *ast_new_to_str(ASTNode *value);
ASTNode *ast_new_to_int(ASTNode *value);

// HTTP request functions
ASTNode *ast_new_http_get(ASTNode *url, ASTNode *headers);
ASTNode *ast_new_http_post(ASTNode *url, ASTNode *data, ASTNode *headers);
ASTNode *ast_new_http_put(ASTNode *url, ASTNode *data, ASTNode *headers);
ASTNode *ast_new_http_delete(ASTNode *url, ASTNode *headers);

// Regex functions
ASTNode *ast_new_regex(const char *pattern, const char *flags);
ASTNode *ast_new_regex_match(ASTNode *regex, ASTNode *text);
ASTNode *ast_new_regex_replace(ASTNode *regex, ASTNode *text, ASTNode *replacement);
ASTNode *ast_new_regex_find_all(ASTNode *regex, ASTNode *text);

#endif
