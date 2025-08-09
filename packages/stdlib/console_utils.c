#include "../core/package_loader.h"
#include "../../include/ast.h"
#include "../../include/variables.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#endif

// ANSI color codes
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"
#define UNDERLINE "\033[4m"

// Colored output functions
ASTNode *tesseract_print_red(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) return ast_new_number(0);
    printf("%s%s%s\n", RED, args[0]->string, RESET);
    return ast_new_number(1);
}

ASTNode *tesseract_print_green(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) return ast_new_number(0);
    printf("%s%s%s\n", GREEN, args[0]->string, RESET);
    return ast_new_number(1);
}

ASTNode *tesseract_print_yellow(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) return ast_new_number(0);
    printf("%s%s%s\n", YELLOW, args[0]->string, RESET);
    return ast_new_number(1);
}

ASTNode *tesseract_print_blue(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) return ast_new_number(0);
    printf("%s%s%s\n", BLUE, args[0]->string, RESET);
    return ast_new_number(1);
}

ASTNode *tesseract_print_bold(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) return ast_new_number(0);
    printf("%s%s%s\n", BOLD, args[0]->string, RESET);
    return ast_new_number(1);
}

ASTNode *tesseract_print_underline(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) return ast_new_number(0);
    printf("%s%s%s\n", UNDERLINE, args[0]->string, RESET);
    return ast_new_number(1);
}

// Screen control functions
ASTNode *tesseract_clear_screen(ASTNode **args, int arg_count) {
    system("clear");
    fflush(stdout);
    return ast_new_number(1);
}

ASTNode *tesseract_move_cursor(ASTNode **args, int arg_count) {
    if (arg_count != 2 || args[0]->type != NODE_NUMBER || args[1]->type != NODE_NUMBER) 
        return ast_new_number(0);
    int row = (int)args[0]->number;
    int col = (int)args[1]->number;
    printf("\033[%d;%dH", row, col);
    return ast_new_number(1);
}

// Input functions
ASTNode *tesseract_read_line(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) return ast_new_string("");
    
    printf("%s", args[0]->string);
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), stdin)) {
        // Remove newline
        buffer[strcspn(buffer, "\n")] = 0;
        set_variable("__function_return_str", buffer);
        return ast_new_string(buffer);
    }
    set_variable("__function_return_str", "");
    return ast_new_string("");
}

ASTNode *tesseract_read_number(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) return ast_new_number(0);
    
    printf("%s", args[0]->string);
    char buffer[64];
    if (fgets(buffer, sizeof(buffer), stdin)) {
        buffer[strcspn(buffer, "\n")] = 0;
        double num = strtod(buffer, NULL);
        snprintf(buffer, sizeof(buffer), "%g", num);
        set_variable("__function_return_str", buffer);
        return ast_new_number(num);
    }
    set_variable("__function_return_str", "0");
    return ast_new_number(0);
}

ASTNode *tesseract_read_yes_no(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) return ast_new_number(0);
    
    char input;
    do {
        printf("%s", args[0]->string);
        scanf(" %c", &input);
        while (getchar() != '\n'); // Clear buffer
        
        if (input == 'y' || input == 'Y') {
            set_variable("__function_return_str", "1");
            return ast_new_number(1);
        }
        if (input == 'n' || input == 'N') {
            set_variable("__function_return_str", "0");
            return ast_new_number(0);
        }
        
        printf("Please enter 'y' or 'n'\n");
    } while (1);
}

// Progress bar function
ASTNode *tesseract_progress_bar(ASTNode **args, int arg_count) {
    if (arg_count != 2 || args[0]->type != NODE_NUMBER || args[1]->type != NODE_NUMBER) 
        return ast_new_number(0);
    
    int current = (int)args[0]->number;
    int total = (int)args[1]->number;
    
    if (total <= 0) return ast_new_number(0);
    
    int width = 50;
    int filled = (current * width) / total;
    int percentage = (current * 100) / total;
    
    printf("\r[");
    for (int i = 0; i < width; i++) {
        if (i < filled) printf("█");
        else printf("░");
    }
    printf("] %d%%", percentage);
    fflush(stdout);
    
    return ast_new_number(1);
}

// Package initialization function
void init_console_utils_package() {
    register_package_function("print_red", tesseract_print_red);
    register_package_function("print_green", tesseract_print_green);
    register_package_function("print_yellow", tesseract_print_yellow);
    register_package_function("print_blue", tesseract_print_blue);
    register_package_function("print_bold", tesseract_print_bold);
    register_package_function("print_underline", tesseract_print_underline);
    register_package_function("clear_screen", tesseract_clear_screen);
    register_package_function("move_cursor", tesseract_move_cursor);
    register_package_function("read_line", tesseract_read_line);
    register_package_function("read_number", tesseract_read_number);
    register_package_function("read_yes_no", tesseract_read_yes_no);
    register_package_function("progress_bar", tesseract_progress_bar);
}