#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "lexer.h"
#include "ast.h"

static Token current_token;

// Helper function to get the next token
static void next_token()
{
    current_token = lexer_next_token();
}

// Helper function to expect a specific token type
static void expect(TokenType type)
{
    if (current_token.type != type)
    {
        fprintf(stderr, "Expected token type %d, but got %d\n", type, current_token.type);
        exit(1);
    }
    next_token();
}

// Forward declarations
static ASTNode *parse_statement();
static ASTNode *parse_expression();
static ASTNode *parse_logical_expression();
static ASTNode *parse_bitwise_expression();
static ASTNode *parse_comparison();
static ASTNode *parse_unary_expression();
static ASTNode *parse_primary();
static ASTNode *parse_binop_rhs(int expr_prec, ASTNode *lhs);
static int get_token_precedence(TokenType tok);
static ASTNode *parse_block();
static ASTNode *parse_class_def();
static ASTNode *parse_member_access(ASTNode *object);
static ASTNode *parse_method_def();
static ASTNode *parse_method_call(ASTNode *object);

// --- Class name tracking for parser ---
#define MAX_CLASS_NAMES 128
static char class_names[MAX_CLASS_NAMES][64];
static int class_name_count = 0;

static void parser_register_class_name(const char *name)
{
    if (class_name_count < MAX_CLASS_NAMES)
    {
        strncpy(class_names[class_name_count++], name, 63);
        class_names[class_name_count - 1][63] = '\0';
    }
}

static int parser_is_class_name(const char *name)
{
    for (int i = 0; i < class_name_count; i++)
    {
        if (strcmp(class_names[i], name) == 0)
            return 1;
    }
    return 0;
}

void parser_init(const char *source)
{
    lexer_init(source);
    next_token(); // Prime the first token
}

ASTNode *parse_program()
{
    ASTNode *block = ast_new_block();
    while (current_token.type != TOK_EOF)
    {
        if (current_token.type == TOK_CLASS)
        {
            ASTNode *class_node = parse_class_def();
            ast_block_add_statement(block, class_node);
        }
        else
        {
            ast_block_add_statement(block, parse_statement());
        }
    }
    return block;
}

static int get_token_precedence(TokenType tok)
{
    switch (tok)
    {
    case TOK_EQ:
    case TOK_NEQ:
        return 20;
    case TOK_LT:
    case TOK_LTE:
    case TOK_GT:
    case TOK_GTE:
        return 30;
    case TOK_PLUS:
    case TOK_MINUS:
        return 40;
    case TOK_MUL:
    case TOK_DIV:
    case TOK_MOD:
        return 50;
    default:
        return -1;
    }
}

static ASTNode *parse_block()
{
    expect(TOK_LBRACE); // consume '{'

    ASTNode *block = ast_new_block();

    while (current_token.type != TOK_RBRACE && current_token.type != TOK_EOF)
    {
        ASTNode *stmt = parse_statement();
        if (stmt != NULL)
        {
            ast_block_add_statement(block, stmt);
        }
    }

    expect(TOK_RBRACE); // consume '}'

    return block;
}

