#ifndef ERROR_H
#define ERROR_H

#include <setjmp.h>

typedef enum {
    ERROR_NONE,
    ERROR_SYNTAX,
    ERROR_RUNTIME,
    ERROR_DIVISION_BY_ZERO,
    ERROR_UNDEFINED_VARIABLE,
    ERROR_TYPE_MISMATCH,
    ERROR_INDEX_OUT_OF_BOUNDS,
    ERROR_FILE_NOT_FOUND,
    ERROR_CUSTOM
} ErrorType;

typedef struct {
    ErrorType type;
    char message[256];
    char file[64];
    int line;
} TesseractError;

// Exception handling state
extern jmp_buf exception_env;
extern TesseractError current_error;
extern int exception_active;

// Error functions
void error_init();
void error_throw(ErrorType type, const char* message);
void error_throw_at_line(ErrorType type, const char* message, int line);
void error_set_location(const char* file, int line);
void error_set_current_file(const char* filename);
const char* error_type_to_string(ErrorType type);
void error_print(const TesseractError* error);

// Macros for easier exception handling
#define TRY() (setjmp(exception_env) == 0)
#define THROW(type, msg) error_throw(type, msg)
#define CATCH() else

#endif