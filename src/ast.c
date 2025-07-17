#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

// --- AST Node Creation ---

ASTNode *ast_new_number(double value)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_NUMBER;
    node->number = value;
    return node;
}

ASTNode *ast_new_string(const char *str)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_STRING;
    strncpy(node->string, str, sizeof(node->string));
    node->string[sizeof(node->string) - 1] = '\0';
    return node;
}

ASTNode *ast_new_var(const char *name)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_VAR;
    strncpy(node->varname, name, sizeof(node->varname));
    node->varname[sizeof(node->varname) - 1] = '\0';
    return node;
}

ASTNode *ast_new_binop(ASTNode *left, ASTNode *right, TokenType op)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_BINOP;
    node->binop.left = left;
    node->binop.right = right;
    node->binop.op = op;
    return node;
}

ASTNode *ast_new_assign(const char *name, ASTNode *value)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_ASSIGN;
    strncpy(node->assign.varname, name, sizeof(node->assign.varname));
    node->assign.varname[sizeof(node->assign.varname) - 1] = '\0';
    node->assign.value = value;
    return node;
}

ASTNode *ast_new_if(ASTNode *cond, ASTNode *then_branch, ASTNode *elseif_branch, ASTNode *else_branch)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_IF;
    node->if_stmt.condition = cond;
    node->if_stmt.then_branch = then_branch;
    node->if_stmt.elseif_branch = elseif_branch;
    node->if_stmt.else_branch = else_branch;
    return node;
}

ASTNode *ast_new_loop(const char *varname, ASTNode *start, ASTNode *end, ASTNode *body)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_LOOP;
    strncpy(node->loop_stmt.varname, varname, sizeof(node->loop_stmt.varname));
    node->loop_stmt.varname[sizeof(node->loop_stmt.varname) - 1] = '\0';
    node->loop_stmt.start = start;
    node->loop_stmt.end = end;
    node->loop_stmt.body = body;
    return node;
}

ASTNode *ast_new_print(ASTNode *expr)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_PRINT;
    node->binop.left = expr;
    return node;
}

ASTNode *ast_new_block()
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_BLOCK;
    node->block.statements = NULL;
    node->block.count = 0;
    return node;
}

void ast_block_add_statement(ASTNode *block, ASTNode *statement)
{
    if (block->type != NODE_BLOCK)
        return;
    if (statement == NULL)
    {
        // Optionally, add a NOP node for empty statements, or just skip
        // ASTNode *nop = malloc(sizeof(ASTNode));
        // nop->type = NODE_NOP;
        // block->block.statements = realloc(block->block.statements, sizeof(ASTNode *) * (block->block.count + 1));
        // block->block.statements[block->block.count++] = nop;
        return;
    }
    block->block.statements = realloc(block->block.statements, sizeof(ASTNode *) * (block->block.count + 1));
    block->block.statements[block->block.count++] = statement;
}

ASTNode *ast_new_import(const char *filename)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_IMPORT;
    strncpy(node->string, filename, sizeof(node->string));
    node->string[sizeof(node->string) - 1] = '\0';
    return node;
}

ASTNode *ast_new_func_def(const char *name, char params[][64], int param_count, ASTNode *body)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_FUNC_DEF;
    strncpy(node->func_def.name, name, sizeof(node->func_def.name));
    node->func_def.name[sizeof(node->func_def.name) - 1] = '\0';
    node->func_def.param_count = param_count;
    for (int i = 0; i < param_count; i++)
    {
        strncpy(node->func_def.params[i], params[i], sizeof(node->func_def.params[i]));
        node->func_def.params[i][sizeof(node->func_def.params[i]) - 1] = '\0';
    }
    node->func_def.body = body;
    return node;
}

ASTNode *ast_new_func_call(const char *name, ASTNode **args, int arg_count)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_FUNC_CALL;
    strncpy(node->func_call.name, name, sizeof(node->func_call.name));
    node->func_call.name[sizeof(node->func_call.name) - 1] = '\0';
    node->func_call.arg_count = arg_count;
    for (int i = 0; i < arg_count; i++)
    {
        node->func_call.args[i] = args[i];
    }
    return node;
}