static ASTNode *parse_primary()
{
    if (current_token.type == TOK_RBRACE)
    {
        printf("Parse error: Unexpected closing '}' found while parsing an expression\n");
        exit(1);
    }
    if (current_token.type == TOK_NUMBER)
    {
        double val = strtod(current_token.text, NULL);
        ASTNode *node = ast_new_number(val);
        next_token();
        return node;
    }

    if (current_token.type == TOK_STRING)
    {
        // Check if this is a format string (contains @)
        if (strchr(current_token.text, '@') != NULL)
        {
            char format[256];
            strncpy(format, current_token.text, sizeof(format));
            format[sizeof(format) - 1] = '\0';

            next_token();

            // Parse format arguments if present
            ASTNode *args[4] = {NULL};
            int arg_count = 0;

            if (current_token.type == TOK_LPAREN)
            {
                next_token();
                while (current_token.type != TOK_RPAREN && current_token.type != TOK_EOF)
                {
                    if (arg_count >= 4)
                    {
                        printf("Parse error: Too many format arguments (max 4)\n");
                        exit(1);
                    }
                    args[arg_count++] = parse_expression();
                    if (current_token.type == TOK_COMMA)
                    {
                        next_token();
                    }
                }
                expect(TOK_RPAREN);
            }

            return ast_new_format_string(format, args, arg_count);
        }
        else
        {
            // Regular string without formatting
            ASTNode *node = ast_new_string(current_token.text);
            next_token();
            return node;
        }
    }
    if (current_token.type == TOK_LBRACKET)
    {
        next_token();
        ASTNode *list = ast_new_list();
        while (current_token.type != TOK_RBRACKET)
        {
            ASTNode *element = parse_expression();
            ast_list_add_element(list, element);
            if (current_token.type == TOK_COMMA)
            {
                next_token();
            }
            else
            {
                break;
            }
        }
        expect(TOK_RBRACKET);

        // Check if this is a list access (like [1,2,3][0])
        if (current_token.type == TOK_LBRACKET)
        {
            next_token();
            ASTNode *index = parse_expression();
            expect(TOK_RBRACKET);
            return ast_new_list_access(list, index);
        }

        return list;
    }

    if (current_token.type == TOK_ID)
    {
        char varname[64];
        strcpy(varname, current_token.text);
        next_token();

        if (current_token.type == TOK_LBRACKET)
        {
            next_token();
            ASTNode *index = parse_expression();
            expect(TOK_RBRACKET);
            return ast_new_list_access(ast_new_var(varname), index);
        }

        if (current_token.type == TOK_LPAREN)
        {
            next_token();
            ASTNode *args[4];
            int arg_count = 0;
            if (current_token.type != TOK_RPAREN)
            {
                while (1)
                {
                    if (arg_count >= 4)
                    {
                        printf("Parse error: Too many function/class call arguments (max 4)\n");
                        exit(1);
                    }
                    args[arg_count++] = parse_expression();
                    if (current_token.type == TOK_COMMA)
                        next_token();
                    else
                        break;
                }
            }
            expect(TOK_RPAREN);

            // Check if this is a class instantiation
            if (parser_is_class_name(varname))
            {
                return ast_new_class_instance(varname, args, arg_count);
            }
            else
            {
                return ast_new_func_call(varname, args, arg_count);
            }
        }

        ASTNode *node = ast_new_var(varname);
        // Check for member access
        if (current_token.type == TOK_DOT)
        {
            node = parse_member_access(node);
        }
        return node;
    }

    // Handle 'self' as a variable (like TOK_ID)
    if (current_token.type == TOK_SELF)
    {
        char varname[64];
        strcpy(varname, current_token.text);
        next_token();
        ASTNode *node = ast_new_var(varname);
        // Check for member access
        if (current_token.type == TOK_DOT)
        {
            node = parse_member_access(node);
        }
        return node;
    }

    if (current_token.type == TOK_LPAREN)
    {
        next_token();
        ASTNode *expr = parse_expression();
        expect(TOK_RPAREN);
        return expr;
    }

    if (current_token.type == TOK_LBRACE)
    {
        return parse_block();
    }
    if (current_token.type == TOK_PATTERN_MATCH)
    {
        next_token();
        expect(TOK_LPAREN);
        ASTNode *pattern = parse_expression();
        expect(TOK_COMMA);
        ASTNode *noise = parse_expression();
        expect(TOK_RPAREN);
        return ast_new_pattern_match(pattern, noise);
    }

    if (current_token.type == TOK_DICT_NEW)
    {
        next_token();
        expect(TOK_LBRACE);
        ASTNode *dict = ast_new_dict();
        while (current_token.type != TOK_RBRACE)
        {
            ASTNode *key = parse_expression();
            expect(TOK_ASSIGN);
            ASTNode *value = parse_expression();
            ast_dict_add_pair(dict, key, value);
            if (current_token.type == TOK_COMMA)
            {
                next_token();
            }
            else
            {
                break;
            }
        }
        expect(TOK_RBRACE);
        return dict;
    }

    if (current_token.type == TOK_STACK_NEW)
    {
        next_token();
        return ast_new_stack();
    }

    if (current_token.type == TOK_DICT_GET ||
        current_token.type == TOK_DICT_SET ||
        current_token.type == TOK_DICT_KEYS ||
        current_token.type == TOK_DICT_VALUES)
    {
        TokenType func_type = current_token.type;
        next_token();
        expect(TOK_LPAREN);
        ASTNode *dict = parse_expression();
        
        if (func_type == TOK_DICT_KEYS || func_type == TOK_DICT_VALUES)
        {
            expect(TOK_RPAREN);
            if (func_type == TOK_DICT_KEYS)
                return ast_new_dict_keys(dict);
            else
                return ast_new_dict_values(dict);
        }
        else if (func_type == TOK_DICT_GET)
        {
            expect(TOK_COMMA);
            ASTNode *key = parse_expression();
            expect(TOK_RPAREN);
            return ast_new_dict_get(dict, key);
        }
        else // TOK_DICT_SET
        {
            expect(TOK_COMMA);
            ASTNode *key = parse_expression();
            expect(TOK_COMMA);
            ASTNode *value = parse_expression();
            expect(TOK_RPAREN);
            return ast_new_dict_set(dict, key, value);
        }
    }

    if (current_token.type == TOK_STACK_PUSH ||
        current_token.type == TOK_STACK_POP ||
        current_token.type == TOK_STACK_PEEK ||
        current_token.type == TOK_STACK_SIZE ||
        current_token.type == TOK_STACK_EMPTY)
    {
        TokenType func_type = current_token.type;
        next_token();
        expect(TOK_LPAREN);
        ASTNode *stack = parse_expression();
        
        if (func_type == TOK_STACK_PUSH)
        {
            expect(TOK_COMMA);
            ASTNode *value = parse_expression();
            expect(TOK_RPAREN);
            return ast_new_stack_push(stack, value);
        }
        else
        {
            expect(TOK_RPAREN);
            switch (func_type)
            {
            case TOK_STACK_POP:
                return ast_new_stack_pop(stack);
            case TOK_STACK_PEEK:
                return ast_new_stack_peek(stack);
            case TOK_STACK_SIZE:
                return ast_new_stack_size(stack);
            case TOK_STACK_EMPTY:
                return ast_new_stack_empty(stack);
            default:
                printf("Parse error: Unknown stack function\n");
                exit(1);
            }
        }
    }

    if (current_token.type == TOK_LIST_LEN ||
        current_token.type == TOK_LIST_APPEND ||
        current_token.type == TOK_LIST_PREPEND ||
        current_token.type == TOK_LIST_POP ||
        current_token.type == TOK_LIST_INSERT ||
        current_token.type == TOK_LIST_REMOVE)
    {
        TokenType func_type = current_token.type;
        next_token();

        expect(TOK_LPAREN);
        ASTNode *list = parse_expression();

        if (func_type == TOK_LIST_LEN || func_type == TOK_LIST_POP)
        {
            expect(TOK_RPAREN);
            if (func_type == TOK_LIST_LEN)
                return ast_new_list_len(list);
            else
                return ast_new_list_pop(list);
        }
        else if (func_type == TOK_LIST_INSERT)
        {
            expect(TOK_COMMA);
            ASTNode *index = parse_expression();
            expect(TOK_COMMA);
            ASTNode *value = parse_expression();
            expect(TOK_RPAREN);
            return ast_new_list_insert(list, index, value);
        }
        else
        {
            expect(TOK_COMMA);
            ASTNode *arg = parse_expression();
            expect(TOK_RPAREN);

            switch (func_type)
            {
            case TOK_LIST_APPEND:
                return ast_new_list_append(list, arg);
            case TOK_LIST_PREPEND:
                return ast_new_list_prepend(list, arg);
            case TOK_LIST_REMOVE:
                return ast_new_list_remove(list, arg);
            default:
                printf("Parse error: Unknown list function\n");
                exit(1);
            }
        }
    }

    printf("Parse error: Unexpected token '%s' (type %d) in primary expression\n", current_token.text, current_token.type);
    exit(1);

    exit(1);
}

