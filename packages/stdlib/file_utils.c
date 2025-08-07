#include "../core/package_loader.h"
#include "../../include/ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

ASTNode *file_exists(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) return ast_new_number(0);
    
    FILE *file = fopen(args[0]->string, "r");
    if (file) {
        fclose(file);
        return ast_new_number(1);
    }
    return ast_new_number(0);
}

ASTNode *file_size(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) return ast_new_number(-1);
    
    struct stat st;
    if (stat(args[0]->string, &st) == 0) {
        return ast_new_number((double)st.st_size);
    }
    return ast_new_number(-1);
}

ASTNode *delete_file(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) return ast_new_number(0);
    
    return ast_new_number(remove(args[0]->string) == 0 ? 1 : 0);
}

ASTNode *copy_file(ASTNode **args, int arg_count) {
    if (arg_count != 2 || args[0]->type != NODE_STRING || args[1]->type != NODE_STRING) 
        return ast_new_number(0);
    
    FILE *src = fopen(args[0]->string, "rb");
    if (!src) return ast_new_number(0);
    
    FILE *dst = fopen(args[1]->string, "wb");
    if (!dst) {
        fclose(src);
        return ast_new_number(0);
    }
    
    char buffer[1024];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dst);
    }
    
    fclose(src);
    fclose(dst);
    return ast_new_number(1);
}