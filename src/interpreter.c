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

// Forward declaration of print_node
static void print_node(ASTNode *node);

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
        else if (value_node->type == NODE_LIST)
        {
            set_list_variable(root->assign.varname, value_node);
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
        if (root->binop.left->type == NODE_LIST_ACCESS)
        {
            double result = eval_expression(root->binop.left);
            printf("%g\n", result);
        }
        else
        {
            print_node(root->binop.left);
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
    else if (root->type == NODE_LIST)
    {
        // Do nothing here, as lists are handled in print_node
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
    case NODE_LIST:
    {
        print_node(node);
        return 0;
    }
    case NODE_LIST_ACCESS:
    {
        double index = eval_expression(node->list_access.index);
        int i = (int)index;
        ASTNode *list_node = node->list_access.list;

        // Handle variable list access
        if (list_node->type == NODE_VAR)
        {
            ASTNode *list = get_list_variable(list_node->varname);
            if (!list)
            {
                printf("Runtime error: Undefined list variable '%s'\n", list_node->varname);
                exit(1);
            }
            list_node = list;
        }

        if (list_node->type != NODE_LIST)
        {
            printf("Runtime error: List access only supported on list nodes\n");
            exit(1);
        }

        if (i < 0 || i >= list_node->list.count)
        {
            printf("Runtime error: List index out of bounds\n");
            exit(1);
        }
        return eval_expression(list_node->list.elements[i]);
    }
    default:
        printf("Runtime error: Unexpected AST node type in eval_expression\n");
        exit(1);
    }
}

// Function to convert a list to a string representation
static char *list_to_string(ASTNode *list)
{
    if (list->type != NODE_LIST)
    {
        return strdup("Not a list");
    }

    // Start with an opening bracket
    char *result = strdup("[");
    for (int i = 0; i < list->list.count; i++)
    {
        // Evaluate each element and convert it to a string
        double element_value = eval_expression(list->list.elements[i]);
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%g", element_value);

        // Append the element to the result
        result = realloc(result, strlen(result) + strlen(buffer) + 3); // +3 for ", " or "]\0"
        strcat(result, buffer);

        // Add a comma and space if not the last element
        if (i < list->list.count - 1)
        {
            strcat(result, ", ");
        }
    }

    // Close the bracket
    strcat(result, "]");
    return result;
}

// Modify your print logic to handle lists
static void print_node(ASTNode *node)
{
    switch (node->type)
    {
    case NODE_NUMBER:
        printf("%g\n", node->number);
        break;
    case NODE_STRING:
        printf("%s\n", node->string);
        break;
    case NODE_LIST:
    {
        char *list_str = list_to_string(node);
        printf("%s\n", list_str);
        free(list_str);
        break;
    }
    case NODE_VAR:
    {
        // Check if it's a list variable
        ASTNode *list = get_list_variable(node->varname);
        if (list)
        {
            char *list_str = list_to_string(list);
            printf("%s\n", list_str);
            free(list_str);
        }
        else
        {
            const char *val = get_variable(node->varname);
            if (val)
            {
                printf("%s\n", val);
            }
            else
            {
                printf("Undefined variable '%s'\n", node->varname);
            }
        }
        break;
    }
    default:
        printf("%g\n", eval_expression(node));
        break;
    }
}