static ASTNode *parse_binop_rhs(int expr_prec, ASTNode *lhs)
{
    while (1)
    {
        int tok_prec = get_token_precedence(current_token.type);

        if (tok_prec < expr_prec)
            return lhs;

        TokenType binop = current_token.type;
        next_token();

        ASTNode *rhs = parse_logical_expression();

        int next_prec = get_token_precedence(current_token.type);
        if (tok_prec < next_prec)
        {
            rhs = parse_binop_rhs(tok_prec + 1, rhs);
        }

        lhs = ast_new_binop(lhs, rhs, binop);
    }

    return lhs; // defensive
}

static ASTNode *parse_expression()
{
    ASTNode *lhs = parse_logical_expression();
    return parse_binop_rhs(0, lhs);
}

static ASTNode *parse_logical_expression()
{
    ASTNode *left = parse_bitwise_expression();

    while (current_token.type == TOK_AND || current_token.type == TOK_OR)
    {
        TokenType op = current_token.type;
        next_token(); // Move past the operator
        ASTNode *right = parse_bitwise_expression();

        if (op == TOK_AND)
            left = ast_new_and(left, right);
        else // TOK_OR
            left = ast_new_or(left, right);
    }

    return left;
}

static ASTNode *parse_bitwise_expression()
{
    ASTNode *left = parse_comparison();

    while (current_token.type == TOK_BITWISE_AND ||
           current_token.type == TOK_BITWISE_OR ||
           current_token.type == TOK_BITWISE_XOR)
    {
        TokenType op = current_token.type;
        next_token(); // Move past the operator
        ASTNode *right = parse_comparison();

        switch (op)
        {
        case TOK_BITWISE_AND:
            left = ast_new_bitwise_and(left, right);
            break;
        case TOK_BITWISE_OR:
            left = ast_new_bitwise_or(left, right);
            break;
        case TOK_BITWISE_XOR:
            left = ast_new_bitwise_xor(left, right);
            break;
        }
    }

    return left;
}

