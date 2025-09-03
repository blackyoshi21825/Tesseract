#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "lexer.h"

extern int debug_mode;

static const char *input;
static int pos;
static int current_line;
static int current_column;

// Token mapping table - the key to making it compact!
typedef struct {
    const char *text;
    TokenType type;
    int length;
} TokenMapping;

// Sorted by length (longest first) to avoid partial matches
static TokenMapping token_map[] = {
    // Longest tokens first
    {"::sliding_window_stats", TOK_SLIDING_WINDOW_STATS, 22},
    {"::sensitivity_threshold", TOK_SENSITIVITY_THRESHOLD, 23},
    {"::temporal_interpolate", TOK_TEMPORAL_INTERPOLATE, 22},
    {"::temporal_aggregate", TOK_TEMPORAL_AGGREGATE, 20},
    {"::temporal_condition", TOK_TEMPORAL_CONDITION, 20},
    {"::temporal_correlate", TOK_TEMPORAL_CORRELATE, 20},
    {"::pattern_match", TOK_PATTERN_MATCH, 15},
    {"::gremove_vertex", TOK_GRAPH_REMOVE_VERTEX, 16},
    {"::symmetric_diff", TOK_SET_SYMMETRIC_DIFF, 16},
    {"::temporal_query", TOK_TEMPORAL_QUERY, 16},
    {"::gremove_edge", TOK_GRAPH_REMOVE_EDGE, 14},
    {"::intersection", TOK_SET_INTERSECTION, 14},
    {"::gadd_vertex", TOK_GRAPH_ADD_VERTEX, 13},
    {"::http_delete", TOK_HTTP_DELETE, 13},
    {"::difference", TOK_SET_DIFFERENCE, 12},
    {"::tpostorder", TOK_TREE_POSTORDER, 12},
    {"::gneighbors", TOK_GRAPH_NEIGHBORS, 12},
    {"::rfind_all", TOK_REGEX_FIND_ALL, 11},
    {"::tpreorder", TOK_TREE_PREORDER, 11},
    {"::substring", TOK_STRING_SUBSTRING, 11},
    {"::scontains", TOK_SET_CONTAINS, 11},
    {"::http_post", TOK_HTTP_POST, 11},
    {"::gadd_edge", TOK_GRAPH_ADD_EDGE, 11},
    {"::lisEmpty", TOK_LINKED_LIST_ISEMPTY, 10},
    {"::rreplace", TOK_REGEX_REPLACE, 10},
    {"::lremove", TOK_LINKED_LIST_REMOVE, 9},
    {"::tinorder", TOK_TREE_INORDER, 10},
    {"::tdelete", TOK_TREE_DELETE, 9},
    {"::tsearch", TOK_TREE_SEARCH, 9},
    {"::tinsert", TOK_TREE_INSERT, 9},
    {"::sremove", TOK_SET_REMOVE, 9},
    {"::prepend", TOK_LIST_PREPEND, 9},
    {"::enqueue", TOK_QUEUE_ENQUEUE, 9},
    {"::dequeue", TOK_QUEUE_DEQUEUE, 9},
    {"::replace", TOK_STRING_REPLACE, 9},
    {"::http_put", TOK_HTTP_PUT, 10},
    {"::http_get", TOK_HTTP_GET, 10},
    {"temporal$", TOK_TEMPORAL, 9},
    {"continue", TOK_CONTINUE, 8},
    {"default$", TOK_DEFAULT, 8},
    {"foreach$", TOK_FOREACH, 8},
    {"finally$", TOK_FINALLY, 8},
    {"::append", TOK_LIST_APPEND, 8},
    {"::insert", TOK_LIST_INSERT, 8},
    {"::remove", TOK_LIST_REMOVE, 8},
    {"::length", TOK_STRING_LENGTH, 8},
    {"::values", TOK_DICT_VALUES, 8},
    {"::fwrite", TOK_FILE_WRITE, 8},
    {"::fclose", TOK_FILE_CLOSE, 8},
    {"::to_str", TOK_TO_STR, 8},
    {"::to_int", TOK_TO_INT, 8},
    {"::rmatch", TOK_REGEX_MATCH, 8},
    {"::random", TOK_RANDOM, 8},
    {"::sclear", TOK_SET_CLEAR, 8},
    {"::sempty", TOK_SET_EMPTY, 8},
    {"elseif$", TOK_ELSEIF, 7},
    {"import$", TOK_IMPORT, 7},
    {"switch$", TOK_SWITCH, 7},
    {"::print", TOK_PRINT, 7},
    {"::input", TOK_INPUT, 7},
    {"::front", TOK_QUEUE_FRONT, 7},
    {"::qsize", TOK_QUEUE_SIZE, 7},
    {"::empty", TOK_STACK_EMPTY, 7},
    {"::split", TOK_STRING_SPLIT, 7},
    {"::upper", TOK_STRING_UPPER, 7},
    {"::lower", TOK_STRING_LOWER, 7},
    {"::union", TOK_SET_UNION, 7},
    {"<queue>", TOK_QUEUE_NEW, 7},
    {"<regex>", TOK_REGEX_NEW, 7},
    {"::lsize", TOK_LINKED_LIST_SIZE, 7},
    {"::ssize", TOK_SET_SIZE, 7},
    {"::scopy", TOK_SET_COPY, 7},
    {"while$", TOK_WHILE, 6},
    {"class$", TOK_CLASS, 6},
    {"catch$", TOK_CATCH, 6},
    {"throw$", TOK_THROW, 6},
    {"yield$", TOK_YIELD, 6},
    {"::push", TOK_STACK_PUSH, 6},
    {"::peek", TOK_STACK_PEEK, 6},
    {"::back", TOK_QUEUE_BACK, 6},
    {"::keys", TOK_DICT_KEYS, 6},
    {"::fopen", TOK_FILE_OPEN, 7},
    {"::fread", TOK_FILE_READ, 7},
    {"::type", TOK_TYPE, 6},
    {"::join", TOK_STRING_JOIN, 6},
    {"::ladd", TOK_LINKED_LIST_ADD, 6},
    {"::lget", TOK_LINKED_LIST_GET, 6},
    {"::sadd", TOK_SET_ADD, 6},
    {"::gdfs", TOK_GRAPH_DFS, 6},
    {"::gbfs", TOK_GRAPH_BFS, 6},
    {"<tree>", TOK_TREE_NEW, 6},
    {"<stack>", TOK_STACK_NEW, 7},
    {"<linked>", TOK_LINKED_LIST_NEW, 8},
    {"<graph>", TOK_GRAPH_NEW, 7},
    {"loop$", TOK_LOOP, 5},
    {"func$", TOK_FUNC, 5},
    {"case$", TOK_CASE, 5},
    {"iter$", TOK_ITERATOR, 5},
    {"next$", TOK_NEXT, 5},
    {"break", TOK_BREAK, 5},
    {"false", TOK_FALSE, 5},
    {"UNDEF", TOK_UNDEF, 5},
    {"::pop", TOK_STACK_POP, 5},
    {"::len", TOK_LIST_LEN, 5},
    {"::get", TOK_DICT_GET, 5},
    {"::set", TOK_DICT_SET, 5},
    {"let$", TOK_LET, 4},
    {"if$", TOK_IF, 3},
    {"else", TOK_ELSE, 4},
    {"try$", TOK_TRY, 4},
    {"gen$", TOK_GENERATOR, 4},
    {"true", TOK_TRUE, 4},
    {"self", TOK_SELF, 4},
    {"dict", TOK_DICT_NEW, 4},
    {"::size", TOK_STACK_SIZE, 6},
    {"and", TOK_AND, 3},
    {"not", TOK_NOT, 3},
    {"or", TOK_OR, 2},
    {"in", TOK_IN, 2},
    {":=", TOK_ASSIGN, 2},
    {"=>", TOK_ARROW, 2},
    {"++", TOK_INCREMENT, 2},
    {"--", TOK_DECREMENT, 2},
    {"+=", TOK_PLUS_ASSIGN, 2},
    {"-=", TOK_MINUS_ASSIGN, 2},
    {"*=", TOK_MUL_ASSIGN, 2},
    {"/=", TOK_DIV_ASSIGN, 2},
    {"%=", TOK_MOD_ASSIGN, 2},
    {">=", TOK_GTE, 2},
    {"<=", TOK_LTE, 2},
    {"==", TOK_EQ, 2},
    {"!=", TOK_NEQ, 2},
    {NULL, TOK_UNKNOWN, 0} // Sentinel
};