ASTNode *ast_new_list()
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_LIST;
    node->list.elements = NULL;
    node->list.count = 0;
    return node;
}

void ast_list_add_element(ASTNode *list, ASTNode *element)
{
    if (list->type != NODE_LIST)
        return;
    list->list.elements = realloc(list->list.elements, sizeof(ASTNode *) * (list->list.count + 1));
    list->list.elements[list->list.count++] = element;
}

ASTNode *ast_list_access(ASTNode *list, int index)
{
    if (list->type != NODE_LIST || index < 0 || index >= list->list.count)
        return NULL;
    return list->list.elements[index];
}

ASTNode *ast_list_slice(ASTNode *list, int start, int end)
{
    if (list->type != NODE_LIST || start < 0 || end > list->list.count || start > end)
        return NULL;
    ASTNode *new_list = ast_new_list();
    for (int i = start; i < end; i++)
    {
        ast_list_add_element(new_list, list->list.elements[i]);
    }
    return new_list;
}

ASTNode *ast_new_list_access(ASTNode *list, ASTNode *index)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_LIST_ACCESS;
    node->list_access.list = list;
    node->list_access.index = index;
    return node;
}

ASTNode *ast_new_list_len(ASTNode *list)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_LIST_LEN;
    node->list_access.list = list;
    return node;
}

ASTNode *ast_new_list_append(ASTNode *list, ASTNode *value)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_LIST_APPEND;
    node->binop.left = list;
    node->binop.right = value;
    return node;
}

ASTNode *ast_new_list_prepend(ASTNode *list, ASTNode *value)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_LIST_PREPEND;
    node->binop.left = list;
    node->binop.right = value;
    return node;
}

ASTNode *ast_new_list_pop(ASTNode *list)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_LIST_POP;
    node->list_access.list = list;
    return node;
}

ASTNode *ast_new_list_insert(ASTNode *list, ASTNode *index, ASTNode *value)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_LIST_INSERT;
    node->list_access.list = list;
    node->list_access.index = index;
    node->binop.left = value; // Reusing binop.left for the value
    return node;
}

ASTNode *ast_new_list_remove(ASTNode *list, ASTNode *value)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_LIST_REMOVE;
    node->binop.left = list;
    node->binop.right = value;
    return node;
}

ASTNode *ast_new_and(ASTNode *left, ASTNode *right)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_AND;
    node->binop.left = left;
    node->binop.right = right;
    return node;
}

ASTNode *ast_new_or(ASTNode *left, ASTNode *right)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_OR;
    node->binop.left = left;
    node->binop.right = right;
    return node;
}

ASTNode *ast_new_not(ASTNode *operand)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_NOT;
    node->unop.operand = operand;
    return node;
}

ASTNode *ast_new_bitwise_not(ASTNode *operand)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_BITWISE_NOT;
    node->unop.operand = operand;
    return node;
}

ASTNode *ast_new_bitwise_and(ASTNode *left, ASTNode *right)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_BITWISE_AND;
    node->binop.left = left;
    node->binop.right = right;
    node->binop.op = TOK_BITWISE_AND;
    return node;
}

ASTNode *ast_new_bitwise_or(ASTNode *left, ASTNode *right)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_BITWISE_OR;
    node->binop.left = left;
    node->binop.right = right;
    node->binop.op = TOK_BITWISE_OR;
    return node;
}

ASTNode *ast_new_bitwise_xor(ASTNode *left, ASTNode *right)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_BITWISE_XOR;
    node->binop.left = left;
    node->binop.right = right;
    node->binop.op = TOK_BITWISE_XOR;
    return node;
}
ASTNode *ast_new_pattern_match(ASTNode *pattern, ASTNode *noise)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_PATTERN_MATCH;
    node->pattern_match.pattern = pattern;
    node->pattern_match.noise = noise;
    return node;
}

ASTNode *ast_new_format_string(const char *format, ASTNode **args, int arg_count)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_FORMAT_STRING;
    strncpy(node->format_str.format, format, sizeof(node->format_str.format));
    node->format_str.format[sizeof(node->format_str.format) - 1] = '\0';
    node->format_str.arg_count = arg_count;
    for (int i = 0; i < arg_count; i++)
    {
        node->format_str.args[i] = args[i];
    }
    return node;
}