static ASTNode *parse_comparison()
{
    ASTNode *left = parse_unary_expression();

    while (current_token.type == TOK_EQ || current_token.type == TOK_NEQ ||
           current_token.type == TOK_LT || current_token.type == TOK_GT ||
           current_token.type == TOK_LTE || current_token.type == TOK_GTE)
    {
        TokenType op = current_token.type;
        next_token(); // Move past the operator
        ASTNode *right = parse_unary_expression();
        left = ast_new_binop(left, right, op);
    }

    return left;
}

static ASTNode *parse_unary_expression()
{
    if (current_token.type == TOK_NOT || current_token.type == TOK_BITWISE_NOT)
    {
        TokenType op = current_token.type;
        next_token();
        ASTNode *operand = parse_unary_expression();
        if (op == TOK_NOT)
        {
            return ast_new_not(operand);
        }
        else if (op == TOK_BITWISE_NOT)
        {
            return ast_new_bitwise_not(operand);
        }
    }

    return parse_primary();
}

static ASTNode *parse_statement()
{
    // Handle block if statement starts with '{'
    if (current_token.type == TOK_LBRACE)
    {
        return parse_block();
    }

    // Skip standalone semicolons as empty statements
    while (current_token.type == TOK_SEMICOLON)
    {
        next_token();
        // Return NULL for empty statement so it is not added to block
        return NULL;
    }

    if (current_token.type == TOK_LET)
    {
        next_token();
        if (current_token.type != TOK_ID && current_token.type != TOK_SELF)
        {
            printf("Parse error: Expected variable name after let$\n");
            exit(1);
        }

        // Handle both regular variables and self.member
        char varname[64];
        if (current_token.type == TOK_SELF)
        {
            // Handle self.member case
            strcpy(varname, current_token.text);
            next_token();
            if (current_token.type == TOK_DOT)
            {
                // This is a member access (self.member)
                ASTNode *self_node = ast_new_var(varname);
                ASTNode *member_access = parse_member_access(self_node);

                expect(TOK_ASSIGN);
                ASTNode *val = parse_expression();

                // Create a special assignment node for member access
                ASTNode *assign = malloc(sizeof(ASTNode));
                assign->type = NODE_MEMBER_ASSIGN;
                assign->member_assign.object = member_access->member_access.object;
                strncpy(assign->member_assign.member_name,
                        member_access->member_access.member_name,
                        sizeof(assign->member_assign.member_name));
                assign->member_assign.value = val;

                ast_free(member_access);
                return assign;
            }
            else
            {
                // Just regular self variable
                strcpy(varname, current_token.text);
                next_token();
            }
        }
        else
        {
            // Regular variable
            strcpy(varname, current_token.text);
            next_token();
        }

        expect(TOK_ASSIGN);
        ASTNode *val = parse_expression();
        return ast_new_assign(varname, val);
    }

    if (current_token.type == TOK_PRINT)
    {
        next_token();
        ASTNode *expr = parse_expression();
        return ast_new_print(expr);
    }

    if (current_token.type == TOK_IF)
    {
        next_token();
        ASTNode *cond = parse_expression();
        ASTNode *then_branch = parse_statement();

        ASTNode *elseif_chain = NULL;
        ASTNode **elseif_chain_ptr = &elseif_chain;
        while (current_token.type == TOK_ELSEIF)
        {
            next_token();
            ASTNode *elseif_cond = parse_expression();
            ASTNode *elseif_then = parse_statement();
            ASTNode *new_elseif = ast_new_if(elseif_cond, elseif_then, NULL, NULL);
            *elseif_chain_ptr = new_elseif;
            elseif_chain_ptr = &new_elseif->if_stmt.elseif_branch;
        }

        ASTNode *else_branch = NULL;
        if (current_token.type == TOK_ELSE)
        {
            next_token();
            else_branch = parse_statement();
        }

        // Attach the elseif_chain to the main if node
        return ast_new_if(cond, then_branch, elseif_chain, else_branch);
    }

    if (current_token.type == TOK_LOOP)
    {
        next_token();
        if (current_token.type != TOK_ID)
        {
            printf("Parse error: Expected loop variable name\n");
            exit(1);
        }
        char loop_var[64];
        strcpy(loop_var, current_token.text);
        next_token();
        expect(TOK_ASSIGN);
        ASTNode *start = parse_expression();
        expect(TOK_ARROW);
        ASTNode *end = parse_expression();
        ASTNode *body = parse_statement();

        return ast_new_loop(loop_var, start, end, body);
    }

    if (current_token.type == TOK_IMPORT)
    {
        next_token();
        if (current_token.type != TOK_STRING)
        {
            printf("Parse error: Expected string literal after import$\n");
            exit(1);
        }
        ASTNode *node = ast_new_import(current_token.text);
        next_token();
        return node;
    }

    if (current_token.type == TOK_FUNC)
    {
        next_token();
        if (current_token.type != TOK_ID)
        {
            printf("Parse error: Expected function name after func$\n");
            exit(1);
        }

        char fname[64];
        strcpy(fname, current_token.text);
        next_token();

        expect(TOK_LPAREN);

        char params[4][64];
        int param_count = 0;

        if (current_token.type != TOK_RPAREN)
        {
            while (1)
            {
                if (param_count >= 4)
                {
                    printf("Parse error: Too many function parameters (max 4)\n");
                    exit(1);
                }
                if (current_token.type != TOK_ID && current_token.type != TOK_SELF)
                {
                    printf("Parse error: Expected parameter name\n");
                    exit(1);
                }
                strcpy(params[param_count++], current_token.text);
                next_token();
                if (current_token.type == TOK_COMMA)
                    next_token();
                else
                    break;
            }
        }

        expect(TOK_RPAREN);
        expect(TOK_ARROW);
        ASTNode *body = parse_statement();

        return ast_new_func_def(fname, params, param_count, body);
    }

    if (current_token.type == TOK_RBRACE || current_token.type == TOK_EOF)
    {
        return NULL; // Signal "no more statement" to caller
    }
    ASTNode *expr = parse_expression();
    return expr;
}

