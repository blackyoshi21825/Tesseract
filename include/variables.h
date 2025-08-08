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

typedef struct {
    char name[64];
    ASTNode *body;
    char params[4][64];
    int param_count;
} Generator;

typedef struct {
    Generator *generator;
    ASTNode **arg_values;
    int arg_count;
    int current_position;
    int is_exhausted;
    ASTNode *current_yield_value;
} Iterator;

void set_variable(const char *name, const char *value);
void set_list_variable(const char *name, ASTNode *list);
void set_dict_variable(const char *name, ASTNode *dict);
void set_stack_variable(const char *name, ASTNode *stack);
void set_queue_variable(const char *name, ASTNode *queue);
void set_linked_list_variable(const char *name, ASTNode *list);
void set_regex_variable(const char *name, ASTNode *regex);
void set_set_variable(const char *name, ASTNode *set);
void set_undef_variable(const char *name);
const char *get_variable(const char *name);
ASTNode *get_list_variable(const char *name);
ASTNode *get_dict_variable(const char *name);
ASTNode *get_stack_variable(const char *name);
ASTNode *get_queue_variable(const char *name);
ASTNode *get_linked_list_variable(const char *name);
ASTNode *get_regex_variable(const char *name);
ASTNode *get_set_variable(const char *name);
int is_undef_variable(const char *name);

// Temporal variable functions
void set_temporal_variable(const char *name, const char *value, int max_history);
const char *get_temporal_variable(const char *name, int time_offset);
int get_temporal_variable_count(const char *name);
TemporalVariable *get_temporal_var_struct(const char *name);

// Generator and iterator functions
void register_generator(const char *name, char params[][64], int param_count, ASTNode *body);
Generator *find_generator(const char *name);
void set_iterator_variable(const char *name, Iterator *iterator);
Iterator *get_iterator_variable(const char *name);
Iterator *create_iterator(Generator *gen, ASTNode **args, int arg_count);
ASTNode *iterator_next(Iterator *iter);
void free_iterator(Iterator *iter);

#endif
