#include "../core/package_loader.h"
#include "../../include/ast.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Forward declaration
void ast_list_append(ASTNode *list, ASTNode *element) {
    ast_list_add_element(list, element);
}

static FILE *current_db = NULL;
static char current_db_path[256] = "";
static int last_id = 0;

ASTNode *tesseract_db_open(ASTNode **args, int arg_count)
{
    if (arg_count != 1 || args[0]->type != NODE_STRING)
        return ast_new_number(0);

    if (current_db) {
        fclose(current_db);
    }

    strncpy(current_db_path, args[0]->string, sizeof(current_db_path) - 1);
    current_db = fopen(current_db_path, "a+");
    
    return ast_new_number(current_db ? 1 : 0);
}

ASTNode *tesseract_db_close(ASTNode **args, int arg_count)
{
    if (current_db) {
        fclose(current_db);
        current_db = NULL;
    }
    return ast_new_number(1);
}

ASTNode *tesseract_db_execute(ASTNode **args, int arg_count)
{
    if (arg_count != 1 || args[0]->type != NODE_STRING || !current_db)
        return ast_new_number(0);

    fprintf(current_db, "%s\n", args[0]->string);
    fflush(current_db);
    last_id++;
    
    return ast_new_number(1);
}

ASTNode *tesseract_db_query(ASTNode **args, int arg_count)
{
    ASTNode *result = ast_new_list();
    if (!current_db) return result;
    
    rewind(current_db);
    char line[1024];
    while (fgets(line, sizeof(line), current_db)) {
        line[strcspn(line, "\n")] = 0;
        ast_list_add_element(result, ast_new_string(line));
    }
    
    return result;
}

ASTNode *tesseract_db_last_insert_id(ASTNode **args, int arg_count)
{
    return ast_new_number(last_id);
}

ASTNode *tesseract_db_changes(ASTNode **args, int arg_count)
{
    return ast_new_number(1);
}

ASTNode *tesseract_db_create_table(ASTNode **args, int arg_count)
{
    if (arg_count != 2 || args[0]->type != NODE_STRING || args[1]->type != NODE_STRING)
        return ast_new_number(0);
        
    if (current_db) {
        fprintf(current_db, "CREATE TABLE %s (%s)\n", args[0]->string, args[1]->string);
        fflush(current_db);
    }
    
    return ast_new_number(1);
}

// Package initialization function
void init_database_package()
{
    register_package_function("db_open", tesseract_db_open);
    register_package_function("db_close", tesseract_db_close);
    register_package_function("db_execute", tesseract_db_execute);
    register_package_function("db_query", tesseract_db_query);
    register_package_function("db_last_insert_id", tesseract_db_last_insert_id);
    register_package_function("db_changes", tesseract_db_changes);
    register_package_function("db_create_table", tesseract_db_create_table);
}