#ifndef LEXER_H
#define LEXER_H

typedef enum
{
    TOK_FUNC,
    TOK_LET,
    TOK_ID,
    TOK_ASSIGN,
    TOK_STRING,
    TOK_COMMA,
    TOK_NUMBER,
    TOK_PRINT,
    TOK_IF,
    TOK_ELSE,
    TOK_ELSEIF,
    TOK_LOOP,
    TOK_IMPORT,
    TOK_PLUS,
    TOK_MINUS,
    TOK_MUL,
    TOK_DIV,
    TOK_MOD,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_GT,
    TOK_LT,
    TOK_GTE,
    TOK_LTE,
    TOK_EQ,
    TOK_NEQ,
    TOK_SEMICOLON,
    TOK_ARROW,
    TOK_UNKNOWN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_LBRACKET,
    TOK_RBRACKET,
    TOK_LIST_LEN,
    TOK_LIST_APPEND,
    TOK_LIST_PREPEND,
    TOK_LIST_POP,
    TOK_LIST_INSERT,
    TOK_LIST_REMOVE,
    TOK_AND,
    TOK_OR,
    TOK_NOT,
    TOK_BITWISE_AND,
    TOK_BITWISE_OR,
    TOK_BITWISE_XOR,
    TOK_BITWISE_NOT,
    TOK_PATTERN_MATCH,
    TOK_FORMAT_SPECIFIER,
    TOK_EOF,
} TokenType;

typedef struct
{
    TokenType type;
    char string_value[256];
    char text[64];
    double number_value;
} Token;

void lexer_init(const char *source);
Token lexer_next_token();

#endif