ASTNode *parse_class_def()
{
    expect(TOK_CLASS);
    if (current_token.type != TOK_ID)
    {
        printf("Parse error: Expected class name after class$\n");
        exit(1);
    }
    char class_name[64];
    strncpy(class_name, current_token.text, 64);
    class_name[63] = '\0';
    next_token();
    expect(TOK_LBRACE); // Start of class body

    ASTNode *block = ast_new_block();
    while (current_token.type != TOK_RBRACE && current_token.type != TOK_EOF)
    {
        if (current_token.type == TOK_FUNC)
        {
            ASTNode *method = parse_method_def();
            ast_block_add_statement(block, method);
        }
        else
        {
            ASTNode *stmt = parse_statement();
            if (stmt != NULL)
            {
                ast_block_add_statement(block, stmt);
            }
        }
    }
    expect(TOK_RBRACE); // End of class body

    parser_register_class_name(class_name); // Register the class name
    return ast_new_class_def(class_name, block);
}

ASTNode *parse_member_access(ASTNode *object)
{
    while (current_token.type == TOK_DOT)
    {
        next_token();
        if (current_token.type != TOK_ID)
        {
            printf("Parse error: Expected member name after '.'\n");
            exit(1);
        }
        char member_name[64];
        strncpy(member_name, current_token.text, 64);
        member_name[63] = '\0';
        next_token();
        if (current_token.type == TOK_LPAREN)
        {
            // Method call
            ASTNode **args = malloc(sizeof(ASTNode *) * 8);
            int arg_count = 0;
            next_token(); // consume '('
            if (current_token.type != TOK_RPAREN)
            {
                do
                {
                    args[arg_count++] = parse_expression();
                    if (current_token.type == TOK_COMMA)
                        next_token();
                } while (current_token.type != TOK_RPAREN);
            }
            expect(TOK_RPAREN);
            object = ast_new_method_call(object, member_name, args, arg_count);
        }
        else
        {
            // Field access
            object = ast_new_member_access(object, member_name);
        }
    }
    return object;
}

ASTNode *parse_method_def()
{
    // Assumes current_token is TOK_FUNC
    expect(TOK_FUNC);
    if (current_token.type != TOK_ID)
    {
        printf("Parse error: Expected method name after func$\n");
        exit(1);
    }
    char method_name[64];
    strncpy(method_name, current_token.text, 64);
    method_name[63] = '\0';
    next_token();

    expect(TOK_LPAREN);
    char params[4][64];
    int param_count = 0;
    if (current_token.type != TOK_RPAREN)
    {
        while (1)
        {
            if (param_count >= 4)
            {
                printf("Parse error: Too many method parameters (max 4)\n");
                exit(1);
            }
            if (current_token.type != TOK_ID && current_token.type != TOK_SELF)
            {
                printf("Parse error: Expected parameter name in method\n");
                exit(1);
            }
            strncpy(params[param_count++], current_token.text, 64);
            params[param_count - 1][63] = '\0';
            next_token();
            if (current_token.type == TOK_COMMA)
                next_token();
            else
                break;
        }
    }
    expect(TOK_RPAREN);
    expect(TOK_ARROW);
    ASTNode *body = parse_statement();
    return ast_new_method_def(method_name, params, param_count, body);
}