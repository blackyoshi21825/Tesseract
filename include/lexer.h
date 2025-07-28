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
    TOK_WHILE,
    TOK_SWITCH,
    TOK_CASE,
    TOK_DEFAULT,
    TOK_IMPORT,
    TOK_INPUT,
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
    TOK_CLASS,               // class$
    TOK_DOT,                 // .
    TOK_SELF,                // self
    TOK_DICT_NEW,            // dict
    TOK_DICT_GET,            // ::get
    TOK_DICT_SET,            // ::set
    TOK_DICT_KEYS,           // ::keys
    TOK_DICT_VALUES,         // ::values
    TOK_STACK_NEW,           // <stack>
    TOK_STACK_PUSH,          // ::push
    TOK_STACK_POP,           // ::pop
    TOK_STACK_PEEK,          // ::peek
    TOK_STACK_SIZE,          // ::size
    TOK_STACK_EMPTY,         // ::empty
    TOK_QUEUE_NEW,           // <queue>
    TOK_QUEUE_ENQUEUE,       // ::enqueue
    TOK_QUEUE_DEQUEUE,       // ::dequeue
    TOK_QUEUE_FRONT,         // ::front
    TOK_QUEUE_BACK,          // ::back
    TOK_QUEUE_ISEMPTY,       // ::isEmpty
    TOK_QUEUE_SIZE,          // ::qsize
    TOK_LINKED_LIST_NEW,     // <linked>
    TOK_LINKED_LIST_ADD,     // ::ladd
    TOK_LINKED_LIST_REMOVE,  // ::lremove
    TOK_LINKED_LIST_GET,     // ::lget
    TOK_LINKED_LIST_SIZE,    // ::lsize
    TOK_LINKED_LIST_ISEMPTY, // ::lisEmpty
    TOK_FILE_OPEN,
    TOK_FILE_READ,
    TOK_FILE_WRITE,
    TOK_FILE_CLOSE,
    TOK_TO_STR,
    TOK_TO_INT,
    TOK_HTTP_GET,
    TOK_HTTP_POST,
    TOK_HTTP_PUT,
    TOK_HTTP_DELETE,
    TOK_REGEX_NEW,           // <regex>
    TOK_REGEX_MATCH,         // ::rmatch
    TOK_REGEX_REPLACE,       // ::rreplace
    TOK_REGEX_FIND_ALL,      // ::rfind_all
    TOK_QUESTION,            // ?
    TOK_COLON,               // :
    TOK_TRUE,                // true
    TOK_FALSE,               // false
    TOK_AT,                  // @
    TOK_TEMPORAL,            // temporal$
    TOK_IN,                  // in
    TOK_TEMP_NEW,            // <temp@N>
    TOK_TEMPORAL_AGGREGATE,  // ::temporal_aggregate
    TOK_TEMPORAL_PATTERN,    // ::temporal_pattern
    TOK_TEMPORAL_CONDITION,  // ::temporal_condition
    TOK_SLIDING_WINDOW_STATS, // ::sliding_window_stats
    TOK_SENSITIVITY_THRESHOLD, // ::sensitivity_threshold
    TOK_TEMPORAL_QUERY,      // ::temporal_query
    TOK_TEMPORAL_CORRELATE,  // ::temporal_correlate
    TOK_TEMPORAL_INTERPOLATE, // ::temporal_interpolate
    TOK_TRY,                 // try$
    TOK_CATCH,               // catch$
    TOK_THROW,               // throw$
    TOK_FINALLY,             // finally$
    TOK_LAMBDA,              // =>
    TOK_INTERPOLATED_STRING, // "Hello ${name}"
    TOK_SET_NEW,             // {1, 2, 3}
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
