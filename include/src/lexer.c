#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

static const char* input;
static int pos;

void lexer_init(const char *source) {
    input = source;
    pos = 0;
}

static void skip_whitespace() {
    while (isspace(input[pos])) pos++;
}

static int starts_with(const char* str) {
    return strncmp(&input[pos], str, strlen(str)) == 0;
}

static int safe_peek(int offset) {
    if (input[pos + offset] == '\0') return 0;
    return input[pos + offset];
}

Token lexer_next_token() {
    Token token;
    token.type = TOK_UNKNOWN;
    token.text[0] = '\0';

    skip_whitespace();

    if (input[pos] == '\0') {
        token.type = TOK_EOF;
        return token;
    }

    // Keywords and multi-char tokens (order matters)
    if (starts_with("let$")) {
        token.type = TOK_LET;
        strcpy(token.text, "let$");
        pos += 4;
        return token;
    }
    if (starts_with("::print")) {
        token.type = TOK_PRINT;
        strcpy(token.text, "::print");
        pos += 7;
        return token;
    }
    if (starts_with("if$")) {
        token.type = TOK_IF;
        strcpy(token.text, "if$");
        pos += 3;
        return token;
    }
    if (starts_with("elseif$")) {
        token.type = TOK_ELSEIF;
        strcpy(token.text, "elseif$");
        pos += 7;
        return token;
    }
    if (starts_with("else")) {
        token.type = TOK_ELSE;
        strcpy(token.text, "else");
        pos += 4;
        return token;
    }
    if (starts_with("loop$")) {
        token.type = TOK_LOOP;
        strcpy(token.text, "loop$");
        pos += 5;
        return token;
    }
    if (starts_with("import$")) {
        token.type = TOK_IMPORT;
        strcpy(token.text, "import$");
        pos += 7;
        return token;
    }
    if (starts_with("func$")) {
        token.type = TOK_FUNC;
        strcpy(token.text, "func$");
        pos += 5;
        return token;
    }

    // Assignment operator :=
    if (input[pos] == ':' && safe_peek(1) == '=') {
        token.type = TOK_ASSIGN;
        strcpy(token.text, ":=");
        pos += 2;
        return token;
    }

    // Arrow operator =>
    if (input[pos] == '=' && safe_peek(1) == '>') {
        token.type = TOK_ARROW;
        strcpy(token.text, "=>");
        pos += 2;
        return token;
    }

    // Arrow operator ⟶ (UTF-8: E2 87 92)
    if ((unsigned char)input[pos] == 0xE2 && (unsigned char)safe_peek(1) == 0x87 && (unsigned char)safe_peek(2) == 0x92) {
        token.type = TOK_ARROW;
        strcpy(token.text, "⟶");
        pos += 3;
        return token;
    }

    // Single char tokens and operators
    switch (input[pos]) {
        case '+': token.type = TOK_PLUS; token.text[0] = '+'; token.text[1] = '\0'; pos++; return token;
        case '-': token.type = TOK_MINUS; token.text[0] = '-'; token.text[1] = '\0'; pos++; return token;
        case '*': token.type = TOK_MUL; token.text[0] = '*'; token.text[1] = '\0'; pos++; return token;
        case '/': token.type = TOK_DIV; token.text[0] = '/'; token.text[1] = '\0'; pos++; return token;
        case '%': token.type = TOK_MOD; token.text[0] = '%'; token.text[1] = '\0'; pos++; return token;
        case '(': token.type = TOK_LPAREN; token.text[0] = '('; token.text[1] = '\0'; pos++; return token;
        case ')': token.type = TOK_RPAREN; token.text[0] = ')'; token.text[1] = '\0'; pos++; return token;
        case ';': token.type = TOK_SEMICOLON; token.text[0] = ';'; token.text[1] = '\0'; pos++; return token;
        case '>':
            if (safe_peek(1) == '=') {
                token.type = TOK_GTE;
                strcpy(token.text, ">=");
                pos += 2;
            } else {
                token.type = TOK_GT;
                token.text[0] = '>';
                token.text[1] = '\0';
                pos++;
            }
            return token;
        case '<':
            if (safe_peek(1) == '=') {
                token.type = TOK_LTE;
                strcpy(token.text, "<=");
                pos += 2;
            } else {
                token.type = TOK_LT;
                token.text[0] = '<';
                token.text[1] = '\0';
                pos++;
            }
            return token;
        case '=':
            if (safe_peek(1) == '=') {
                token.type = TOK_EQ;
                strcpy(token.text, "==");
                pos += 2;
                return token;
            }
            break;
        case '!':
            if (safe_peek(1) == '=') {
                token.type = TOK_NEQ;
                strcpy(token.text, "!=");
                pos += 2;
                return token;
            }
            break;
        case ',':
            token.type = TOK_COMMA;
            token.text[0] = ',';
            token.text[1] = '\0';
            pos++;
            return token;
        case '{':
            token.type = TOK_LBRACE;
            token.text[0] = '{';
            token.text[1] = '\0';
            pos++;
            return token;

        case '}':
            token.type = TOK_RBRACE;
            token.text[0] = '}';
            token.text[1] = '\0';
            pos++;
            return token;
    }

    // String literal
    if (input[pos] == '"') {
        pos++;
        int start = pos;
        while (input[pos] != '"' && input[pos] != '\0') pos++;
        int len = pos - start;
        if (input[pos] == '"') pos++;
        if (len >= sizeof(token.text)) len = sizeof(token.text) - 1;
        strncpy(token.text, &input[start], len);
        token.text[len] = '\0';
        token.type = TOK_STRING;
        return token;
    }

    // Number literal (integer or float)
    if (isdigit(input[pos])) {
        int start = pos;
        while (isdigit(input[pos]) || input[pos] == '.') pos++;
        int len = pos - start;
        if (len >= sizeof(token.text)) len = sizeof(token.text) - 1;
        strncpy(token.text, &input[start], len);
        token.text[len] = '\0';
        token.type = TOK_NUMBER;
        return token;
    }

    // Identifier
    if (isalpha(input[pos]) || input[pos] == '@' || input[pos] == '$' || input[pos] == '_') {
        int start = pos;
        while (isalnum(input[pos]) || input[pos] == '@' || input[pos] == '$' || input[pos] == '_') pos++;
        int len = pos - start;
        if (len >= sizeof(token.text)) len = sizeof(token.text) - 1;
        strncpy(token.text, &input[start], len);
        token.text[len] = '\0';
        token.type = TOK_ID;
        return token;
    }

    token.type = TOK_UNKNOWN;
    token.text[0] = input[pos];
    token.text[1] = '\0';
    pos++;
    return token;
}
