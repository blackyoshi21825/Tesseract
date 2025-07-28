#include "error.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Global exception handling state
jmp_buf exception_env;
TesseractError current_error;
int exception_active = 0;

void error_init() {
    exception_active = 0;
    current_error.type = ERROR_NONE;
    current_error.message[0] = '\0';
    current_error.file[0] = '\0';
    current_error.line = 0;
}

void error_throw(ErrorType type, const char* message) {
    current_error.type = type;
    strncpy(current_error.message, message, sizeof(current_error.message) - 1);
    current_error.message[sizeof(current_error.message) - 1] = '\0';
    
    if (exception_active) {
        longjmp(exception_env, 1);
    } else {
        // No exception handler, print error and exit
        error_print(&current_error);
        exit(1);
    }
}

void error_set_location(const char* file, int line) {
    if (file) {
        strncpy(current_error.file, file, sizeof(current_error.file) - 1);
        current_error.file[sizeof(current_error.file) - 1] = '\0';
    }
    current_error.line = line;
}

const char* error_type_to_string(ErrorType type) {
    switch (type) {
        case ERROR_NONE: return "No Error";
        case ERROR_SYNTAX: return "Syntax Error";
        case ERROR_RUNTIME: return "Runtime Error";
        case ERROR_DIVISION_BY_ZERO: return "Division by Zero";
        case ERROR_UNDEFINED_VARIABLE: return "Undefined Variable";
        case ERROR_TYPE_MISMATCH: return "Type Mismatch";
        case ERROR_INDEX_OUT_OF_BOUNDS: return "Index Out of Bounds";
        case ERROR_FILE_NOT_FOUND: return "File Not Found";
        case ERROR_CUSTOM: return "Custom Error";
        default: return "Unknown Error";
    }
}

void error_print(const TesseractError* error) {
    printf("Error: %s - %s", error_type_to_string(error->type), error->message);
    if (error->file[0] != '\0') {
        printf(" (in %s", error->file);
        if (error->line > 0) {
            printf(":%d", error->line);
        }
        printf(")");
    }
    printf("\n");
}