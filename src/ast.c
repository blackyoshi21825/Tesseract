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

        printf("[DEBUG] BINOP: lhs=%g, rhs=%g, op=%d\n", lhs, rhs, node->binop.op);

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

        // TODO: Support other node types: IF, LOOP, FUNC_DEF, FUNC_CALL, IMPORT

    default:
        fprintf(stderr, "Runtime error: Unsupported AST node type %d\n", node->type);
        exit(1);
    }
}