void lexer_init(const char *source) {
    input = source;
    pos = 0;
    current_line = 1;
    current_column = 1;
}

static void advance_pos(int count) {
    for (int i = 0; i < count; i++) {
        if (input[pos] == '\n') {
            current_line++;
            current_column = 1;
        } else {
            current_column++;
        }
        pos++;
    }
}

static void skip_whitespace() {
    while (isspace(input[pos])) {
        advance_pos(1);
    }
}

static int safe_peek(int offset) {
    if (input[pos + offset] == '\0') return 0;
    return input[pos + offset];
}

// Compact token lookup function
static TokenMapping* find_token_mapping() {
    for (int i = 0; token_map[i].text != NULL; i++) {
        if (strncmp(&input[pos], token_map[i].text, token_map[i].length) == 0) {
            // For keywords that end with letters, check word boundary
            if (isalpha(token_map[i].text[token_map[i].length - 1])) {
                char next_char = input[pos + token_map[i].length];
                if (isalnum(next_char) || next_char == '_' || next_char == '$') {
                    continue; // Not a word boundary
                }
            }
            return &token_map[i];
        }
    }
    return NULL;
}

// Handle <temp@N> pattern
static bool try_parse_temp_token(Token *token) {
    if (strncmp(&input[pos], "<temp@", 6) != 0) return false;

    int start_pos = pos;
    pos += 6;
    while (isdigit(input[pos])) pos++;
    if (input[pos] != '>') {
        pos = start_pos;
        return false;
    }
    pos++; // Skip '>'

    int len = pos - start_pos;
    if (len < sizeof(token->text)) {
        strncpy(token->text, &input[start_pos], len);
        token->text[len] = '\0';
        token->type = TOK_TEMP_NEW;
        return true;
    }

    pos = start_pos;
    return false;
}

