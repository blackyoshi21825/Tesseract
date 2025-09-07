#include "../core/package_loader.h"
#include "../../include/ast.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

// Network utility functions for Tesseract
ASTNode *tesseract_is_valid_ip(ASTNode **args, int arg_count)
{
    if (arg_count != 1 || args[0]->type != NODE_STRING)
        return ast_new_number(0);

    char *ip = args[0]->string;
    int parts = 0;
    char *token = strtok(strdup(ip), ".");
    
    while (token != NULL && parts < 4)
    {
        int num = atoi(token);
        if (num < 0 || num > 255)
            return ast_new_number(0);
        parts++;
        token = strtok(NULL, ".");
    }
    
    return ast_new_number(parts == 4 ? 1 : 0);
}

ASTNode *tesseract_is_valid_email(ASTNode **args, int arg_count)
{
    if (arg_count != 1 || args[0]->type != NODE_STRING)
        return ast_new_number(0);

    char *email = args[0]->string;
    char *at_pos = strchr(email, '@');
    
    if (!at_pos || at_pos == email || at_pos == email + strlen(email) - 1)
        return ast_new_number(0);
    
    char *dot_pos = strrchr(at_pos, '.');
    if (!dot_pos || dot_pos == at_pos + 1)
        return ast_new_number(0);
    
    return ast_new_number(1);
}

ASTNode *tesseract_url_encode(ASTNode **args, int arg_count)
{
    if (arg_count != 1 || args[0]->type != NODE_STRING)
        return ast_new_string("");

    char *str = args[0]->string;
    int len = strlen(str);
    char *result = malloc(len * 3 + 1);
    int pos = 0;
    
    for (int i = 0; i < len; i++)
    {
        char c = str[i];
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
        {
            result[pos++] = c;
        }
        else
        {
            sprintf(result + pos, "%%%02X", (unsigned char)c);
            pos += 3;
        }
    }
    result[pos] = '\0';
    
    ASTNode *node = ast_new_string(result);
    free(result);
    return node;
}

ASTNode *tesseract_extract_domain(ASTNode **args, int arg_count)
{
    if (arg_count != 1 || args[0]->type != NODE_STRING)
        return ast_new_string("");

    char *url = args[0]->string;
    char *protocol = strstr(url, "://");
    char *start = protocol ? protocol + 3 : url;
    char *end = strchr(start, '/');
    
    int len = end ? end - start : strlen(start);
    char *domain = malloc(len + 1);
    strncpy(domain, start, len);
    domain[len] = '\0';
    
    ASTNode *node = ast_new_string(domain);
    free(domain);
    return node;
}

// Package initialization function
void init_network_utils_package()
{
    register_package_function("is_valid_ip", tesseract_is_valid_ip);
    register_package_function("is_valid_email", tesseract_is_valid_email);
    register_package_function("url_encode", tesseract_url_encode);
    register_package_function("extract_domain", tesseract_extract_domain);
}