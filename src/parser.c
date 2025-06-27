#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "lexer.h"
#include "ast.h"

static Token current_token;

static void next_token()
{
    current_token = lexer_next_token();
}

static void expect(TokenType type)
{
    if (current_token.type != type)
    {
        printf("Parse error: Expected token %d but got %d ('%s')\n", type, current_token.type, current_token.text);
        exit(1);
    }
    next_token();
}

// Forward declarations
static ASTNode *parse_statement();
static ASTNode *parse_expression();
static ASTNode *parse_primary();
static ASTNode *parse_binop_rhs(int expr_prec, ASTNode *lhs);
static int get_token_precedence(TokenType tok);
static ASTNode *parse_block();

void parser_init(const char *source)
{
    lexer_init(source);
    next_token();
}

ASTNode *parse_program()
{
    ASTNode *block = ast_new_block();
    while (current_token.type != TOK_EOF)
    {
        ASTNode *stmt = parse_statement();
        ast_block_add_statement(block, stmt);
    }
    return block;
}

static int get_token_precedence(TokenType tok)
{
    switch (tok)
    {
    case TOK_EQ:
    case TOK_NEQ:
        return 10;
    case TOK_LT:
    case TOK_LTE:
    case TOK_GT:
    case TOK_GTE:
        return 20;
    case TOK_PLUS:
    case TOK_MINUS:
        return 30;
    case TOK_MUL:
    case TOK_DIV:
    case TOK_MOD:
        return 40;
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
        ASTNode *node = ast_new_string(current_token.text);
        next_token();
        return node;
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
                        printf("Parse error: Too many function call arguments (max 4)\n");
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

            return ast_new_func_call(varname, args, arg_count);
        }

        return ast_new_var(varname);
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

        TokenType bin_op = current_token.type;
        next_token();

        ASTNode *rhs = parse_expression();

        int next_prec = get_token_precedence(current_token.type);
        if (tok_prec < next_prec)
        {
            rhs = parse_binop_rhs(tok_prec + 1, rhs);
        }

        lhs = ast_new_binop(lhs, rhs, bin_op);
    }

    return lhs; // defensive
}

static ASTNode *parse_expression()
{
    ASTNode *lhs = parse_primary();
    return parse_binop_rhs(0, lhs);
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
    }

    if (current_token.type == TOK_LET)
    {
        next_token();
        if (current_token.type != TOK_ID)
        {
            printf("Parse error: Expected variable name after let$\n");
            exit(1);
        }
        char varname[64];
        strcpy(varname, current_token.text);
        next_token();
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

        ASTNode *elseif_branch = NULL;
        if (current_token.type == TOK_ELSEIF)
        {
            next_token();
            elseif_branch = parse_statement();
        }

        ASTNode *else_branch = NULL;
        if (current_token.type == TOK_ELSE)
        {
            next_token();
            else_branch = parse_statement();
        }

        return ast_new_if(cond, then_branch, elseif_branch, else_branch);
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
                if (current_token.type != TOK_ID)
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
