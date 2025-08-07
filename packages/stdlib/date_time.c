#include "../core/package_loader.h"
#include "../../include/ast.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

ASTNode *get_current_year(ASTNode **args, int arg_count) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    return ast_new_number(tm->tm_year + 1900);
}

ASTNode *get_current_month(ASTNode **args, int arg_count) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    return ast_new_number(tm->tm_mon + 1);
}

ASTNode *get_current_day(ASTNode **args, int arg_count) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    return ast_new_number(tm->tm_mday);
}

ASTNode *get_current_hour(ASTNode **args, int arg_count) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    return ast_new_number(tm->tm_hour);
}

ASTNode *get_current_minute(ASTNode **args, int arg_count) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    return ast_new_number(tm->tm_min);
}

ASTNode *get_current_second(ASTNode **args, int arg_count) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    return ast_new_number(tm->tm_sec);
}

ASTNode *get_timestamp(ASTNode **args, int arg_count) {
    time_t t = time(NULL);
    return ast_new_number((double)t);
}