ASTNode *ast_new_class_def(const char *class_name, ASTNode *body)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_CLASS_DEF;
    strncpy(node->class_def.class_name, class_name, sizeof(node->class_def.class_name));
    node->class_def.class_name[sizeof(node->class_def.class_name) - 1] = '\0';
    node->class_def.body = body;
    return node;
}

ASTNode *ast_new_class_instance(const char *class_name, ASTNode **args, int arg_count)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_CLASS_INSTANCE;
    strncpy(node->class_instance.class_name, class_name, sizeof(node->class_instance.class_name));
    node->class_instance.class_name[sizeof(node->class_instance.class_name) - 1] = '\0';
    for (int i = 0; i < arg_count && i < 8; i++)
    {
        node->class_instance.args[i] = args[i];
    }
    node->class_instance.arg_count = arg_count;
    return node;
}

ASTNode *ast_new_member_access(ASTNode *object, const char *member_name)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_MEMBER_ACCESS;
    node->member_access.object = object;
    strncpy(node->member_access.member_name, member_name, sizeof(node->member_access.member_name));
    node->member_access.member_name[sizeof(node->member_access.member_name) - 1] = '\0';
    return node;
}

ASTNode *ast_new_method_def(const char *method_name, char params[][64], int param_count, ASTNode *body)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_METHOD_DEF;
    strncpy(node->method_def.method_name, method_name, sizeof(node->method_def.method_name));
    node->method_def.method_name[sizeof(node->method_def.method_name) - 1] = '\0';
    for (int i = 0; i < param_count && i < 4; i++)
    {
        strncpy(node->method_def.params[i], params[i], 64);
        node->method_def.params[i][63] = '\0';
    }
    node->method_def.param_count = param_count;
    node->method_def.body = body;
    return node;
}

ASTNode *ast_new_method_call(ASTNode *object, const char *method_name, ASTNode **args, int arg_count)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_METHOD_CALL;
    node->method_call.object = object;
    strncpy(node->method_call.method_name, method_name, sizeof(node->method_call.method_name));
    node->method_call.method_name[sizeof(node->method_call.method_name) - 1] = '\0';
    node->method_call.args = args;
    node->method_call.arg_count = arg_count;
    return node;
}

ASTNode *ast_new_member_assign(ASTNode *object, const char *member_name, ASTNode *value)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_MEMBER_ASSIGN;
    node->member_assign.object = object;
    strncpy(node->member_assign.member_name, member_name, sizeof(node->member_assign.member_name));
    node->member_assign.member_name[sizeof(node->member_assign.member_name) - 1] = '\0';
    node->member_assign.value = value;
    return node;
}

ASTNode *ast_new_dict()
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_DICT;
    node->dict.keys = NULL;
    node->dict.values = NULL;
    node->dict.count = 0;
    return node;
}

void ast_dict_add_pair(ASTNode *dict, ASTNode *key, ASTNode *value)
{
    if (dict->type != NODE_DICT) return;
    dict->dict.keys = realloc(dict->dict.keys, sizeof(ASTNode *) * (dict->dict.count + 1));
    dict->dict.values = realloc(dict->dict.values, sizeof(ASTNode *) * (dict->dict.count + 1));
    dict->dict.keys[dict->dict.count] = key;
    dict->dict.values[dict->dict.count] = value;
    dict->dict.count++;
}

ASTNode *ast_new_dict_get(ASTNode *dict, ASTNode *key)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_DICT_GET;
    node->dict_get.dict = dict;
    node->dict_get.key = key;
    return node;
}

ASTNode *ast_new_dict_set(ASTNode *dict, ASTNode *key, ASTNode *value)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_DICT_SET;
    node->dict_set.dict = dict;
    node->dict_set.key = key;
    node->dict_set.value = value;
    return node;
}

ASTNode *ast_new_dict_keys(ASTNode *dict)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_DICT_KEYS;
    node->dict_get.dict = dict;
    return node;
}

