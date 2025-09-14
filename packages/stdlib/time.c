#include "../core/package_loader.h"
#include "../../include/ast.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#endif

ASTNode *tesseract_sleep(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_NUMBER)
        return ast_new_number(0);

    double seconds = args[0]->number;
    if (seconds < 0)
        return ast_new_number(0);

#ifdef _WIN32
    Sleep((DWORD)(seconds * 1000));
#else
    usleep((unsigned int)(seconds * 1000000));
#endif

    return ast_new_number(1);
}

ASTNode *tesseract_time(ASTNode **args, int arg_count) {
    time_t t = time(NULL);
    return ast_new_number((double)t);
}

ASTNode *tesseract_clock(ASTNode **args, int arg_count) {
    clock_t c = clock();
    return ast_new_number((double)c / CLOCKS_PER_SEC);
}

ASTNode *tesseract_delay(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_NUMBER)
        return ast_new_number(0);

    double milliseconds = args[0]->number;
    if (milliseconds < 0)
        return ast_new_number(0);

#ifdef _WIN32
    Sleep((DWORD)milliseconds);
#else
    usleep((unsigned int)(milliseconds * 1000));
#endif

    return ast_new_number(1);
}

ASTNode *tesseract_elapsed(ASTNode **args, int arg_count) {
    static clock_t start_time = 0;
    
    if (arg_count == 0) {
        // Start timing
        start_time = clock();
        return ast_new_number(0);
    } else {
        // Get elapsed time
        clock_t current_time = clock();
        double elapsed = (double)(current_time - start_time) / CLOCKS_PER_SEC;
        return ast_new_number(elapsed);
    }
}

void init_time_package() {
    register_package_function("sleep", tesseract_sleep);
    register_package_function("time", tesseract_time);
    register_package_function("clock", tesseract_clock);
    register_package_function("delay", tesseract_delay);
    register_package_function("elapsed", tesseract_elapsed);
}