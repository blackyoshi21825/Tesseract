#ifndef VARIABLES_H
#define VARIABLES_H
#include "ast.h"

#define MAX_TEMPORAL_HISTORY 10

typedef struct {
    char *value;
    int timestamp;
} TemporalEntry;

typedef struct {
    TemporalEntry history[MAX_TEMPORAL_HISTORY];
    int current_index;
    int count;
    int max_history;
} TemporalVariable;

void set_variable(const char *name, const char *value);
void set_list_variable(const char *name, ASTNode *list);
void set_dict_variable(const char *name, ASTNode *dict);
void set_stack_variable(const char *name, ASTNode *stack);
void set_queue_variable(const char *name, ASTNode *queue);
void set_linked_list_variable(const char *name, ASTNode *list);
void set_regex_variable(const char *name, ASTNode *regex);
void set_set_variable(const char *name, ASTNode *set);
const char *get_variable(const char *name);
ASTNode *get_list_variable(const char *name);
ASTNode *get_dict_variable(const char *name);
ASTNode *get_stack_variable(const char *name);
ASTNode *get_queue_variable(const char *name);
ASTNode *get_linked_list_variable(const char *name);
ASTNode *get_regex_variable(const char *name);
ASTNode *get_set_variable(const char *name);

// Temporal variable functions
void set_temporal_variable(const char *name, const char *value, int max_history);
const char *get_temporal_variable(const char *name, int time_offset);
int get_temporal_variable_count(const char *name);
TemporalVariable *get_temporal_var_struct(const char *name);

#endif