ASTNode *ast_new_dict_values(ASTNode *dict)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_DICT_VALUES;
    node->dict_get.dict = dict;
    return node;
}

ASTNode *ast_new_stack()
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_STACK;
    node->stack.elements = NULL;
    node->stack.count = 0;
    return node;
}

void ast_stack_add_element(ASTNode *stack, ASTNode *element)
{
    if (stack->type != NODE_STACK) return;
    stack->stack.elements = realloc(stack->stack.elements, sizeof(ASTNode *) * (stack->stack.count + 1));
    stack->stack.elements[stack->stack.count++] = element;
}

ASTNode *ast_new_stack_push(ASTNode *stack, ASTNode *value)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_STACK_PUSH;
    node->stack_push.stack = stack;
    node->stack_push.value = value;
    return node;
}

ASTNode *ast_new_stack_pop(ASTNode *stack)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_STACK_POP;
    node->stack_op.stack = stack;
    return node;
}

ASTNode *ast_new_stack_peek(ASTNode *stack)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_STACK_PEEK;
    node->stack_op.stack = stack;
    return node;
}

ASTNode *ast_new_stack_size(ASTNode *stack)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_STACK_SIZE;
    node->stack_op.stack = stack;
    return node;
}

ASTNode *ast_new_stack_empty(ASTNode *stack)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_STACK_EMPTY;
    node->stack_op.stack = stack;
    return node;
}

ASTNode *ast_new_queue()
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_QUEUE;
    node->queue.elements = NULL;
    node->queue.count = 0;
    return node;
}

void ast_queue_add_element(ASTNode *queue, ASTNode *element)
{
    if (queue->type != NODE_QUEUE) return;
    queue->queue.elements = realloc(queue->queue.elements, sizeof(ASTNode *) * (queue->queue.count + 1));
    queue->queue.elements[queue->queue.count++] = element;
}

ASTNode *ast_new_queue_enqueue(ASTNode *queue, ASTNode *value)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_QUEUE_ENQUEUE;
    node->queue_enqueue.queue = queue;
    node->queue_enqueue.value = value;
    return node;
}

ASTNode *ast_new_queue_dequeue(ASTNode *queue)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_QUEUE_DEQUEUE;
    node->queue_op.queue = queue;
    return node;
}

ASTNode *ast_new_queue_front(ASTNode *queue)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_QUEUE_FRONT;
    node->queue_op.queue = queue;
    return node;
}

ASTNode *ast_new_queue_back(ASTNode *queue)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_QUEUE_BACK;
    node->queue_op.queue = queue;
    return node;
}

ASTNode *ast_new_queue_isempty(ASTNode *queue)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_QUEUE_ISEMPTY;
    node->queue_op.queue = queue;
    return node;
}

ASTNode *ast_new_queue_size(ASTNode *queue)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_QUEUE_SIZE;
    node->queue_op.queue = queue;
    return node;
}

ASTNode *ast_new_linked_list()
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_LINKED_LIST;
    node->linked_list.elements = NULL;
    node->linked_list.count = 0;
    return node;
}

void ast_linked_list_add_element(ASTNode *list, ASTNode *element)
{
    if (list->type != NODE_LINKED_LIST) return;
    list->linked_list.elements = realloc(list->linked_list.elements, sizeof(ASTNode *) * (list->linked_list.count + 1));
    list->linked_list.elements[list->linked_list.count++] = element;
}

ASTNode *ast_new_linked_list_add(ASTNode *list, ASTNode *value)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_LINKED_LIST_ADD;
    node->linked_list_op.list = list;
    node->linked_list_op.value = value;
    return node;
}

ASTNode *ast_new_linked_list_remove(ASTNode *list, ASTNode *value)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_LINKED_LIST_REMOVE;
    node->linked_list_op.list = list;
    node->linked_list_op.value = value;
    return node;
}

ASTNode *ast_new_linked_list_get(ASTNode *list, ASTNode *index)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_LINKED_LIST_GET;
    node->linked_list_get.list = list;
    node->linked_list_get.index = index;
    return node;
}

ASTNode *ast_new_linked_list_size(ASTNode *list)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_LINKED_LIST_SIZE;
    node->linked_list_op.list = list;
    return node;
}

