#include "../core/package_loader.h"
#include "../../include/ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

ASTNode *system_command(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) return ast_new_number(-1);
    
    int result = system(args[0]->string);
    return ast_new_number(result);
}

ASTNode *get_env_var(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) return ast_new_string("UNDEF");
    
    char *env_value = getenv(args[0]->string);
    if (env_value) {
        return ast_new_string(env_value);
    }
    return ast_new_string("UNDEF");
}

ASTNode *sleep_ms(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_NUMBER) return ast_new_number(0);
    
    int ms = (int)args[0]->number;
    #ifdef _WIN32
        Sleep(ms);
    #else
        usleep(ms * 1000);
    #endif
    return ast_new_number(1);
}

ASTNode *exit_program(ASTNode **args, int arg_count) {
    int code = 0;
    if (arg_count == 1 && args[0]->type == NODE_NUMBER) {
        code = (int)args[0]->number;
    }
    exit(code);
    return ast_new_number(0);
}

void init_system_utils_package() {
    register_package_function("system_command", system_command);
    register_package_function("get_env", get_env_var);
    register_package_function("sleep_ms", sleep_ms);
    register_package_function("exit", exit_program);
}