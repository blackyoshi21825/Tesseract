#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "interpreter.h"
#include "variables.h"
#include "ast.h"
#include "parser.h"

#define MAX_FUNCTIONS 1000000

const char *bool_to_str(bool value);

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
static char *list_to_string(ASTNode *list);

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
    else if (root->type == NODE_LIST_APPEND || root->type == NODE_LIST_PREPEND ||
             root->type == NODE_LIST_POP || root->type == NODE_LIST_INSERT ||
             root->type == NODE_LIST_REMOVE)
    {
        eval_expression(root);
    }
    else if (root->type == NODE_PATTERN_MATCH)
    {
        eval_expression(root); // Call eval_expression without expecting a return value
    }
    else
    {
        printf("Error: Unknown AST node type in interpret()\n");
        exit(1);
    }
}

static double eval_expression(ASTNode *node)
{
    if (!node)
    {
        fprintf(stderr, "Runtime error: Null AST node in eval\n");
        exit(1);
    }

    switch (node->type)
    {
    case NODE_NUMBER:
        return node->number; // Directly access the number field

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
            if (right == 0)
            {
                fprintf(stderr, "Runtime error: Division by zero\n");
                exit(1);
            }
            return left / right;
        case TOK_MOD:
            return fmod(left, right);
        default:
            fprintf(stderr, "Runtime error: Unknown binary operator %d\n", node->binop.op);
            exit(1);
        }
    }

    case NODE_BITWISE_NOT:
    {
        double operand = eval_expression(node->unop.operand);
        return (double)(~(int)operand);
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

        ASTNode *element = list_node->list.elements[i];
        if (element->type == NODE_NUMBER)
        {
            return element->number;
        }
        else if (element->type == NODE_STRING)
        {
            printf("%s\n", element->string);
            return 0; // Return 0 for string access
        }
        else
        {
            printf("Runtime error: Unsupported list element type\n");
            exit(1);
        }
    }

    case NODE_LIST_LEN:
    {
        ASTNode *list_node = node->list_access.list;
        if (list_node->type == NODE_VAR)
        {
            list_node = get_list_variable(list_node->varname);
            if (!list_node)
            {
                printf("Runtime error: Undefined list variable\n");
                exit(1);
            }
        }
        if (list_node->type != NODE_LIST)
        {
            printf("Runtime error: len() expects a list\n");
            exit(1);
        }
        return list_node->list.count;
    }

    case NODE_LIST_APPEND:
    {
        ASTNode *list_node = node->binop.left;
        ASTNode *value_node = node->binop.right;

        if (list_node->type == NODE_VAR)
        {
            ASTNode *list = get_list_variable(list_node->varname);
            if (!list)
            {
                printf("Runtime error: Undefined list variable\n");
                exit(1);
            }
            list_node = list;
        }

        if (list_node->type != NODE_LIST)
        {
            printf("Runtime error: append() expects a list\n");
            exit(1);
        }

        ast_list_add_element(list_node, value_node);
        return 0; // Return success
    }

    case NODE_LIST_PREPEND:
    {
        ASTNode *list_node = node->binop.left;
        ASTNode *value_node = node->binop.right;

        if (list_node->type == NODE_VAR)
        {
            ASTNode *list = get_list_variable(list_node->varname);
            if (!list)
            {
                printf("Runtime error: Undefined list variable\n");
                exit(1);
            }
            list_node = list;
        }

        if (list_node->type != NODE_LIST)
        {
            printf("Runtime error: prepend() expects a list\n");
            exit(1);
        }

        // Shift all elements to make room at the beginning
        list_node->list.elements = realloc(list_node->list.elements,
                                           sizeof(ASTNode *) * (list_node->list.count + 1));
        for (int i = list_node->list.count; i > 0; i--)
        {
            list_node->list.elements[i] = list_node->list.elements[i - 1];
        }
        list_node->list.elements[0] = value_node;
        list_node->list.count++;
        return 0; // Return success
    }

    case NODE_LIST_POP:
    {
        ASTNode *list_node = node->list_access.list;
        if (list_node->type == NODE_VAR)
        {
            ASTNode *list = get_list_variable(list_node->varname);
            if (!list)
            {
                printf("Runtime error: Undefined list variable\n");
                exit(1);
            }
            list_node = list;
        }

        if (list_node->type != NODE_LIST || list_node->list.count == 0)
        {
            printf("Runtime error: pop() expects a non-empty list\n");
            exit(1);
        }

        double val = eval_expression(list_node->list.elements[list_node->list.count - 1]);
        list_node->list.count--;
        return val;
    }

    case NODE_LIST_INSERT:
    {
        ASTNode *list_node = node->list_access.list;
        ASTNode *index_node = node->list_access.index;
        ASTNode *value_node = node; // Use the current node as the value node

        if (list_node->type != NODE_LIST)
        {
            fprintf(stderr, "Runtime error: insert() expects a list\n");
            exit(1);
        }

        int index = (int)eval_expression(index_node);
        if (index < 0 || index > list_node->list.count)
        {
            fprintf(stderr, "Runtime error: Index out of bounds in insert()\n");
            exit(1);
        }

        ASTNode *new_value = ast_new_number(eval_expression(value_node));

        // Create a new array with one extra element
        ASTNode **new_elements = malloc(sizeof(ASTNode *) * (list_node->list.count + 1));

        // Copy elements before the insertion point
        for (int i = 0; i < index; i++)
        {
            new_elements[i] = list_node->list.elements[i];
        }

        // Insert the new value
        new_elements[index] = new_value;

        // Copy elements after the insertion point
        for (int i = index; i < list_node->list.count; i++)
        {
            new_elements[i + 1] = list_node->list.elements[i];
        }

        // Free the old array and assign the new one
        free(list_node->list.elements);
        list_node->list.elements = new_elements;
        list_node->list.count++;

        return 0;
    }

    case NODE_LIST_REMOVE:
    {
        ASTNode *list_node = node->binop.left;
        ASTNode *value_node = node->binop.right;

        if (list_node->type == NODE_VAR)
        {
            ASTNode *list = get_list_variable(list_node->varname);
            if (!list)
            {
                printf("Runtime error: Undefined list variable\n");
                exit(1);
            }
            list_node = list;
        }

        if (list_node->type != NODE_LIST)
        {
            printf("Runtime error: remove() expects a list\n");
            exit(1);
        }

        double value = eval_expression(value_node);
        int found = 0;

        for (int i = 0; i < list_node->list.count; i++)
        {
            double element = eval_expression(list_node->list.elements[i]);
            if (element == value)
            {
                found = 1;
                // Shift elements to fill the gap
                for (int j = i; j < list_node->list.count - 1; j++)
                {
                    list_node->list.elements[j] = list_node->list.elements[j + 1];
                }
                list_node->list.count--;
                break;
            }
        }

        if (!found)
        {
            printf("Runtime error: Value not found in list\n");
            exit(1);
        }
        return 0; // Return success
    }

    case NODE_AND:
    {
        bool left = eval_expression(node->binop.left) != 0;
        bool right = eval_expression(node->binop.right) != 0;
        return left && right;
    }

    case NODE_OR:
    {
        bool left = eval_expression(node->binop.left) != 0;
        bool right = eval_expression(node->binop.right) != 0;
        return left || right;
    }

    case NODE_NOT:
    {
        bool operand = eval_expression(node->unop.operand) != 0;
        return !operand;
    }

    case NODE_BITWISE_AND:
    {
        int lhs = (int)eval_expression(node->binop.left);
        int rhs = (int)eval_expression(node->binop.right);
        return (double)(lhs & rhs);
    }

    case NODE_BITWISE_OR:
    {
        int lhs = (int)eval_expression(node->binop.left);
        int rhs = (int)eval_expression(node->binop.right);
        return (double)(lhs | rhs);
    }

    case NODE_BITWISE_XOR:
    {
        int lhs = (int)eval_expression(node->binop.left);
        int rhs = (int)eval_expression(node->binop.right);
        return (double)(lhs ^ rhs);
    }

    case NODE_PRINT:
    {
        double value = eval_expression(node->binop.left);
        if (value == 0 || value == 1)
        {
            printf("%s\n", bool_to_str((bool)value));
        }
        else
        {
            printf("%g\n", value);
        }
        return value;
    }

    case NODE_PATTERN_MATCH:
    {
        // Get the pattern and noise strings
        const char *pattern_str = NULL;
        const char *noise_str = NULL;

        if (node->pattern_match.pattern->type == NODE_STRING)
        {
            pattern_str = node->pattern_match.pattern->string;
        }
        else if (node->pattern_match.pattern->type == NODE_VAR)
        {
            pattern_str = get_variable(node->pattern_match.pattern->varname);
        }

        if (node->pattern_match.noise->type == NODE_STRING)
        {
            noise_str = node->pattern_match.noise->string;
        }
        else if (node->pattern_match.noise->type == NODE_VAR)
        {
            noise_str = get_variable(node->pattern_match.noise->varname);
        }

        if (!pattern_str || !noise_str)
        {
            printf("Runtime error: pattern_match expects string arguments\n");
            exit(1);
        }

        int pattern_len = strlen(pattern_str);
        int noise_len = strlen(noise_str);

        if (pattern_len == 0 || noise_len == 0)
        {
            return 0; // No matches if either string is empty
        }

        // Create a list to store the match positions
        ASTNode *result_list = ast_new_list();

        // Naive pattern matching algorithm
        for (int i = 0; i <= noise_len - pattern_len; i++)
        {
            int match = 1;
            for (int j = 0; j < pattern_len; j++)
            {
                if (noise_str[i + j] != pattern_str[j])
                {
                    match = 0;
                    break;
                }
            }
            if (match)
            {
                // Add the position to the result list
                ast_list_add_element(result_list, ast_new_number(i));
            }
        }

        // Store the result in a special variable or print it directly
        char *list_str = list_to_string(result_list);
        printf("%s\n", list_str); // Print the result
        free(list_str);
        ast_free(result_list);
        return 0;
    }

    default:
        fprintf(stderr, "Runtime error: Unsupported AST node type %d\n", node->type);
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

    char *result = strdup("[");
    for (int i = 0; i < list->list.count; i++)
    {
        char buffer[256];
        ASTNode *element = list->list.elements[i];

        if (element->type == NODE_NUMBER)
        {
            snprintf(buffer, sizeof(buffer), "%g", element->number);
        }
        else if (element->type == NODE_STRING)
        {
            snprintf(buffer, sizeof(buffer), "%s", element->string);
        }
        else
        {
            snprintf(buffer, sizeof(buffer), "Unknown");
        }

        result = realloc(result, strlen(result) + strlen(buffer) + 3);
        strcat(result, buffer);

        if (i < list->list.count - 1)
        {
            strcat(result, ", ");
        }
    }

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
                printf("Runtime Error: Undefined variable '%s'\n", node->varname);
            }
        }
        break;
    }
    default:
        printf("", eval_expression(node));
        break;
    }
}

// Helper function to convert boolean to string
const char *bool_to_str(bool value)
{
    return value ? "true" : "false";
}

// Modify your print function (if you have one)
void interpret_print(ASTNode *node)
{
    double value = eval_expression(node->binop.left);
    if (value == 0 || value == 1)
    {
        printf("%s\n", bool_to_str((bool)value));
    }
    else
    {
        printf("%g\n", value);
    }
}