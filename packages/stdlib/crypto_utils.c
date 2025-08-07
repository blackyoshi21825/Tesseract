#include "../core/package_loader.h"
#include "../../include/ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ASTNode *simple_hash(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) return ast_new_number(0);
    
    char *str = args[0]->string;
    unsigned long hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    
    return ast_new_number((double)hash);
}

ASTNode *base64_encode_simple(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) return ast_new_string("");
    
    char *input = args[0]->string;
    int len = strlen(input);
    char *encoded = malloc(((len + 2) / 3) * 4 + 1);
    
    const char *chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i, j = 0;
    
    for (i = 0; i < len; i += 3) {
        int a = input[i];
        int b = (i + 1 < len) ? input[i + 1] : 0;
        int c = (i + 2 < len) ? input[i + 2] : 0;
        
        int bitmap = (a << 16) | (b << 8) | c;
        
        encoded[j++] = chars[(bitmap >> 18) & 63];
        encoded[j++] = chars[(bitmap >> 12) & 63];
        encoded[j++] = (i + 1 < len) ? chars[(bitmap >> 6) & 63] : '=';
        encoded[j++] = (i + 2 < len) ? chars[bitmap & 63] : '=';
    }
    
    encoded[j] = '\0';
    ASTNode *result = ast_new_string(encoded);
    free(encoded);
    return result;
}

ASTNode *generate_random_string(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_NUMBER) return ast_new_string("");
    
    int length = (int)args[0]->number;
    if (length <= 0) return ast_new_string("");
    
    char *str = malloc(length + 1);
    const char *chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    
    for (int i = 0; i < length; i++) {
        str[i] = chars[rand() % 62];
    }
    str[length] = '\0';
    
    ASTNode *result = ast_new_string(str);
    free(str);
    return result;
}

void init_crypto_utils_package() {
    register_package_function("simple_hash", simple_hash);
    register_package_function("base64_encode", base64_encode_simple);
    register_package_function("random_string", generate_random_string);
}