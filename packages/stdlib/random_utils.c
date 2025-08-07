#include "../core/package_loader.h"
#include "../../include/ast.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>

static int random_initialized = 0;

// Random utility functions for Tesseract
ASTNode *tesseract_random_int(ASTNode **args, int arg_count)
{
    if (!random_initialized)
    {
        srand(time(NULL));
        random_initialized = 1;
    }
    
    if (arg_count == 0)
        return ast_new_number(rand());
    
    if (arg_count == 1 && args[0]->type == NODE_NUMBER)
    {
        int max = (int)args[0]->number;
        return ast_new_number(rand() % max);
    }
    
    if (arg_count == 2 && args[0]->type == NODE_NUMBER && args[1]->type == NODE_NUMBER)
    {
        int min = (int)args[0]->number;
        int max = (int)args[1]->number;
        return ast_new_number(min + rand() % (max - min + 1));
    }
    
    return ast_new_number(0);
}

ASTNode *tesseract_random_float(ASTNode **args, int arg_count)
{
    if (!random_initialized)
    {
        srand(time(NULL));
        random_initialized = 1;
    }
    
    double r = (double)rand() / RAND_MAX;
    
    if (arg_count == 0)
        return ast_new_number(r);
    
    if (arg_count == 1 && args[0]->type == NODE_NUMBER)
    {
        double max = args[0]->number;
        return ast_new_number(r * max);
    }
    
    if (arg_count == 2 && args[0]->type == NODE_NUMBER && args[1]->type == NODE_NUMBER)
    {
        double min = args[0]->number;
        double max = args[1]->number;
        return ast_new_number(min + r * (max - min));
    }
    
    return ast_new_number(0);
}

ASTNode *tesseract_random_choice(ASTNode **args, int arg_count)
{
    if (arg_count != 1 || args[0]->type != NODE_LIST || args[0]->list.count == 0)
        return ast_new_number(0);
    
    if (!random_initialized)
    {
        srand(time(NULL));
        random_initialized = 1;
    }
    
    ASTNode *list = args[0];
    int index = rand() % list->list.count;
    return list->list.elements[index];
}

ASTNode *tesseract_random_string(ASTNode **args, int arg_count)
{
    if (arg_count != 1 || args[0]->type != NODE_NUMBER)
        return ast_new_string("");
    
    if (!random_initialized)
    {
        srand(time(NULL));
        random_initialized = 1;
    }
    
    int length = (int)args[0]->number;
    if (length <= 0)
        return ast_new_string("");
    
    char *chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int chars_len = strlen(chars);
    char *result = malloc(length + 1);
    
    for (int i = 0; i < length; i++)
    {
        result[i] = chars[rand() % chars_len];
    }
    result[length] = '\0';
    
    ASTNode *node = ast_new_string(result);
    free(result);
    return node;
}

// Package initialization function
void init_random_utils_package()
{
    register_package_function("random_int", tesseract_random_int);
    register_package_function("random_float", tesseract_random_float);
    register_package_function("random_choice", tesseract_random_choice);
    register_package_function("random_string", tesseract_random_string);
}