const char* token_type_name(TokenType type) {
    switch(type) {
        case TOK_ID: return "ID";
        case TOK_NUMBER: return "NUMBER";
        case TOK_STRING: return "STRING";
        case TOK_LET: return "LET";
        case TOK_FUNC: return "FUNC";
        case TOK_IF: return "IF";
        case TOK_PRINT: return "PRINT";
        case TOK_ASSIGN: return "ASSIGN";
        case TOK_PLUS: return "PLUS";
        case TOK_MINUS: return "MINUS";
        case TOK_EOF: return "EOF";
        default: return "OTHER";
    }
}

Token debug_return_token(Token token) {
    if (debug_mode) {
        printf("[DEBUG] Token: %s '%s' at %d:%d\n",
               token_type_name(token.type), token.text, token.line, token.column);
    }
    return token;
}

Token lexer_next_token() {
    Token token;
    token.type = TOK_UNKNOWN;
    token.text[0] = '\0';
    token.line = current_line;
    token.column = current_column;

    skip_whitespace();
    token.line = current_line;
    token.column = current_column;

    if (input[pos] == '\0') {
        token.type = TOK_EOF;
        return token;
    }

    // Handle comments
    if (input[pos] == '#') {
        while (input[pos] != '\n' && input[pos] != '\0') advance_pos(1);
        return lexer_next_token();
    }

    // Try <temp@N> pattern first
    if (try_parse_temp_token(&token)) {
        return debug_return_token(token);
    }

    // Try UTF-8 arrow âŸ¶
    if ((unsigned char)input[pos] == 0xE2 &&
        (unsigned char)safe_peek(1) == 0x87 &&
        (unsigned char)safe_peek(2) == 0x92) {
        token.type = TOK_ARROW;
        strcpy(token.text, "âŸ¶");
        pos += 3;
        return debug_return_token(token);
    }

    // **THE KEY CHANGE**: Use table lookup instead of massive if/else chain
    TokenMapping* mapping = find_token_mapping();
    if (mapping) {
        token.type = mapping->type;
        strncpy(token.text, mapping->text, mapping->length);
        token.text[mapping->length] = '\0';
        pos += mapping->length;
        return debug_return_token(token);
    }

    // Single character operators (compact switch)
    char ch = input[pos];
    switch (ch) {
    case '+': case '-': case '*': case '/': case '%':
    case '(': case ')': case ';': case '>': case '<':
    case '=': case '!': case ',': case '{': case '}':
    case '[': case ']': case '&': case '|': case '^':
    case '~': case '@': case '.': case '?': case ':':
        {
            // Check for compound operators first
            char next = safe_peek(1);
            TokenType compound_type = TOK_UNKNOWN;

            if ((ch == '>' && next == '=') || (ch == '<' && next == '=') ||
                (ch == '=' && next == '=') || (ch == '!' && next == '=')) {
                pos += 2;
                token.text[0] = ch;
                token.text[1] = next;
                token.text[2] = '\0';
                token.type = (ch == '>' && next == '=') ? TOK_GTE :
                            (ch == '<' && next == '=') ? TOK_LTE :
                            (ch == '=' && next == '=') ? TOK_EQ : TOK_NEQ;
                return debug_return_token(token);
            }

            // Single character
            pos++;
            token.text[0] = ch;
            token.text[1] = '\0';
            token.type = (ch == '+') ? TOK_PLUS : (ch == '-') ? TOK_MINUS :
                        (ch == '*') ? TOK_MUL : (ch == '/') ? TOK_DIV :
                        (ch == '%') ? TOK_MOD : (ch == '(') ? TOK_LPAREN :
                        (ch == ')') ? TOK_RPAREN : (ch == ';') ? TOK_SEMICOLON :
                        (ch == '>') ? TOK_GT : (ch == '<') ? TOK_LT :
                        (ch == ',') ? TOK_COMMA : (ch == '{') ? TOK_LBRACE :
                        (ch == '}') ? TOK_RBRACE : (ch == '[') ? TOK_LBRACKET :
                        (ch == ']') ? TOK_RBRACKET : (ch == '&') ? TOK_BITWISE_AND :
                        (ch == '|') ? TOK_BITWISE_OR : (ch == '^') ? TOK_BITWISE_XOR :
                        (ch == '~') ? TOK_BITWISE_NOT : (ch == '@') ? TOK_AT :
                        (ch == '.') ? TOK_DOT : (ch == '?') ? TOK_QUESTION :
                        (ch == ':') ? TOK_COLON : TOK_UNKNOWN;
            return debug_return_token(token);
        }
    }

    // String literals
    if (input[pos] == '"') {
        pos++;
        int start = pos;
        bool has_interpolation = false;

        // Check for interpolation
        int temp_pos = pos;
        while (input[temp_pos] != '"' && input[temp_pos] != '\0') {
            if (input[temp_pos] == '$' && input[temp_pos + 1] == '{') {
                has_interpolation = true;
                break;
            }
            if (input[temp_pos] == '\\' && input[temp_pos + 1] == '"') {
                temp_pos += 2;
            } else {
                temp_pos++;
            }
        }

        // Parse content
        while (input[pos] != '"' && input[pos] != '\0') {
            if (input[pos] == '\\' && input[pos + 1] == '"') {
                pos += 2;
            } else {
                pos++;
            }
        }

        int len = pos - start;
        if (len >= sizeof(token.text)) len = sizeof(token.text) - 1;
        strncpy(token.text, &input[start], len);
        token.text[len] = '\0';

        token.type = has_interpolation ? TOK_INTERPOLATED_STRING : TOK_STRING;
        if (input[pos] == '"') pos++;

        // Handle regex flags
        if (input[pos] == '/' && input[pos + 1] == '/') {
            pos += 2;
            int flag_start = pos;
            while (isalpha(input[pos])) pos++;
            int flag_len = pos - flag_start;

            char combined[512];
            snprintf(combined, sizeof(combined), "%s//%.*s", token.text, flag_len, &input[flag_start]);
            strncpy(token.string_value, combined, sizeof(token.string_value));
            token.string_value[sizeof(token.string_value) - 1] = '\0';
        }

        return debug_return_token(token);
    }

    // Numbers
    if (isdigit(input[pos])) {
        int start = pos;
        while (isdigit(input[pos]) || input[pos] == '.') pos++;
        int len = pos - start;
        if (len >= sizeof(token.text)) len = sizeof(token.text) - 1;
        strncpy(token.text, &input[start], len);
        token.text[len] = '\0';
        token.type = TOK_NUMBER;
        return debug_return_token(token);
    }

    // Identifiers
    if (isalpha(input[pos]) || input[pos] == '$' || input[pos] == '_') {
        int start = pos;
        while (isalnum(input[pos]) || input[pos] == '$' || input[pos] == '_') pos++;
        int len = pos - start;
        if (len >= sizeof(token.text)) len = sizeof(token.text) - 1;
        strncpy(token.text, &input[start], len);
        token.text[len] = '\0';
        token.type = TOK_ID;
        return debug_return_token(token);
    }

    // Unknown character
    token.type = TOK_UNKNOWN;
    token.text[0] = input[pos];
    token.text[1] = '\0';
    pos++;
    return debug_return_token(token);
}