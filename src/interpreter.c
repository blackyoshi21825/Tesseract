#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interpreter.h"
#include "variables.h"
#include "ast.h"
#include "parser.h"

#define MAX_FUNCTIONS 100

typedef struct
{
    char name[64];
    ASTNode *body;
    char params[4][64];
    int param_count;
} Function;

static Function function_table[MAX_FUNCTIONS];
static int function_count = 0;

static double eval_expression(ASTNode *node);

// Forward declaration for file reading
char *read_file(const char *filename);

void register_function(const char *name, char params[][64], int param_count, ASTNode *body)
{
    if (function_count >= MAX_FUNCTIONS)
    {
        printf("Function table overflow!\n");
        exit(1);
    }
    strcpy(function_table[function_count].name, name);
    function_table[function_count].body = body;
    function_table[function_count].param_count = param_count;
    for (int i = 0; i < param_count; i++)
    {
        strcpy(function_table[function_count].params[i], params[i]);
    }
    function_count++;
}

Function *find_function(const char *name)
{
    for (int i = 0; i < function_count; i++)
    {
        if (strcmp(function_table[i].name, name) == 0)
        {
            return &function_table[i];
        }
    }
    return NULL;
}

void interpret(ASTNode *root)
{
    if (root->type == NODE_BLOCK)
    {
        for (int i = 0; i < root->block.count; i++)
        {
            interpret(root->block.statements[i]);
        }
    }
    else if (root->type == NODE_ASSIGN)
    {
        ASTNode *value_node = root->assign.value;
        if (value_node->type == NODE_STRING)
        {
            set_variable(root->assign.varname, value_node->string);
        }
        else
        {
            double val = eval_expression(value_node);
            char buf[64];
            snprintf(buf, sizeof(buf), "%g", val);
            set_variable(root->assign.varname, buf);
        }
    }
    else if (root->type == NODE_PRINT)
    {
        ASTNode *expr = root->binop.left;
        if (expr->type == NODE_STRING)
        {
            printf("%s\n", expr->string);
        }
        else if (expr->type == NODE_VAR)
        {
            const char *val = get_variable(expr->varname);
            if (val)
            {
                printf("%s\n", val);
            }
            else
            {
                printf("Runtime error: Undefined variable '%s'\n", expr->varname);
                exit(1);
            }
        }
        else
        {
            double val = eval_expression(expr);
            printf("%g\n", val);
        }
    }
    else if (root->type == NODE_IF)
    {
        double cond = eval_expression(root->if_stmt.condition);
        if (cond != 0)
        {
            interpret(root->if_stmt.then_branch);
        }
        else if (root->if_stmt.elseif_branch)
        {
            interpret(root->if_stmt.elseif_branch);
        }
        else if (root->if_stmt.else_branch)
        {
            interpret(root->if_stmt.else_branch);
        }
    }
    else if (root->type == NODE_LOOP)
    {
        double start = eval_expression(root->loop_stmt.start);
        double end = eval_expression(root->loop_stmt.end);
        for (int i = (int)start; i <= (int)end; i++)
        {
            char buf[64];
            snprintf(buf, sizeof(buf), "%d", i);
            set_variable(root->loop_stmt.varname, buf);
            interpret(root->loop_stmt.body);
        }
    }
    else if (root->type == NODE_IMPORT)
    {
        char *source = read_file(root->string);
        if (!source)
        {
            printf("Import error: Could not open file '%s'\n", root->string);
            exit(1);
        }
        parser_init(source);
        ASTNode *import_root = parse_program();
        interpret(import_root);
        free(source);
    }
    else if (root->type == NODE_FUNC_DEF)
    {
        register_function(root->func_def.name, root->func_def.params, root->func_def.param_count, root->func_def.body);
    }
    else if (root->type == NODE_FUNC_CALL)
    {
        Function *fn = find_function(root->func_call.name);
        if (!fn)
        {
            printf("Runtime error: Undefined function '%s'\n", root->func_call.name);
            exit(1);
        }
        if (fn->param_count != root->func_call.arg_count)
        {
            printf("Runtime error: Function '%s' expects %d args but got %d\n",
                   root->func_call.name, fn->param_count, root->func_call.arg_count);
            exit(1);
        }

        for (int i = 0; i < fn->param_count; i++)
        {
            double val = eval_expression(root->func_call.args[i]);
            char buf[64];
            snprintf(buf, sizeof(buf), "%g", val);
            set_variable(fn->params[i], buf);
        }

        interpret(fn->body);
    }
    else
    {
        printf("Error: Unknown AST node type in interpret()\n");
        exit(1);
    }
}

static double eval_expression(ASTNode *node)
{
    switch (node->type)
    {
    case NODE_NUMBER:
        return node->number;
    case NODE_STRING:
    {
        char *endptr;
        double val = strtod(node->string, &endptr);
        if (endptr == node->string)
            return 0.0;
        return val;
    }
    case NODE_VAR:
    {
        const char *val = get_variable(node->varname);
        if (!val)
        {
            printf("Runtime error: Undefined variable '%s'\n", node->varname);
            exit(1);
        }
        char *endptr;
        double dval = strtod(val, &endptr);
        if (endptr == val)
            return 0.0;
        return dval;
    }
    case NODE_BINOP:
    {
        double left = eval_expression(node->binop.left);
        double right = eval_expression(node->binop.right);
        switch (node->binop.op)
        {
        case TOK_PLUS:
            return left + right;
        case TOK_MINUS:
            return left - right;
        case TOK_MUL:
            return left * right;
        case TOK_DIV:
            return left / right;
        case TOK_MOD:
            return (int)left % (int)right;
        case TOK_GT:
            return left > right;
        case TOK_LT:
            return left < right;
        case TOK_GTE:
            return left >= right;
        case TOK_LTE:
            return left <= right;
        case TOK_EQ:
            return left == right;
        case TOK_NEQ:
            return left != right;
        default:
            printf("Runtime error: Unknown operator in expression\n");
            exit(1);
        }
    }
    default:
        printf("Runtime error: Unexpected AST node type in eval_expression\n");
        exit(1);
    }
}
