#include "../core/package_loader.h"
#include "../../include/ast.h"
#include <string.h>
#include <ctype.h>

// String utility functions for Tesseract
ASTNode *tesseract_str_reverse(ASTNode **args, int arg_count)
{
    if (arg_count != 1 || args[0]->type != NODE_STRING)
        return ast_new_string("");

    char *str = args[0]->string;
    int len = strlen(str);
    char *result = malloc(len + 1);

    for (int i = 0; i < len; i++)
    {
        result[i] = str[len - 1 - i];
    }
    result[len] = '\0';

    ASTNode *node = ast_new_string(result);
    free(result);
    return node;
}

ASTNode *tesseract_str_contains(ASTNode **args, int arg_count)
{
    if (arg_count != 2 || args[0]->type != NODE_STRING || args[1]->type != NODE_STRING)
        return ast_new_number(0);

    char *haystack = args[0]->string;
    char *needle = args[1]->string;

    return ast_new_number(strstr(haystack, needle) != NULL ? 1 : 0);
}

ASTNode *tesseract_str_starts_with(ASTNode **args, int arg_count)
{
    if (arg_count != 2 || args[0]->type != NODE_STRING || args[1]->type != NODE_STRING)
        return ast_new_number(0);

    char *str = args[0]->string;
    char *prefix = args[1]->string;

    return ast_new_number(strncmp(str, prefix, strlen(prefix)) == 0 ? 1 : 0);
}

ASTNode *tesseract_str_ends_with(ASTNode **args, int arg_count)
{
    if (arg_count != 2 || args[0]->type != NODE_STRING || args[1]->type != NODE_STRING)
        return ast_new_number(0);

    char *str = args[0]->string;
    char *suffix = args[1]->string;
    int str_len = strlen(str);
    int suffix_len = strlen(suffix);

    if (suffix_len > str_len)
        return ast_new_number(0);

    return ast_new_number(strcmp(str + str_len - suffix_len, suffix) == 0 ? 1 : 0);
}

ASTNode *tesseract_str_trim(ASTNode **args, int arg_count)
{
    if (arg_count != 1 || args[0]->type != NODE_STRING)
        return ast_new_string("");

    char *str = args[0]->string;
    int len = strlen(str);

    // Find start
    int start = 0;
    while (start < len && isspace(str[start]))
        start++;

    // Find end
    int end = len - 1;
    while (end >= start && isspace(str[end]))
        end--;

    int new_len = end - start + 1;
    char *result = malloc(new_len + 1);
    strncpy(result, str + start, new_len);
    result[new_len] = '\0';

    ASTNode *node = ast_new_string(result);
    free(result);
    return node;
}

ASTNode *tesseract_str_repeat(ASTNode **args, int arg_count)
{
    if (arg_count != 2 || args[0]->type != NODE_STRING || args[1]->type != NODE_NUMBER)
        return ast_new_string("");

    char *str = args[0]->string;
    int count = (int)args[1]->number;

    if (count <= 0)
        return ast_new_string("");

    int str_len = strlen(str);
    char *result = malloc(str_len * count + 1);
    result[0] = '\0';

    for (int i = 0; i < count; i++)
    {
        strcat(result, str);
    }

    ASTNode *node = ast_new_string(result);
    free(result);
    return node;
}

// Package initialization function
void init_string_utils_package()
{
    register_package_function("str_reverse", tesseract_str_reverse);
    register_package_function("str_contains", tesseract_str_contains);
    register_package_function("str_starts_with", tesseract_str_starts_with);
    register_package_function("str_ends_with", tesseract_str_ends_with);
    register_package_function("str_trim", tesseract_str_trim);
    register_package_function("str_repeat", tesseract_str_repeat);
}