ASTNode *ast_new_linked_list_isempty(ASTNode *list)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_LINKED_LIST_ISEMPTY;
    node->linked_list_op.list = list;
    return node;
}

// --- AST Free ---

void ast_free(ASTNode *node)
{
    if (!node)
        return;
    switch (node->type)
    {
    case NODE_BINOP:
        ast_free(node->binop.left);
        ast_free(node->binop.right);
        break;
    case NODE_ASSIGN:
        ast_free(node->assign.value);
        break;
    case NODE_IF:
        ast_free(node->if_stmt.condition);
        ast_free(node->if_stmt.then_branch);
        ast_free(node->if_stmt.elseif_branch);
        ast_free(node->if_stmt.else_branch);
        break;
    case NODE_LOOP:
        ast_free(node->loop_stmt.start);
        ast_free(node->loop_stmt.end);
        ast_free(node->loop_stmt.body);
        break;
    case NODE_PRINT:
        ast_free(node->binop.left);
        break;
    case NODE_BLOCK:
        for (int i = 0; i < node->block.count; i++)
        {
            ast_free(node->block.statements[i]);
        }
        free(node->block.statements);
        break;
    case NODE_FUNC_DEF:
        ast_free(node->func_def.body);
        break;
    case NODE_FUNC_CALL:
        for (int i = 0; i < node->func_call.arg_count; i++)
        {
            ast_free(node->func_call.args[i]);
        }
        break;
    case NODE_LIST:
        for (int i = 0; i < node->list.count; i++)
        {
            ast_free(node->list.elements[i]);
        }
        free(node->list.elements);
        break;
    case NODE_LIST_ACCESS:
        ast_free(node->list_access.list);
        ast_free(node->list_access.index);
        break;
    case NODE_NOT:
        ast_free(node->unop.operand);
        break;
    case NODE_BITWISE_NOT:
        ast_free(node->unop.operand);
        break;
    case NODE_FORMAT_STRING:
        for (int i = 0; i < node->format_str.arg_count; i++)
        {
            ast_free(node->format_str.args[i]);
        }
        break;
    case NODE_MEMBER_ASSIGN:
        ast_free(node->member_assign.object);
        ast_free(node->member_assign.value);
        break;
    case NODE_DICT:
        for (int i = 0; i < node->dict.count; i++)
        {
            ast_free(node->dict.keys[i]);
            ast_free(node->dict.values[i]);
        }
        free(node->dict.keys);
        free(node->dict.values);
        break;
    case NODE_DICT_GET:
    case NODE_DICT_KEYS:
    case NODE_DICT_VALUES:
        ast_free(node->dict_get.dict);
        if (node->type == NODE_DICT_GET)
            ast_free(node->dict_get.key);
        break;
    case NODE_DICT_SET:
        ast_free(node->dict_set.dict);
        ast_free(node->dict_set.key);
        ast_free(node->dict_set.value);
        break;
    case NODE_STACK:
        for (int i = 0; i < node->stack.count; i++)
        {
            ast_free(node->stack.elements[i]);
        }
        free(node->stack.elements);
        break;
    case NODE_STACK_PUSH:
        ast_free(node->stack_push.stack);
        ast_free(node->stack_push.value);
        break;
    case NODE_STACK_POP:
    case NODE_STACK_PEEK:
    case NODE_STACK_SIZE:
    case NODE_STACK_EMPTY:
        ast_free(node->stack_op.stack);
        break;
    case NODE_QUEUE:
        for (int i = 0; i < node->queue.count; i++)
        {
            ast_free(node->queue.elements[i]);
        }
        free(node->queue.elements);
        break;
    case NODE_QUEUE_ENQUEUE:
        ast_free(node->queue_enqueue.queue);
        ast_free(node->queue_enqueue.value);
        break;
    case NODE_QUEUE_DEQUEUE:
    case NODE_QUEUE_FRONT:
    case NODE_QUEUE_BACK:
    case NODE_QUEUE_ISEMPTY:
    case NODE_QUEUE_SIZE:
        ast_free(node->queue_op.queue);
        break;
    case NODE_LINKED_LIST:
        for (int i = 0; i < node->linked_list.count; i++)
        {
            ast_free(node->linked_list.elements[i]);
        }
        free(node->linked_list.elements);
        break;
    case NODE_LINKED_LIST_ADD:
    case NODE_LINKED_LIST_REMOVE:
        ast_free(node->linked_list_op.list);
        ast_free(node->linked_list_op.value);
        break;
    case NODE_LINKED_LIST_GET:
        ast_free(node->linked_list_get.list);
        ast_free(node->linked_list_get.index);
        break;
    case NODE_LINKED_LIST_SIZE:
    case NODE_LINKED_LIST_ISEMPTY:
        ast_free(node->linked_list_op.list);
        break;
    default:
        break;
    }
    free(node);
}

