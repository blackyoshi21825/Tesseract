#include "../core/package_loader.h"
#include "../../include/ast.h"
#include <string.h>
#include <ctype.h>

// JSON utility functions for Tesseract
ASTNode *tesseract_json_escape(ASTNode **args, int arg_count)
{
    if (arg_count != 1 || args[0]->type != NODE_STRING)
        return ast_new_string("");

    char *str = args[0]->string;
    int len = strlen(str);
    char *result = malloc(len * 2 + 1);
    int pos = 0;
    
    for (int i = 0; i < len; i++)
    {
        char c = str[i];
        switch (c)
        {
            case '"':
                result[pos++] = '\\';
                result[pos++] = '"';
                break;
            case '\\':
                result[pos++] = '\\';
                result[pos++] = '\\';
                break;
            case '\n':
                result[pos++] = '\\';
                result[pos++] = 'n';
                break;
            case '\r':
                result[pos++] = '\\';
                result[pos++] = 'r';
                break;
            case '\t':
                result[pos++] = '\\';
                result[pos++] = 't';
                break;
            default:
                result[pos++] = c;
                break;
        }
    }
    result[pos] = '\0';
    
    ASTNode *node = ast_new_string(result);
    free(result);
    return node;
}

ASTNode *tesseract_json_unescape(ASTNode **args, int arg_count)
{
    if (arg_count != 1 || args[0]->type != NODE_STRING)
        return ast_new_string("");

    char *str = args[0]->string;
    int len = strlen(str);
    char *result = malloc(len + 1);
    int pos = 0;
    
    for (int i = 0; i < len; i++)
    {
        if (str[i] == '\\' && i + 1 < len)
        {
            switch (str[i + 1])
            {
                case '"':
                    result[pos++] = '"';
                    i++;
                    break;
                case '\\':
                    result[pos++] = '\\';
                    i++;
                    break;
                case 'n':
                    result[pos++] = '\n';
                    i++;
                    break;
                case 'r':
                    result[pos++] = '\r';
                    i++;
                    break;
                case 't':
                    result[pos++] = '\t';
                    i++;
                    break;
                default:
                    result[pos++] = str[i];
                    break;
            }
        }
        else
        {
            result[pos++] = str[i];
        }
    }
    result[pos] = '\0';
    
    ASTNode *node = ast_new_string(result);
    free(result);
    return node;
}

ASTNode *tesseract_is_valid_json_string(ASTNode **args, int arg_count)
{
    if (arg_count != 1 || args[0]->type != NODE_STRING)
        return ast_new_number(0);

    char *str = args[0]->string;
    int len = strlen(str);
    
    if (len < 2 || str[0] != '"' || str[len-1] != '"')
        return ast_new_number(0);
    
    for (int i = 1; i < len - 1; i++)
    {
        if (str[i] == '"' && str[i-1] != '\\')
            return ast_new_number(0);
    }
    
    return ast_new_number(1);
}

ASTNode *tesseract_json_format_number(ASTNode **args, int arg_count)
{
    if (arg_count != 1 || args[0]->type != NODE_NUMBER)
        return ast_new_string("0");

    double num = args[0]->number;
    char buffer[32];
    
    if (num == (int)num)
        sprintf(buffer, "%d", (int)num);
    else
        sprintf(buffer, "%.6g", num);
    
    return ast_new_string(buffer);
}

// Package initialization function
void init_json_utils_package()
{
    register_package_function("json_escape", tesseract_json_escape);
    register_package_function("json_unescape", tesseract_json_unescape);
    register_package_function("is_valid_json_string", tesseract_is_valid_json_string);
    register_package_function("json_format_number", tesseract_json_format_number);
}