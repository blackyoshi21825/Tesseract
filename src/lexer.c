#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "lexer.h"

static const char *input;
static int pos;
static int current_line;
static int current_column;

void lexer_init(const char *source)
{
    input = source;
    pos = 0;
    current_line = 1;
    current_column = 1;
}

static void advance_pos(int count)
{
    for (int i = 0; i < count; i++)
    {
        if (input[pos] == '\n')
        {
            current_line++;
            current_column = 1;
        }
        else
        {
            current_column++;
        }
        pos++;
    }
}

static void skip_whitespace()
{
    while (isspace(input[pos]))
    {
        advance_pos(1);
    }
}

static int starts_with(const char *str)
{
    return strncmp(&input[pos], str, strlen(str)) == 0;
}

static int safe_peek(int offset)
{
    if (input[pos + offset] == '\0')
        return 0;
    return input[pos + offset];
}

Token lexer_next_token()
{
    Token token;
    token.type = TOK_UNKNOWN;
    token.text[0] = '\0';
    token.line = current_line;
    token.column = current_column;

    skip_whitespace();
    
    // Update token position after skipping whitespace
    token.line = current_line;
    token.column = current_column;

    if (input[pos] == '\0')
    {
        token.type = TOK_EOF;
        return token;
    }

    // Handle comments starting with '#'
    if (input[pos] == '#')
    {
        while (input[pos] != '\n' && input[pos] != '\0')
            advance_pos(1);
        return lexer_next_token(); // Skip the comment and get the next token
    }

    // Keywords and multi-char tokens (order matters)
    if (starts_with("let$"))
    {
        token.type = TOK_LET;
        strcpy(token.text, "let$");
        advance_pos(4);
        return token;
    }
    if (starts_with("::print"))
    {
        token.type = TOK_PRINT;
        strcpy(token.text, "::print");
        pos += 7;
        return token;
    }
    if (starts_with("::input"))
    {
        token.type = TOK_INPUT;
        strcpy(token.text, "::input");
        pos += 7;
        return token;
    }
    if (starts_with("if$"))
    {
        token.type = TOK_IF;
        strcpy(token.text, "if$");
        pos += 3;
        return token;
    }
    if (starts_with("elseif$"))
    {
        token.type = TOK_ELSEIF;
        strcpy(token.text, "elseif$");
        pos += 7;
        return token;
    }
    if (starts_with("else"))
    {
        token.type = TOK_ELSE;
        strcpy(token.text, "else");
        pos += 4;
        return token;
    }
    if (starts_with("loop$"))
    {
        token.type = TOK_LOOP;
        strcpy(token.text, "loop$");
        pos += 5;
        return token;
    }
    if (starts_with("foreach$"))
    {
        token.type = TOK_FOREACH;
        strcpy(token.text, "foreach$");
        pos += 8;
        return token;
    }
    if (starts_with("break") && !isalnum(input[pos + 5]))
    {
        token.type = TOK_BREAK;
        strcpy(token.text, "break");
        pos += 5;
        return token;
    }
    if (starts_with("continue") && !isalnum(input[pos + 8]))
    {
        token.type = TOK_CONTINUE;
        strcpy(token.text, "continue");
        pos += 8;
        return token;
    }
    if (starts_with("while$"))
    {
        token.type = TOK_WHILE;
        strcpy(token.text, "while$");
        pos += 6;
        return token;
    }
    if (starts_with("switch$"))
    {
        token.type = TOK_SWITCH;
        strcpy(token.text, "switch$");
        pos += 7;
        return token;
    }
    if (starts_with("case$"))
    {
        token.type = TOK_CASE;
        strcpy(token.text, "case$");
        pos += 5;
        return token;
    }
    if (starts_with("default$"))
    {
        token.type = TOK_DEFAULT;
        strcpy(token.text, "default$");
        pos += 8;
        return token;
    }
    if (starts_with("import$"))
    {
        token.type = TOK_IMPORT;
        strcpy(token.text, "import$");
        pos += 7;
        return token;
    }
    if (starts_with("func$"))
    {
        token.type = TOK_FUNC;
        strcpy(token.text, "func$");
        pos += 5;
        return token;
    }
    if (starts_with("class$"))
    {
        pos += 6;
        token.type = TOK_CLASS;
        strcpy(token.text, "class$");
        return token;
    }
    if (starts_with("self"))
    {
        pos += 4;
        token.type = TOK_SELF;
        strcpy(token.text, "self");
        return token;
    }

    if (starts_with("and"))
    {
        token.type = TOK_AND;
        strcpy(token.text, "and");
        pos += 3;
        return token;
    }
    if (starts_with("or"))
    {
        token.type = TOK_OR;
        strcpy(token.text, "or");
        pos += 2;
        return token;
    }
    if (starts_with("true") && !isalnum(input[pos + 4]))
    {
        token.type = TOK_TRUE;
        strcpy(token.text, "true");
        pos += 4;
        return token;
    }
    if (starts_with("false") && !isalnum(input[pos + 5]))
    {
        token.type = TOK_FALSE;
        strcpy(token.text, "false");
        pos += 5;
        return token;
    }
    if (starts_with("UNDEF") && !isalnum(input[pos + 5]))
    {
        token.type = TOK_UNDEF;
        strcpy(token.text, "UNDEF");
        pos += 5;
        return token;
    }
    if (starts_with("not"))
    {
        token.type = TOK_NOT;
        strcpy(token.text, "not");
        pos += 3;
        return token;
    }

    // Assignment operator :=
    if (input[pos] == ':' && safe_peek(1) == '=')
    {
        token.type = TOK_ASSIGN;
        strcpy(token.text, ":=");
        pos += 2;
        return token;
    }

    // Arrow operator =>
    if (input[pos] == '=' && safe_peek(1) == '>')
    {
        token.type = TOK_ARROW;
        strcpy(token.text, "=>");
        pos += 2;
        return token;
    }

    // Arrow operator ⟶ (UTF-8: E2 87 92)
    if ((unsigned char)input[pos] == 0xE2 && (unsigned char)safe_peek(1) == 0x87 && (unsigned char)safe_peek(2) == 0x92)
    {
        token.type = TOK_ARROW;
        strcpy(token.text, "⟶");
        pos += 3;
        return token;
    }

    if (starts_with("::push"))
    {
        token.type = TOK_STACK_PUSH;
        strcpy(token.text, "::push");
        pos += 6;
        return token;
    }
    if (starts_with("::pop"))
    {
        token.type = TOK_STACK_POP;
        strcpy(token.text, "::pop");
        pos += 5;
        return token;
    }
    if (starts_with("::peek"))
    {
        token.type = TOK_STACK_PEEK;
        strcpy(token.text, "::peek");
        pos += 6;
        return token;
    }
    if (starts_with("::enqueue"))
    {
        token.type = TOK_QUEUE_ENQUEUE;
        strcpy(token.text, "::enqueue");
        pos += 9;
        return token;
    }
    if (starts_with("::dequeue"))
    {
        token.type = TOK_QUEUE_DEQUEUE;
        strcpy(token.text, "::dequeue");
        pos += 9;
        return token;
    }
    if (starts_with("::front"))
    {
        token.type = TOK_QUEUE_FRONT;
        strcpy(token.text, "::front");
        pos += 7;
        return token;
    }
    if (starts_with("::back"))
    {
        token.type = TOK_QUEUE_BACK;
        strcpy(token.text, "::back");
        pos += 6;
        return token;
    }
    if (starts_with("::isEmpty"))
    {
        token.type = TOK_QUEUE_ISEMPTY;
        strcpy(token.text, "::isEmpty");
        pos += 9;
        return token;
    }
    if (starts_with("::qsize"))
    {
        token.type = TOK_QUEUE_SIZE;
        strcpy(token.text, "::qsize");
        pos += 7;
        return token;
    }
    if (starts_with("::size"))
    {
        token.type = TOK_STACK_SIZE;
        strcpy(token.text, "::size");
        pos += 6;
        return token;
    }
    if (starts_with("::empty"))
    {
        token.type = TOK_STACK_EMPTY;
        strcpy(token.text, "::empty");
        pos += 7;
        return token;
    }
    if (starts_with("::length"))
    {
        token.type = TOK_STRING_LENGTH;
        strcpy(token.text, "::length");
        pos += 8;
        return token;
    }
    if (starts_with("::len"))
    {
        token.type = TOK_LIST_LEN;
        strcpy(token.text, "::len");
        pos += 5;
        return token;
    }
    if (starts_with("::append"))
    {
        token.type = TOK_LIST_APPEND;
        strcpy(token.text, "::append");
        pos += 8;
        return token;
    }
    if (starts_with("::prepend"))
    {
        token.type = TOK_LIST_PREPEND;
        strcpy(token.text, "::prepend");
        pos += 9;
        return token;
    }
    if (starts_with("::insert"))
    {
        token.type = TOK_LIST_INSERT;
        strcpy(token.text, "::insert");
        pos += 8;
        return token;
    }
    if (starts_with("::remove"))
    {
        token.type = TOK_LIST_REMOVE;
        strcpy(token.text, "::remove");
        pos += 8;
        return token;
    }
    if (starts_with("::pattern_match"))
    {
        token.type = TOK_PATTERN_MATCH;
        strcpy(token.text, "::pattern_match");
        pos += 15;
        return token;
    }
    if (starts_with("::get"))
    {
        token.type = TOK_DICT_GET;
        strcpy(token.text, "::get");
        pos += 5;
        return token;
    }
    if (starts_with("::set"))
    {
        token.type = TOK_DICT_SET;
        strcpy(token.text, "::set");
        pos += 5;
        return token;
    }
    if (starts_with("::keys"))
    {
        token.type = TOK_DICT_KEYS;
        strcpy(token.text, "::keys");
        pos += 6;
        return token;
    }
    if (starts_with("::values"))
    {
        token.type = TOK_DICT_VALUES;
        strcpy(token.text, "::values");
        pos += 8;
        return token;
    }
    if (starts_with("dict"))
    {
        token.type = TOK_DICT_NEW;
        strcpy(token.text, "dict");
        pos += 4;
        return token;
    }
    if (starts_with("<stack>"))
    {
        token.type = TOK_STACK_NEW;
        strcpy(token.text, "<stack>");
        pos += 7;
        return token;
    }
    if (starts_with("<queue>"))
    {
        token.type = TOK_QUEUE_NEW;
        strcpy(token.text, "<queue>");
        pos += 7;
        return token;
    }
    if (starts_with("<linked>"))
    {
        token.type = TOK_LINKED_LIST_NEW;
        strcpy(token.text, "<linked>");
        pos += 8;
        return token;
    }
    if (starts_with("<temp@"))
    {
        // Parse <temp@N>
        int start_pos = pos;
        pos += 6; // Skip "<temp@"
        while (isdigit(input[pos])) pos++; // Skip digits
        if (input[pos] == '>')
        {
            pos++; // Skip '>'
            int len = pos - start_pos;
            if (len < sizeof(token.text))
            {
                strncpy(token.text, &input[start_pos], len);
                token.text[len] = '\0';
                token.type = TOK_TEMP_NEW;
                return token;
            }
        }
        // If parsing failed, reset position and continue
        pos = start_pos;
    }
    if (starts_with("::ladd"))
    {
        token.type = TOK_LINKED_LIST_ADD;
        strcpy(token.text, "::ladd");
        pos += 6;
        return token;
    }
    if (starts_with("::lremove"))
    {
        token.type = TOK_LINKED_LIST_REMOVE;
        strcpy(token.text, "::lremove");
        pos += 9;
        return token;
    }
    if (starts_with("::lget"))
    {
        token.type = TOK_LINKED_LIST_GET;
        strcpy(token.text, "::lget");
        pos += 6;
        return token;
    }
    if (starts_with("::lsize"))
    {
        token.type = TOK_LINKED_LIST_SIZE;
        strcpy(token.text, "::lsize");
        pos += 7;
        return token;
    }
    if (starts_with("::lisEmpty"))
    {
        token.type = TOK_LINKED_LIST_ISEMPTY;
        strcpy(token.text, "::lisEmpty");
        pos += 10;
        return token;
    }
    if (starts_with("::fopen"))
    {
        token.type = TOK_FILE_OPEN;
        strcpy(token.text, "::fopen");
        pos += 7;
        return token;
    }
    if (starts_with("::fread"))
    {
        token.type = TOK_FILE_READ;
        strcpy(token.text, "::fread");
        pos += 7;
        return token;
    }
    if (starts_with("::fwrite"))
    {
        token.type = TOK_FILE_WRITE;
        strcpy(token.text, "::fwrite");
        pos += 8;
        return token;
    }
    if (starts_with("::fclose"))
    {
        token.type = TOK_FILE_CLOSE;
        strcpy(token.text, "::fclose");
        pos += 8;
        return token;
    }
    if (starts_with("::to_str"))
    {
        token.type = TOK_TO_STR;
        strcpy(token.text, "::to_str");
        pos += 8;
        return token;
    }
    if (starts_with("::to_int"))
    {
        token.type = TOK_TO_INT;
        strcpy(token.text, "::to_int");
        pos += 8;
        return token;
    }
    if (starts_with("::type"))
    {
        token.type = TOK_TYPE;
        strcpy(token.text, "::type");
        pos += 6;
        return token;
    }
    if (starts_with("::split"))
    {
        token.type = TOK_STRING_SPLIT;
        strcpy(token.text, "::split");
        pos += 7;
        return token;
    }
    if (starts_with("::join"))
    {
        token.type = TOK_STRING_JOIN;
        strcpy(token.text, "::join");
        pos += 6;
        return token;
    }
    if (starts_with("::replace"))
    {
        token.type = TOK_STRING_REPLACE;
        strcpy(token.text, "::replace");
        pos += 9;
        return token;
    }
    if (starts_with("::substring"))
    {
        token.type = TOK_STRING_SUBSTRING;
        strcpy(token.text, "::substring");
        pos += 11;
        return token;
    }
    if (starts_with("::upper"))
    {
        token.type = TOK_STRING_UPPER;
        strcpy(token.text, "::upper");
        pos += 7;
        return token;
    }
    if (starts_with("::lower"))
    {
        token.type = TOK_STRING_LOWER;
        strcpy(token.text, "::lower");
        pos += 7;
        return token;
    }
    if (starts_with("::random"))
    {
        token.type = TOK_RANDOM;
        strcpy(token.text, "::random");
        pos += 8;
        return token;
    }
    if (starts_with("::http_get"))
    {
        token.type = TOK_HTTP_GET;
        strcpy(token.text, "::http_get");
        pos += 10;
        return token;
    }
    if (starts_with("::http_post"))
    {
        token.type = TOK_HTTP_POST;
        strcpy(token.text, "::http_post");
        pos += 11;
        return token;
    }
    if (starts_with("::http_put"))
    {
        token.type = TOK_HTTP_PUT;
        strcpy(token.text, "::http_put");
        pos += 10;
        return token;
    }
    if (starts_with("::http_delete"))
    {
        token.type = TOK_HTTP_DELETE;
        strcpy(token.text, "::http_delete");
        pos += 13;
        return token;
    }
    if (starts_with("<regex>"))
    {
        token.type = TOK_REGEX_NEW;
        strcpy(token.text, "<regex>");
        pos += 7;
        return token;
    }
    if (starts_with("::rmatch"))
    {
        token.type = TOK_REGEX_MATCH;
        strcpy(token.text, "::rmatch");
        pos += 8;
        return token;
    }
    if (starts_with("::rreplace"))
    {
        token.type = TOK_REGEX_REPLACE;
        strcpy(token.text, "::rreplace");
        pos += 10;
        return token;
    }
    if (starts_with("::rfind_all"))
    {
        token.type = TOK_REGEX_FIND_ALL;
        strcpy(token.text, "::rfind_all");
        pos += 11;
        return token;
    }
    if (starts_with("::temporal_aggregate"))
    {
        token.type = TOK_TEMPORAL_AGGREGATE;
        strcpy(token.text, "::temporal_aggregate");
        pos += 20;
        return token;
    }
    if (starts_with("::temporal_condition"))
    {
        token.type = TOK_TEMPORAL_CONDITION;
        strcpy(token.text, "::temporal_condition");
        pos += 20;
        return token;
    }
    if (starts_with("::temporal_pattern"))
    {
        token.type = TOK_TEMPORAL_PATTERN;
        strcpy(token.text, "::temporal_pattern");
        pos += 18;
        return token;
    }
    if (starts_with("::sliding_window_stats"))
    {
        token.type = TOK_SLIDING_WINDOW_STATS;
        strcpy(token.text, "::sliding_window_stats");
        pos += 22;
        return token;
    }
    if (starts_with("::sensitivity_threshold"))
    {
        token.type = TOK_SENSITIVITY_THRESHOLD;
        strcpy(token.text, "::sensitivity_threshold");
        pos += 23;
        return token;
    }
    if (starts_with("::temporal_query"))
    {
        token.type = TOK_TEMPORAL_QUERY;
        strcpy(token.text, "::temporal_query");
        pos += 16;
        return token;
    }
    if (starts_with("::temporal_correlate"))
    {
        token.type = TOK_TEMPORAL_CORRELATE;
        strcpy(token.text, "::temporal_correlate");
        pos += 20;
        return token;
    }
    if (starts_with("::temporal_interpolate"))
    {
        token.type = TOK_TEMPORAL_INTERPOLATE;
        strcpy(token.text, "::temporal_interpolate");
        pos += 22;
        return token;
    }
    if (starts_with("temporal$"))
    {
        token.type = TOK_TEMPORAL;
        strcpy(token.text, "temporal$");
        pos += 9;
        return token;
    }
    if (starts_with("in") && !isalnum(input[pos + 2]))
    {
        token.type = TOK_IN;
        strcpy(token.text, "in");
        pos += 2;
        return token;
    }
    if (starts_with("try$"))
    {
        token.type = TOK_TRY;
        strcpy(token.text, "try$");
        pos += 4;
        return token;
    }
    if (starts_with("catch$"))
    {
        token.type = TOK_CATCH;
        strcpy(token.text, "catch$");
        pos += 6;
        return token;
    }
    if (starts_with("throw$"))
    {
        token.type = TOK_THROW;
        strcpy(token.text, "throw$");
        pos += 6;
        return token;
    }
    if (starts_with("finally$"))
    {
        token.type = TOK_FINALLY;
        strcpy(token.text, "finally$");
        pos += 8;
        return token;
    }

    // Single char tokens and operators
    switch (input[pos])
    {
    case '+':
        token.type = TOK_PLUS;
        token.text[0] = '+';
        token.text[1] = '\0';
        pos++;
        return token;
    case '-':
        token.type = TOK_MINUS;
        token.text[0] = '-';
        token.text[1] = '\0';
        pos++;
        return token;
    case '*':
        token.type = TOK_MUL;
        token.text[0] = '*';
        token.text[1] = '\0';
        pos++;
        return token;
    case '/':
        token.type = TOK_DIV;
        token.text[0] = '/';
        token.text[1] = '\0';
        pos++;
        return token;
    case '%':
        token.type = TOK_MOD;
        token.text[0] = '%';
        token.text[1] = '\0';
        pos++;
        return token;
    case '(':
        token.type = TOK_LPAREN;
        token.text[0] = '(';
        token.text[1] = '\0';
        pos++;
        return token;
    case ')':
        token.type = TOK_RPAREN;
        token.text[0] = ')';
        token.text[1] = '\0';
        pos++;
        return token;
    case ';':
        token.type = TOK_SEMICOLON;
        token.text[0] = ';';
        token.text[1] = '\0';
        pos++;
        return token;
    case '>':
        if (safe_peek(1) == '=')
        {
            token.type = TOK_GTE;
            strcpy(token.text, ">=");
            pos += 2;
        }
        else
        {
            token.type = TOK_GT;
            token.text[0] = '>';
            token.text[1] = '\0';
            pos++;
        }
        return token;
    case '<':
        if (safe_peek(1) == '=')
        {
            token.type = TOK_LTE;
            strcpy(token.text, "<=");
            pos += 2;
        }
        else
        {
            token.type = TOK_LT;
            token.text[0] = '<';
            token.text[1] = '\0';
            pos++;
        }
        return token;
    case '=':
        if (safe_peek(1) == '=')
        {
            token.type = TOK_EQ;
            strcpy(token.text, "==");
            pos += 2;
            return token;
        }
        break;
    case '!':
        if (safe_peek(1) == '=')
        {
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
    case '[':
        token.type = TOK_LBRACKET;
        token.text[0] = '[';
        token.text[1] = '\0';
        pos++;
        return token;
    case ']':
        token.type = TOK_RBRACKET;
        token.text[0] = ']';
        token.text[1] = '\0';
        pos++;
        return token;
    case '&':
        token.type = TOK_BITWISE_AND;
        token.text[0] = '&';
        token.text[1] = '\0';
        pos++;
        return token;
    case '|':
        token.type = TOK_BITWISE_OR;
        token.text[0] = '|';
        token.text[1] = '\0';
        pos++;
        return token;
    case '^':
        token.type = TOK_BITWISE_XOR;
        token.text[0] = '^';
        token.text[1] = '\0';
        pos++;
        return token;
    case '~':
        token.type = TOK_BITWISE_NOT;
        token.text[0] = '~';
        token.text[1] = '\0';
        pos++;
        return token;
    case '@':
        token.type = TOK_AT;
        token.text[0] = '@';
        token.text[1] = '\0';
        pos++;
        return token;
    case '.':
        token.type = TOK_DOT;
        token.text[0] = '.';
        token.text[1] = '\0';
        pos++;
        return token;
    case '?':
        token.type = TOK_QUESTION;
        token.text[0] = '?';
        token.text[1] = '\0';
        pos++;
        return token;
    case ':':
        // Check if it's part of := assignment operator
        if (safe_peek(1) == '=')
        {
            // This will be handled by the := case above
            break;
        }
        token.type = TOK_COLON;
        token.text[0] = ':';
        token.text[1] = '\0';
        pos++;
        return token;
    }
    // String literal or interpolated string
    if (input[pos] == '"')
    {
        pos++;
        int start = pos;
        bool has_interpolation = false;
        
        // Check for ${} patterns
        int temp_pos = pos;
        while (input[temp_pos] != '"' && input[temp_pos] != '\0')
        {
            if (input[temp_pos] == '$' && input[temp_pos + 1] == '{')
            {
                has_interpolation = true;
                break;
            }
            if (input[temp_pos] == '\\' && input[temp_pos + 1] == '"')
            {
                temp_pos += 2;
            }
            else
            {
                temp_pos++;
            }
        }
        
        // Parse the string content
        while (input[pos] != '"' && input[pos] != '\0')
        {
            if (input[pos] == '\\' && input[pos + 1] == '"')
            {
                pos += 2;
            }
            else
            {
                pos++;
            }
        }
        
        int len = pos - start;
        if (len >= sizeof(token.text))
            len = sizeof(token.text) - 1;
        strncpy(token.text, &input[start], len);
        token.text[len] = '\0';
        
        token.type = has_interpolation ? TOK_INTERPOLATED_STRING : TOK_STRING;
        if (input[pos] == '"')
            pos++;
        
        // Check if this is followed by //flags (regex pattern)
        if (input[pos] == '/' && input[pos + 1] == '/')
        {
            pos += 2; // Skip //
            int flag_start = pos;
            while (isalpha(input[pos]))
                pos++;
            int flag_len = pos - flag_start;
            
            // Store pattern and flags in string_value for later parsing
            char combined[512];
            snprintf(combined, sizeof(combined), "%s//%.*s", token.text, flag_len, &input[flag_start]);
            strncpy(token.string_value, combined, sizeof(token.string_value));
            token.string_value[sizeof(token.string_value) - 1] = '\0';
        }
        
        return token;
    }

    // Number literal (integer or float)
    if (isdigit(input[pos]))
    {
        int start = pos;
        while (isdigit(input[pos]) || input[pos] == '.')
            pos++;
        int len = pos - start;
        if (len >= sizeof(token.text))
            len = sizeof(token.text) - 1;
        strncpy(token.text, &input[start], len);
        token.text[len] = '\0';
        token.type = TOK_NUMBER;
        return token;
    }

    // Identifier
    if (isalpha(input[pos]) || input[pos] == '$' || input[pos] == '_')
    {
        int start = pos;
        while (isalnum(input[pos]) || input[pos] == '$' || input[pos] == '_')
            pos++;
        int len = pos - start;
        if (len >= sizeof(token.text))
            len = sizeof(token.text) - 1;
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