// --- Simple Variable Table for Evaluation ---

typedef struct Var
{
    char name[64];
    double value;
    struct Var *next;
} Var;

static Var *var_list = NULL;

static double get_var_value(const char *name)
{
    for (Var *v = var_list; v != NULL; v = v->next)
    {
        if (strcmp(v->name, name) == 0)
        {
            return v->value;
        }
    }
    fprintf(stderr, "Runtime error: Undefined variable '%s'\n", name);
    exit(1);
}

static void set_var_value(const char *name, double value)
{
    for (Var *v = var_list; v != NULL; v = v->next)
    {
        if (strcmp(v->name, name) == 0)
        {
            v->value = value;
            return;
        }
    }
    Var *new_var = malloc(sizeof(Var));
    strncpy(new_var->name, name, sizeof(new_var->name));
    new_var->name[sizeof(new_var->name) - 1] = '\0';
    new_var->value = value;
    new_var->next = var_list;
    var_list = new_var;
}

// --- AST Evaluation Function ---

double ast_eval(ASTNode *node)
{
    if (!node)
    {
        fprintf(stderr, "Runtime error: Null AST node in eval\n");
        exit(1);
    }

    switch (node->type)
    {
    case NODE_NUMBER:
        return node->number;

    case NODE_VAR:
        return get_var_value(node->varname);

    case NODE_ASSIGN:
    {
        double val = ast_eval(node->assign.value);
        set_var_value(node->assign.varname, val);
        return val;
    }

    case NODE_BINOP:
    {
        double lhs = ast_eval(node->binop.left);
        double rhs = ast_eval(node->binop.right);

        switch (node->binop.op)
        {
        case TOK_PLUS:
            return lhs + rhs;
        case TOK_MINUS:
            return lhs - rhs;
        case TOK_MUL:
            return lhs * rhs;
        case TOK_DIV:
            if (rhs == 0)
            {
                fprintf(stderr, "Runtime error: Division by zero\n");
                exit(1);
            }
            return lhs / rhs;
        default:
            fprintf(stderr, "Runtime error: Unknown binary operator %d\n", node->binop.op);
            exit(1);
        }
    }

    case NODE_PRINT:
    {
        double val = ast_eval(node->binop.left);
        printf("%g\n", val);
        return val;
    }

    case NODE_BLOCK:
    {
        double last = 0;
        for (int i = 0; i < node->block.count; i++)
        {
            last = ast_eval(node->block.statements[i]);
        }
        return last;
    }

    case NODE_AND:
    {
        double lhs = ast_eval(node->binop.left);
        double rhs = ast_eval(node->binop.right);
        return lhs && rhs;
    }

    case NODE_OR:
    {
        double lhs = ast_eval(node->binop.left);
        double rhs = ast_eval(node->binop.right);
        return lhs || rhs;
    }

    case NODE_NOT:
    {
        double val = ast_eval(node->unop.operand);
        return !val;
    }

    case NODE_BITWISE_NOT:
    {
        double val = ast_eval(node->unop.operand);
        return ~(int)val; // Assuming the value is an integer for bitwise operations
    }

    case NODE_CLASS_DEF:
    {
        // Ignore class definitions in ast_eval
        return 0;
    }
    default:
        fprintf(stderr, "Runtime error: Unsupported AST node type %d\n", node->type);
        exit(1);
    }
}