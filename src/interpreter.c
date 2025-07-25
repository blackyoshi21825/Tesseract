#include "tesseract_pch.h"
#include <ctype.h>

#define MAX_FUNCTIONS 1000000
#define MAX_CLASSES 1000000
#define MAX_FILE_HANDLES 1024

static FILE *file_handles[MAX_FILE_HANDLES];
static int file_handle_count = 0;

typedef struct ObjectInstance ObjectInstance;

const char *bool_to_str(bool value);

// Flag to track if HTTP has been initialized
static bool http_initialized = false;

// Structure to store response data
struct ResponseData
{
    char *data;
    size_t size;
};

// Callback function for libcurl to write response data
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct ResponseData *resp = (struct ResponseData *)userp;

    char *ptr = realloc(resp->data, resp->size + realsize + 1);
    if (!ptr)
    {
        printf("Error: Memory allocation failed\n");
        return 0;
    }

    resp->data = ptr;
    memcpy(&(resp->data[resp->size]), contents, realsize);
    resp->size += realsize;
    resp->data[resp->size] = '\0';

    return realsize;
}

// Initialize HTTP functionality
static void init_http()
{
    if (!http_initialized)
    {
        curl_global_init(CURL_GLOBAL_ALL);
        http_initialized = true;
    }
}

// Perform HTTP request and return response as string
static char *perform_http_request(const char *url, const char *method, const char *data, ASTNode *headers)
{
    init_http();

    CURL *curl;
    CURLcode res;
    struct ResponseData response_data;
    struct curl_slist *header_list = NULL;

    // Initialize response data
    response_data.data = malloc(1);
    response_data.size = 0;
    response_data.data[0] = '\0';

    curl = curl_easy_init();
    if (!curl)
    {
        free(response_data.data);
        return strdup("Error: Failed to initialize curl");
    }

    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, url);

    // Always add these headers to prevent libcurl's automatic form encoding
    header_list = curl_slist_append(header_list, "Content-Type: application/json");

    // Set request method
    if (strcmp(method, "POST") == 0)
    {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        if (data)
        {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(data));
        }
    }
    else if (strcmp(method, "PUT") == 0)
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        if (data)
        {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(data));
        }
    }
    else if (strcmp(method, "DELETE") == 0)
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }

    // Set headers if provided
    if (headers && headers->type == NODE_DICT)
    {
        for (int i = 0; i < headers->dict.count; i++)
        {
            if (headers->dict.keys[i]->type == NODE_STRING && headers->dict.values[i]->type == NODE_STRING)
            {
                char header_line[512];
                snprintf(header_line, sizeof(header_line), "%s: %s",
                         headers->dict.keys[i]->string,
                         headers->dict.values[i]->string);
                header_list = curl_slist_append(header_list, header_line);
            }
        }
    }

    // Always set the header list to ensure Content-Type is applied
    if (header_list)
    {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
    }

    // Set callback function to receive response
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response_data);

    // Set user agent
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Tesseract/1.0");

    // Perform the request
    res = curl_easy_perform(curl);

    // Clean up
    if (header_list)
    {
        curl_slist_free_all(header_list);
    }
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Error: %s", curl_easy_strerror(res));
        free(response_data.data);
        return strdup(error_msg);
    }

    return response_data.data;
}

typedef struct
{
    char name[64];
    ASTNode *body;
    char params[4][64];
    int param_count;
} Function;

typedef struct
{
    char name[64];
    ASTNode *class_node;
} ClassEntry;

static Function function_table[MAX_FUNCTIONS];
static int function_count = 0;

static ClassEntry class_table[MAX_CLASSES];
static int class_count = 0;

static double eval_expression(ASTNode *node);
static char *list_to_string(ASTNode *list);
static char *get_string_value(ASTNode *node);

// Forward declaration for file reading
char *read_file(const char *filename);

int add_file_handle(FILE *f)
{
    if (file_handle_count >= MAX_FILE_HANDLES)
    {
        return -1; // No space left
    }
    file_handles[file_handle_count] = f;
    return file_handle_count++;
}

FILE *get_file_handle(int handle)
{
    if (handle < 0 || handle >= file_handle_count || file_handles[handle] == NULL)
    {
        return NULL; // Invalid handle
    }
    return file_handles[handle];
}

void remove_file_handle(int handle)
{
    if (handle >= 0 && handle < file_handle_count)
    {
        if (file_handles[handle] != NULL)
        {
            fclose(file_handles[handle]);
            file_handles[handle] = NULL;
        }
    }
}

// Forward declaration of print_node
static void print_node(ASTNode *node);

// Forward declaration for HTTP request functions
static char *perform_http_request(const char *url, const char *method, const char *data, ASTNode *headers);
static void init_http();

// Forward declarations for regex functions
static int regex_match_pattern(const char *pattern, const char *text, const char *flags);
static void regex_find_all_matches(const char *pattern, const char *text, const char *flags, ASTNode *result_list);
static char *regex_replace_pattern(const char *pattern, const char *text, const char *replacement, const char *flags);
static int regex_match_at_position(const char *pattern, const char *text, int case_insensitive);
static int regex_get_match_length(const char *pattern, const char *text, int case_insensitive);
static int regex_match_char(char pattern_char, char text_char, int case_insensitive);

static FieldEntry *object_get_field(ObjectInstance *obj, const char *field);

ObjectInstance *object_new(const char *class_name)
{
    ObjectInstance *obj = malloc(sizeof(ObjectInstance));
    if (!obj)
    {
        printf("Memory error: Could not allocate object\n");
        exit(1);
    }

    strncpy(obj->class_name, class_name, sizeof(obj->class_name));
    obj->class_name[sizeof(obj->class_name) - 1] = '\0';
    obj->fields = NULL;
    return obj;
}

void object_free(ObjectInstance *obj)
{
    if (!obj)
        return;

    FieldEntry *entry = obj->fields;
    while (entry)
    {
        FieldEntry *next = entry->next;
        free(entry);
        entry = next;
    }

    free(obj);
}

void register_function(const char *name, char params[][64], int param_count, ASTNode *body)
{
    if (function_count >= MAX_FUNCTIONS)
    {
        printf("Function table overflow!\n");
        exit(1);
    }
    strcpy(function_table[function_count].name, name);
    function_table[function_count].body = body;
    function_table[function_count].param_count = param_count;
    for (int i = 0; i < param_count; i++)
    {
        strcpy(function_table[function_count].params[i], params[i]);
    }
    function_count++;
}

Function *find_function(const char *name)
{
    for (int i = 0; i < function_count; i++)
    {
        if (strcmp(function_table[i].name, name) == 0)
        {
            return &function_table[i];
        }
    }
    return NULL;
}

void register_class(const char *name, ASTNode *class_node)
{
    for (int i = 0; i < class_count; i++)
    {
        if (strcmp(class_table[i].name, name) == 0)
        {
            class_table[i].class_node = class_node;
            return;
        }
    }
    if (class_count < MAX_CLASSES)
    {
        strncpy(class_table[class_count].name, name, 64);
        class_table[class_count].name[63] = '\0';
        class_table[class_count].class_node = class_node;
        class_count++;
    }
}

ASTNode *get_class(const char *name)
{
    for (int i = 0; i < class_count; i++)
    {
        if (strcmp(class_table[i].name, name) == 0)
        {
            return class_table[i].class_node;
        }
    }
    return NULL;
}

ASTNode *instantiate_class(const char *name, ASTNode **args, int arg_count)
{
    ASTNode *class_node = get_class(name);
    if (!class_node)
        return NULL;
    // For now, just create a class instance node
    return ast_new_class_instance(name, args, arg_count);
}

// Helper to set a field on an object
static void object_set_field(ObjectInstance *obj, const char *field, double number_value, const char *string_value, FieldType type)
{
    FieldEntry *entry = object_get_field(obj, field);
    if (!entry)
    {
        entry = malloc(sizeof(FieldEntry));
        strcpy(entry->name, field);
        entry->next = obj->fields;
        obj->fields = entry;
    }
    entry->type = type;
    if (type == FIELD_STRING)
    {
        if (string_value)
        {
            strncpy(entry->string_value, string_value, sizeof(entry->string_value));
            entry->string_value[sizeof(entry->string_value) - 1] = '\0';
        }
        else
        {
            entry->string_value[0] = '\0';
        }
    }
    else
    {
        entry->number_value = number_value;
    }
}

// Helper to get a field from an object
static FieldEntry *object_get_field(ObjectInstance *obj, const char *field)
{
    FieldEntry *entry = obj->fields;
    while (entry)
    {
        if (strcmp(entry->name, field) == 0)
        {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}
// Store the current self object (for method calls)
static ObjectInstance *current_self = NULL;

void interpret(ASTNode *root)
{
    if (!root)
        return;
    if (root->type == NODE_BLOCK)
    {
        for (int i = 0; i < root->block.count; i++)
        {
            if (root->block.statements[i]) // Only interpret non-NULL statements
                interpret(root->block.statements[i]);
        }
        return;
    }
    else if (root->type == NODE_ASSIGN)
    {
        ASTNode *value_node = root->assign.value;
        
        // Check if this is a temporal variable initialization (let$ x := <temp@5>)
        if (value_node->type == NODE_TEMPORAL_VAR)
        {
            int max_history = (int)value_node->number;
            // Initialize with empty value - will be set on first assignment
            set_temporal_variable(root->assign.varname, "0", max_history);
            return;
        }
        
        if (value_node->type == NODE_STRING)
        {
            set_variable(root->assign.varname, value_node->string);
        }
        else if (value_node->type == NODE_INPUT)
        {
            interpret(value_node); // This will read input and set __last_input
            const char *input_val = get_variable("__last_input");
            if (input_val)
            {
                set_variable(root->assign.varname, input_val);
            }
        }
        else if (value_node->type == NODE_LIST)
        {
            set_list_variable(root->assign.varname, value_node);
        }
        else if (value_node->type == NODE_DICT)
        {
            set_dict_variable(root->assign.varname, value_node);
        }
        else if (value_node->type == NODE_STACK)
        {
            set_stack_variable(root->assign.varname, value_node);
        }
        else if (value_node->type == NODE_QUEUE)
        {
            set_queue_variable(root->assign.varname, value_node);
        }
        else if (value_node->type == NODE_LINKED_LIST)
        {
            set_linked_list_variable(root->assign.varname, value_node);
        }
        else if (value_node->type == NODE_REGEX)
        {
            set_regex_variable(root->assign.varname, value_node);
        }
        else if (value_node->type == NODE_FILE_READ)
        {
            eval_expression(value_node); // This will read input and set __last_file_read
            const char *read_val = get_variable("__last_file_read");
            if (read_val)
            {
                set_variable(root->assign.varname, read_val);
            }
        }
        else if (value_node->type == NODE_TO_STR)
        {
            double result = eval_expression(value_node);
            if (result == -12345.6789)
            {
                const char *str_result = get_variable("__to_str_result");
                if (str_result)
                {
                    set_variable(root->assign.varname, str_result);
                }
            }
            else
            {
                char buf[64];
                snprintf(buf, sizeof(buf), "%g", result);
                set_variable(root->assign.varname, buf);
            }
        }
        else if (value_node->type == NODE_TERNARY)
        {
            // Handle ternary operator specially to preserve string results
            double condition = eval_expression(value_node->ternary.condition);
            ASTNode *result_expr = (condition != 0) ? value_node->ternary.true_expr : value_node->ternary.false_expr;
            
            if (result_expr->type == NODE_STRING)
            {
                set_variable(root->assign.varname, result_expr->string);
            }
            else
            {
                double val = eval_expression(result_expr);
                
                // Check if this is a boolean context (ternary with true/false literals)
                // Look at both branches to see if they are boolean literals
                int is_boolean_context = 0;
                if ((value_node->ternary.true_expr->type == NODE_NUMBER && 
                     (value_node->ternary.true_expr->number == 1.0 || value_node->ternary.true_expr->number == 0.0)) ||
                    (value_node->ternary.false_expr->type == NODE_NUMBER && 
                     (value_node->ternary.false_expr->number == 1.0 || value_node->ternary.false_expr->number == 0.0)))
                {
                    is_boolean_context = 1;
                }
                
                // Also check if the condition is a boolean operation
                if ((value_node->ternary.condition->type == NODE_BINOP &&
                     (value_node->ternary.condition->binop.op == TOK_EQ || value_node->ternary.condition->binop.op == TOK_NEQ ||
                      value_node->ternary.condition->binop.op == TOK_LT || value_node->ternary.condition->binop.op == TOK_GT ||
                      value_node->ternary.condition->binop.op == TOK_LTE || value_node->ternary.condition->binop.op == TOK_GTE)) ||
                    value_node->ternary.condition->type == NODE_AND || value_node->ternary.condition->type == NODE_OR || value_node->ternary.condition->type == NODE_NOT)
                {
                    is_boolean_context = 1;
                }
                
                if (is_boolean_context && (val == 1.0 || val == 0.0))
                {
                    set_variable(root->assign.varname, val == 1.0 ? "true" : "false");
                }
                else
                {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "%g", val);
                    set_variable(root->assign.varname, buf);
                }
            }
        }
        else if (value_node->type == NODE_CLASS_INSTANCE)
        {
            // Create the object instance
            ObjectInstance *obj = object_new(value_node->class_instance.class_name);
            // Initialize fields from class definition
            ASTNode *class_node = get_class(value_node->class_instance.class_name);
            if (class_node)
            {
                ASTNode *body = class_node->class_def.body;
                for (int i = 0; i < body->block.count; i++)
                {
                    ASTNode *stmt = body->block.statements[i];
                    if (stmt->type == NODE_ASSIGN)
                    {
                        if (stmt->assign.value->type == NODE_STRING)
                        {
                            object_set_field(obj, stmt->assign.varname, 0, stmt->assign.value->string, FIELD_STRING);
                        }
                        else
                        {
                            double val = eval_expression(stmt->assign.value);
                            object_set_field(obj, stmt->assign.varname, val, NULL, FIELD_NUMBER);
                        }
                    }
                }
            }
            // Store the pointer as a string in the variable table under the correct variable name
            char buf[32];
            snprintf(buf, sizeof(buf), "%p", (void *)obj);
            set_variable(root->assign.varname, buf);
        }
        else
        {
            double val = eval_expression(value_node);
            char buf[64];
            snprintf(buf, sizeof(buf), "%g", val);
            
            // Check if this is a temporal variable
            TemporalVariable *temp_var = get_temporal_var_struct(root->assign.varname);
            if (temp_var)
            {
                set_temporal_variable(root->assign.varname, buf, temp_var->max_history);
            }
            else
            {
                set_variable(root->assign.varname, buf);
            }
        }
    }
    else if (root->type == NODE_INPUT)
    {
        if (root->input_stmt.prompt)
        {
            if (root->input_stmt.prompt->type == NODE_STRING)
            {
                printf("%s", root->input_stmt.prompt->string);
            }
            else
            {
                // For now, we don't handle dynamic prompts
            }
        }

        char buffer[1024];
        if (fgets(buffer, sizeof(buffer), stdin))
        {
            buffer[strcspn(buffer, "\n")] = 0; // Remove newline
            set_variable("__last_input", buffer);
        }
    }
    else if (root->type == NODE_PRINT)
    {
        if (root->binop.left->type == NODE_LIST_ACCESS)
        {
            double result = eval_expression(root->binop.left);
            printf("%g\n", result);
        }
        else if (root->binop.left->type == NODE_STACK_POP || root->binop.left->type == NODE_STACK_PEEK ||
                 root->binop.left->type == NODE_QUEUE_DEQUEUE || root->binop.left->type == NODE_QUEUE_FRONT)
        {
            // For operations that may print strings directly, only print numeric results
            double result = eval_expression(root->binop.left);
            if (result != 0 ||
                (root->binop.left->type != NODE_QUEUE_DEQUEUE &&
                 root->binop.left->type != NODE_QUEUE_FRONT))
            {
                printf("%g\n", result);
            }
        }
        else if (root->binop.left->type == NODE_TERNARY)
        {
            // Handle ternary operator specially for printing
            double condition = eval_expression(root->binop.left->ternary.condition);
            ASTNode *result_expr = (condition != 0) ? root->binop.left->ternary.true_expr : root->binop.left->ternary.false_expr;
            
            if (result_expr->type == NODE_STRING)
            {
                printf("%s\n", result_expr->string);
            }
            else
            {
                double result = eval_expression(result_expr);
                
                // Check if this is a boolean context (ternary with true/false literals)
                int is_boolean_context = 0;
                if ((root->binop.left->ternary.true_expr->type == NODE_NUMBER && 
                     (root->binop.left->ternary.true_expr->number == 1.0 || root->binop.left->ternary.true_expr->number == 0.0)) ||
                    (root->binop.left->ternary.false_expr->type == NODE_NUMBER && 
                     (root->binop.left->ternary.false_expr->number == 1.0 || root->binop.left->ternary.false_expr->number == 0.0)))
                {
                    is_boolean_context = 1;
                }
                
                // Also check if the condition is a boolean operation
                if ((root->binop.left->ternary.condition->type == NODE_BINOP &&
                     (root->binop.left->ternary.condition->binop.op == TOK_EQ || root->binop.left->ternary.condition->binop.op == TOK_NEQ ||
                      root->binop.left->ternary.condition->binop.op == TOK_LT || root->binop.left->ternary.condition->binop.op == TOK_GT ||
                      root->binop.left->ternary.condition->binop.op == TOK_LTE || root->binop.left->ternary.condition->binop.op == TOK_GTE)) ||
                    root->binop.left->ternary.condition->type == NODE_AND || root->binop.left->ternary.condition->type == NODE_OR || root->binop.left->ternary.condition->type == NODE_NOT)
                {
                    is_boolean_context = 1;
                }
                
                // Check if the result expression itself is a boolean operation
                if ((result_expr->type == NODE_BINOP &&
                     (result_expr->binop.op == TOK_EQ || result_expr->binop.op == TOK_NEQ ||
                      result_expr->binop.op == TOK_LT || result_expr->binop.op == TOK_GT ||
                      result_expr->binop.op == TOK_LTE || result_expr->binop.op == TOK_GTE)) ||
                    result_expr->type == NODE_AND || result_expr->type == NODE_OR || result_expr->type == NODE_NOT)
                {
                    is_boolean_context = 1;
                }
                
                if (is_boolean_context && (result == 1.0 || result == 0.0))
                {
                    printf("%s\n", bool_to_str((bool)result));
                }
                else
                {
                    printf("%g\n", result);
                }
            }
        }
        else if (root->binop.left->type == NODE_AND ||
                 root->binop.left->type == NODE_OR ||
                 root->binop.left->type == NODE_NOT ||
                 (root->binop.left->type == NODE_BINOP &&
                  (root->binop.left->binop.op == TOK_EQ ||
                   root->binop.left->binop.op == TOK_NEQ ||
                   root->binop.left->binop.op == TOK_LT ||
                   root->binop.left->binop.op == TOK_GT ||
                   root->binop.left->binop.op == TOK_LTE ||
                   root->binop.left->binop.op == TOK_GTE)))
        {
            // Handle boolean operations and comparison operators
            double result = eval_expression(root->binop.left);
            printf("%s\n", bool_to_str((bool)result));
        }
        else
        {
            print_node(root->binop.left);
        }
    }
    else if (root->type == NODE_IF)
    {
        ASTNode *current = root;
        while (current)
        {
            double cond = eval_expression(current->if_stmt.condition);
            if (cond != 0)
            {
                interpret(current->if_stmt.then_branch);
                return;
            }
            if (current->if_stmt.elseif_branch)
            {
                current = current->if_stmt.elseif_branch;
            }
            else
            {
                break;
            }
        }
        // If we reach here, none of the conditions matched; execute else_branch of the top-level if
        if (root->if_stmt.else_branch)
        {
            interpret(root->if_stmt.else_branch);
        }
    }
    else if (root->type == NODE_LOOP)
    {
        double start = eval_expression(root->loop_stmt.start);
        double end = eval_expression(root->loop_stmt.end);
        double increment = 1.0; // Default increment
        
        if (root->loop_stmt.increment)
        {
            increment = eval_expression(root->loop_stmt.increment);
        }
        
        if (increment == 0)
        {
            printf("Runtime error: Loop increment cannot be zero\n");
            exit(1);
        }
        
        if (increment > 0)
        {
            // Positive increment (ascending)
            for (double i = start; i <= end; i += increment)
            {
                char buf[64];
                snprintf(buf, sizeof(buf), "%g", i);
                set_variable(root->loop_stmt.varname, buf);
                interpret(root->loop_stmt.body);
            }
        }
        else
        {
            // Negative increment (descending)
            for (double i = start; i >= end; i += increment)
            {
                char buf[64];
                snprintf(buf, sizeof(buf), "%g", i);
                set_variable(root->loop_stmt.varname, buf);
                interpret(root->loop_stmt.body);
            }
        }
    }
    else if (root->type == NODE_WHILE)
    {
        while (eval_expression(root->while_stmt.condition) != 0)
        {
            interpret(root->while_stmt.body);
        }
    }
    else if (root->type == NODE_TEMPORAL_LOOP)
    {
        TemporalVariable *temp_var = get_temporal_var_struct(root->temporal_loop.temporal_var);
        if (!temp_var)
        {
            printf("Runtime error: Variable '%s' is not a temporal variable\n", root->temporal_loop.temporal_var);
            exit(1);
        }
        
        // Iterate through the temporal history
        for (int i = 0; i < temp_var->count; i++)
        {
            // Set the loop variable to current history entry
            set_variable(root->temporal_loop.varname, temp_var->history[i].value);
            interpret(root->temporal_loop.body);
        }
    }
    else if (root->type == NODE_SWITCH)
    {
        double switch_value = eval_expression(root->switch_stmt.expression);
        bool case_matched = false;

        // Check each case
        for (int i = 0; i < root->switch_stmt.case_count; i++)
        {
            ASTNode *case_node = root->switch_stmt.cases[i];
            double case_value = eval_expression(case_node->case_stmt.value);

            if (switch_value == case_value)
            {
                interpret(case_node->case_stmt.body);
                case_matched = true;
                break; // Exit after first match
            }
        }

        // If no case matched, try the default case
        if (!case_matched && root->switch_stmt.default_case)
        {
            interpret(root->switch_stmt.default_case);
        }
    }
    else if (root->type == NODE_IMPORT)
    {
        char *source = read_file(root->string);
        if (!source)
        {
            printf("Import error: Could not open file '%s'\n", root->string);
            exit(1);
        }
        parser_init(source);
        ASTNode *import_root = parse_program();
        interpret(import_root);
        free(source);
    }
    else if (root->type == NODE_FUNC_DEF)
    {
        register_function(root->func_def.name, root->func_def.params, root->func_def.param_count, root->func_def.body);
    }
    else if (root->type == NODE_FUNC_CALL)
    {
        Function *fn = find_function(root->func_call.name);
        if (!fn)
        {
            printf("Runtime error: Undefined function '%s'\n", root->func_call.name);
            exit(1);
        }
        if (fn->param_count != root->func_call.arg_count)
        {
            printf("Runtime error: Function '%s' expects %d args but got %d\n",
                   root->func_call.name, fn->param_count, root->func_call.arg_count);
            exit(1);
        }

        for (int i = 0; i < fn->param_count; i++)
        {
            ASTNode *arg = root->func_call.args[i];
            if (arg->type == NODE_STRING)
            {
                set_variable(fn->params[i], arg->string);
            }
            else
            {
                double val = eval_expression(arg);
                char buf[64];
                snprintf(buf, sizeof(buf), "%g", val);
                set_variable(fn->params[i], buf);
            }
        }

        interpret(fn->body);
    }
    else if (root->type == NODE_CLASS_DEF)
    {
        // Register the class
        register_class(root->class_def.class_name, root);
    }
    else if (root->type == NODE_CLASS_INSTANCE)
    {
        // On assignment: create a new object instance and initialize fields
        ObjectInstance *obj = object_new(root->class_instance.class_name);
        // Initialize fields from class definition
        ASTNode *class_node = get_class(root->class_instance.class_name);
        if (class_node)
        {
            ASTNode *body = class_node->class_def.body;
            for (int i = 0; i < body->block.count; i++)
            {
                ASTNode *stmt = body->block.statements[i];
                if (stmt->type == NODE_ASSIGN)
                {
                    if (stmt->assign.value->type == NODE_STRING)
                    {
                        object_set_field(obj, stmt->assign.varname, 0, stmt->assign.value->string, FIELD_STRING);
                    }
                    else
                    {
                        double val = eval_expression(stmt->assign.value);
                        object_set_field(obj, stmt->assign.varname, val, NULL, FIELD_NUMBER);
                    }
                }
            }
        }
        // Store the pointer as a string in the variable table
        char buf[32];
        snprintf(buf, sizeof(buf), "%p", (void *)obj);
        set_variable("__last_object_ptr", buf); // Used for assignment
    }
    else if (root->type == NODE_MEMBER_ACCESS)
    {
        // Evaluate the object (left side)
        ASTNode *object = root->member_access.object;
        const char *member_name = root->member_access.member_name;
        if (object->type == NODE_VAR && strcmp(object->varname, "self") == 0 && current_self)
        {
            // Accessing self.field
            object_get_field(current_self, member_name);
        }
        else if (object->type == NODE_VAR)
        {
            // Try to get the object instance from a variable table (not implemented in this stub)
            fprintf(stderr, "Error: Only 'self' member access is supported in this stub.\n");
            exit(1);
        }
        else
        {
            fprintf(stderr, "Error: Unsupported object type for member access.\n");
            exit(1);
        }
    }
    else if (root->type == NODE_METHOD_CALL)
    {
        // Evaluate a method call: object.method(args)
        ASTNode *object_node = root->method_call.object;
        ObjectInstance *obj = NULL;
        if (object_node->type == NODE_VAR)
        {
            const char *obj_ptr_str = get_variable(object_node->varname);
            sscanf(obj_ptr_str, "%p", (void **)&obj);
        }
        if (!obj)
        {
            printf("Runtime error: Method call on non-object\n");
            exit(1);
        }
        const char *class_name = obj->class_name;
        ASTNode *class_node = get_class(class_name);
        if (!class_node)
        {
            printf("Runtime error: Class '%s' not found\n", class_name);
            exit(1);
        }
        // Search for the method in the class body
        ASTNode *body = class_node->class_def.body;
        ASTNode *method_def = NULL;
        for (int i = 0; i < body->block.count; i++)
        {
            ASTNode *stmt = body->block.statements[i];
            if (stmt->type == NODE_METHOD_DEF && strcmp(stmt->method_def.method_name, root->method_call.method_name) == 0)
            {
                method_def = stmt;
                break;
            }
        }
        if (!method_def)
        {
            printf("Runtime error: Method '%s' not found in class '%s'\n", root->method_call.method_name, class_name);
            exit(1);
        }
        // Set current self
        current_self = obj;
        // Bind self and arguments
        set_variable("self", "__self__"); // Dummy, real access is via current_self
        for (int i = 0; i < method_def->method_def.param_count && i < root->method_call.arg_count; i++)
        {
            ASTNode *arg = root->method_call.args[i];
            if (arg->type == NODE_STRING)
            {
                set_variable(method_def->method_def.params[i], arg->string);
            }
            else
            {
                double val = eval_expression(arg);
                char buf[64];
                snprintf(buf, sizeof(buf), "%g", val);
                set_variable(method_def->method_def.params[i], buf);
            }
        }
        interpret(method_def->method_def.body);
        current_self = NULL;
    }
    else if (root->type == NODE_MEMBER_ASSIGN)
    {
        // Assignment to an object member: obj.field = value
        ASTNode *object_node = root->member_assign.object;
        const char *member_name = root->member_assign.member_name;
        ASTNode *value_node = root->member_assign.value;
        ObjectInstance *obj = NULL;
        if (object_node->type == NODE_VAR)
        {
            if (strcmp(object_node->varname, "self") == 0 && current_self)
            {
                obj = current_self;
            }
            else
            {
                const char *obj_ptr_str = get_variable(object_node->varname);
                if (!obj_ptr_str)
                {
                    printf("Runtime error: Undefined object variable '%s'\n", object_node->varname);
                    exit(1);
                }
                sscanf(obj_ptr_str, "%p", (void **)&obj);
            }
        }
        if (!obj)
        {
            printf("Runtime error: Member assignment on non-object\n");
            exit(1);
        }
        if (value_node->type == NODE_STRING)
        {
            object_set_field(obj, member_name, 0, value_node->string, FIELD_STRING);
        }
        else if (value_node->type == NODE_VAR)
        {
            const char *str_val = get_variable(value_node->varname);
            if (str_val)
            {
                // Try to parse as number first
                char *endptr;
                double num_val = strtod(str_val, &endptr);
                if (endptr != str_val && *endptr == '\0')
                {
                    // It's a valid number
                    object_set_field(obj, member_name, num_val, NULL, FIELD_NUMBER);
                }
                else
                {
                    // It's a string
                    object_set_field(obj, member_name, 0, str_val, FIELD_STRING);
                }
            }
        }
        else
        {
            double val = eval_expression(value_node);
            object_set_field(obj, member_name, val, NULL, FIELD_NUMBER);
        }
    }
    else if (root->type == NODE_LIST)
    {
        // Do nothing here, as lists are handled in print_node
    }
    else if (root->type == NODE_DICT)
    {
        // Do nothing here, as dicts are handled in print_node
    }
    else if (root->type == NODE_LIST_APPEND || root->type == NODE_LIST_PREPEND ||
             root->type == NODE_LIST_POP || root->type == NODE_LIST_INSERT ||
             root->type == NODE_LIST_REMOVE)
    {
        eval_expression(root);
    }
    else if (root->type == NODE_DICT_GET || root->type == NODE_DICT_SET ||
             root->type == NODE_DICT_KEYS || root->type == NODE_DICT_VALUES)
    {
        eval_expression(root);
    }
    else if (root->type == NODE_STACK_PUSH || root->type == NODE_STACK_POP ||
             root->type == NODE_STACK_PEEK || root->type == NODE_STACK_SIZE ||
             root->type == NODE_STACK_EMPTY)
    {
        eval_expression(root);
    }
    else if (root->type == NODE_QUEUE_ENQUEUE || root->type == NODE_QUEUE_DEQUEUE ||
             root->type == NODE_QUEUE_FRONT || root->type == NODE_QUEUE_BACK ||
             root->type == NODE_QUEUE_ISEMPTY || root->type == NODE_QUEUE_SIZE ||
             root->type == NODE_LINKED_LIST_ADD || root->type == NODE_LINKED_LIST_REMOVE ||
             root->type == NODE_LINKED_LIST_GET || root->type == NODE_LINKED_LIST_SIZE ||
             root->type == NODE_LINKED_LIST_ISEMPTY || root->type == NODE_REGEX ||
             root->type == NODE_REGEX_MATCH || root->type == NODE_REGEX_REPLACE ||
             root->type == NODE_REGEX_FIND_ALL)
    {
        // For linked list remove operations, don't print the result
        if (root->type == NODE_LINKED_LIST_REMOVE)
        {
            eval_expression(root); // Just execute without printing
        }
        else
        {
            eval_expression(root);
        }
    }
    else if (root->type == NODE_PATTERN_MATCH)
    {
        eval_expression(root); // Call eval_expression without expecting a return value
    }
    else if (root->type == NODE_BINOP || root->type == NODE_VAR || root->type == NODE_NUMBER || root->type == NODE_STRING)
    {
        // Evaluate top-level expressions (no side effects, but avoids error)
        eval_expression(root);
    }
    else if (root->type == NODE_FILE_OPEN || root->type == NODE_FILE_READ ||
             root->type == NODE_FILE_WRITE || root->type == NODE_FILE_CLOSE ||
             root->type == NODE_TO_STR || root->type == NODE_TO_INT ||
             root->type == NODE_HTTP_GET || root->type == NODE_HTTP_POST ||
             root->type == NODE_HTTP_PUT || root->type == NODE_HTTP_DELETE ||
             root->type == NODE_TEMPORAL_AGGREGATE || root->type == NODE_TEMPORAL_PATTERN)
    {
        eval_expression(root);
    }
    else
    {
        // Print the node type for easier debugging
        printf("Error: Unknown AST node type %d in interpret()\n", root->type);
        exit(1);
    }
}

static double eval_expression(ASTNode *node)
{
    if (!node)
    {
        fprintf(stderr, "Runtime error: Null AST node in eval\n");
        exit(1);
    }

    switch (node->type)
    {
    case NODE_NUMBER:
        return node->number; // Directly access the number field

    case NODE_STRING:
    {
        char *endptr;
        double val = strtod(node->string, &endptr);
        if (endptr == node->string)
            return 0.0;
        return val;
    }
    case NODE_VAR:
    {
        if (strcmp(node->varname, "self") == 0 && current_self)
        {
            // Return dummy value for self
            return 0;
        }
        
        // Check if it's a temporal variable first
        TemporalVariable *temp_var = get_temporal_var_struct(node->varname);
        if (temp_var)
        {
            // Get current value (time_offset = 0)
            const char *val = get_temporal_variable(node->varname, 0);
            if (!val)
            {
                printf("Runtime error: Cannot access temporal variable '%s'\n", node->varname);
                exit(1);
            }
            char *endptr;
            double dval = strtod(val, &endptr);
            if (endptr == val)
                return 0.0;
            return dval;
        }
        
        const char *val = get_variable(node->varname);
        if (!val)
        {
            printf("Runtime error: Undefined variable '%s'\n", node->varname);
            exit(1);
        }
        char *endptr;
        double dval = strtod(val, &endptr);
        if (endptr == val)
            return 0.0;
        return dval;
    }
    case NODE_INPUT:
    {
        interpret(node);
        const char *val = get_variable("__last_input");
        if (val)
        {
            return atof(val);
        }
        return 0;
    }
    case NODE_BINOP:
    {
        double left = eval_expression(node->binop.left);
        double right = eval_expression(node->binop.right);
        switch (node->binop.op)
        {
        case TOK_PLUS:
            return left + right;
        case TOK_MINUS:
            return left - right;
        case TOK_MUL:
            return left * right;
        case TOK_DIV:
            if (right == 0)
            {
                fprintf(stderr, "Runtime error: Division by zero\n");
                exit(1);
            }
            return left / right;
        case TOK_MOD:
            return fmod(left, right);
        case TOK_EQ:
            return left == right;
        case TOK_NEQ:
            return left != right;
        case TOK_LT:
            return left < right;
        case TOK_GT:
            return left > right;
        case TOK_LTE:
            return left <= right;
        case TOK_GTE:
            return left >= right;
        default:
            fprintf(stderr, "Runtime error: Unknown binary operator %d\n", node->binop.op);
            exit(1);
        }
    }

    case NODE_BITWISE_NOT:
    {
        double operand = eval_expression(node->unop.operand);
        return (double)(~(int)operand);
    }
    case NODE_LIST:
    {
        print_node(node);
        return 0;
    }
    case NODE_LIST_ACCESS:
    {
        double index = eval_expression(node->list_access.index);
        int i = (int)index;
        ASTNode *list_node = node->list_access.list;

        if (list_node->type == NODE_VAR)
        {
            ASTNode *list = get_list_variable(list_node->varname);
            if (!list)
            {
                printf("Runtime error: Undefined list variable '%s'\n", list_node->varname);
                exit(1);
            }
            list_node = list;
        }

        if (list_node->type != NODE_LIST)
        {
            printf("Runtime error: List access only supported on list nodes\n");
            exit(1);
        }

        if (i < 0 || i >= list_node->list.count)
        {
            printf("Runtime error: List index out of bounds\n");
            exit(1);
        }

        ASTNode *element = list_node->list.elements[i];
        if (element->type == NODE_NUMBER)
        {
            return element->number;
        }
        else if (element->type == NODE_STRING)
        {
            printf("%s\n", element->string);
            return 0; // Return 0 for string access
        }
        else
        {
            printf("Runtime error: Unsupported list element type\n");
            exit(1);
        }
    }

    case NODE_LIST_LEN:
    {
        ASTNode *list_node = node->list_access.list;
        if (list_node->type == NODE_VAR)
        {
            list_node = get_list_variable(list_node->varname);
            if (!list_node)
            {
                printf("Runtime error: Undefined list variable\n");
                exit(1);
            }
        }
        if (list_node->type != NODE_LIST)
        {
            printf("Runtime error: len() expects a list\n");
            exit(1);
        }
        return list_node->list.count;
    }

    case NODE_LIST_APPEND:
    {
        ASTNode *list_node = node->binop.left;
        ASTNode *value_node = node->binop.right;

        if (list_node->type == NODE_VAR)
        {
            ASTNode *list = get_list_variable(list_node->varname);
            if (!list)
            {
                printf("Runtime error: Undefined list variable\n");
                exit(1);
            }
            list_node = list;
        }

        if (list_node->type != NODE_LIST)
        {
            printf("Runtime error: append() expects a list\n");
            exit(1);
        }

        ast_list_add_element(list_node, value_node);
        return 0; // Return success
    }

    case NODE_LIST_PREPEND:
    {
        ASTNode *list_node = node->binop.left;
        ASTNode *value_node = node->binop.right;

        if (list_node->type == NODE_VAR)
        {
            ASTNode *list = get_list_variable(list_node->varname);
            if (!list)
            {
                printf("Runtime error: Undefined list variable\n");
                exit(1);
            }
            list_node = list;
        }

        if (list_node->type != NODE_LIST)
        {
            printf("Runtime error: prepend() expects a list\n");
            exit(1);
        }

        // Shift all elements to make room at the beginning
        list_node->list.elements = realloc(list_node->list.elements,
                                           sizeof(ASTNode *) * (list_node->list.count + 1));
        for (int i = list_node->list.count; i > 0; i--)
        {
            list_node->list.elements[i] = list_node->list.elements[i - 1];
        }
        list_node->list.elements[0] = value_node;
        list_node->list.count++;
        return 0; // Return success
    }

    case NODE_LIST_POP:
    {
        ASTNode *list_node = node->list_access.list;
        if (list_node->type == NODE_VAR)
        {
            ASTNode *list = get_list_variable(list_node->varname);
            if (!list)
            {
                printf("Runtime error: Undefined list variable\n");
                exit(1);
            }
            list_node = list;
        }

        if (list_node->type != NODE_LIST || list_node->list.count == 0)
        {
            printf("Runtime error: pop() expects a non-empty list\n");
            exit(1);
        }

        double val = eval_expression(list_node->list.elements[list_node->list.count - 1]);
        list_node->list.count--;
        return val;
    }

    case NODE_LIST_INSERT:
    {
        ASTNode *list_node = node->list_access.list;
        ASTNode *index_node = node->list_access.index;
        ASTNode *value_node = node; // Use the current node as the value node

        if (list_node->type != NODE_LIST)
        {
            fprintf(stderr, "runtime error: insert() expects a list\n");
            exit(1);
        }

        int index = (int)eval_expression(index_node);
        if (index < 0 || index > list_node->list.count)
        {
            fprintf(stderr, "runtime error: Index out of bounds in insert()\n");
            exit(1);
        }

        ASTNode *new_value = ast_new_number(eval_expression(value_node));

        // Create a new array with one extra element
        ASTNode **new_elements = malloc(sizeof(ASTNode *) * (list_node->list.count + 1));

        // Copy elements before the insertion point
        for (int i = 0; i < index; i++)
        {
            new_elements[i] = list_node->list.elements[i];
        }

        // Insert the new value
        new_elements[index] = new_value;

        // Copy elements after the insertion point
        for (int i = index; i < list_node->list.count; i++)
        {
            new_elements[i + 1] = list_node->list.elements[i];
        }

        // Free the old array and assign the new one
        free(list_node->list.elements);
        list_node->list.elements = new_elements;
        list_node->list.count++;

        return 0;
    }

    case NODE_LIST_REMOVE:
    {
        ASTNode *list_node = node->binop.left;
        ASTNode *value_node = node->binop.right;

        if (list_node->type == NODE_VAR)
        {
            ASTNode *list = get_list_variable(list_node->varname);
            if (!list)
            {
                printf("Runtime error: Undefined list variable\n");
                exit(1);
            }
            list_node = list;
        }

        if (list_node->type != NODE_LIST)
        {
            printf("Runtime error: remove() expects a list\n");
            exit(1);
        }

        double value = eval_expression(value_node);
        int found = 0;

        for (int i = 0; i < list_node->list.count; i++)
        {
            double element = eval_expression(list_node->list.elements[i]);
            if (element == value)
            {
                found = 1;
                // Shift elements to fill the gap
                for (int j = i; j < list_node->list.count - 1; j++)
                {
                    list_node->list.elements[j] = list_node->list.elements[j + 1];
                }
                list_node->list.count--;
                break;
            }
        }

        if (!found)
        {
            printf("Runtime error: Value not found in list\n");
            exit(1);
        }
        return 0; // Return success
    }

    case NODE_AND:
    {
        bool left = eval_expression(node->binop.left) != 0;
        bool right = eval_expression(node->binop.right) != 0;
        return left && right;
    }

    case NODE_OR:
    {
        bool left = eval_expression(node->binop.left) != 0;
        bool right = eval_expression(node->binop.right) != 0;
        return left || right;
    }

    case NODE_NOT:
    {
        bool operand = eval_expression(node->unop.operand) != 0;
        return !operand;
    }

    case NODE_BITWISE_AND:
    {
        int lhs = (int)eval_expression(node->binop.left);
        int rhs = (int)eval_expression(node->binop.right);
        return (double)(lhs & rhs);
    }

    case NODE_BITWISE_OR:
    {
        int lhs = (int)eval_expression(node->binop.left);
        int rhs = (int)eval_expression(node->binop.right);
        return (double)(lhs | rhs);
    }

    case NODE_BITWISE_XOR:
    {
        int lhs = (int)eval_expression(node->binop.left);
        int rhs = (int)eval_expression(node->binop.right);
        return (double)(lhs ^ rhs);
    }

    case NODE_PRINT:
    {
        double value = eval_expression(node->binop.left);
        if (value == 1.0 || value == 0.0)
        {
            printf("%s\n", bool_to_str((bool)value));
        }
        else
        {
            printf("%g\n", value);
        }
        return value;
    }

    case NODE_PATTERN_MATCH:
    {
        // Get the pattern and noise strings
        const char *pattern_str = NULL;
        const char *noise_str = NULL;

        if (node->pattern_match.pattern->type == NODE_STRING)
        {
            pattern_str = node->pattern_match.pattern->string;
        }
        else if (node->pattern_match.pattern->type == NODE_VAR)
        {
            pattern_str = get_variable(node->pattern_match.pattern->varname);
        }

        if (node->pattern_match.noise->type == NODE_STRING)
        {
            noise_str = node->pattern_match.noise->string;
        }
        else if (node->pattern_match.noise->type == NODE_VAR)
        {
            noise_str = get_variable(node->pattern_match.noise->varname);
        }

        if (!pattern_str || !noise_str)
        {
            printf("Runtime error: pattern_match expects string arguments\n");
            exit(1);
        }

        int pattern_len = strlen(pattern_str);
        int noise_len = strlen(noise_str);

        if (pattern_len == 0 || noise_len == 0)
        {
            return 0; // No matches if either string is empty
        }

        // Create a list to store the match positions
        ASTNode *result_list = ast_new_list();

        // Naive pattern matching algorithm
        for (int i = 0; i <= noise_len - pattern_len; i++)
        {
            int match = 1;
            for (int j = 0; j < pattern_len; j++)
            {
                if (noise_str[i + j] != pattern_str[j])
                {
                    match = 0;
                    break;
                }
            }
            if (match)
            {
                // Add the position to the result list
                ast_list_add_element(result_list, ast_new_number(i));
            }
        }

        // Store the result in a special variable or print it directly
        char *list_str = list_to_string(result_list);
        printf("%s\n", list_str); // Print the result
        free(list_str);
        ast_free(result_list);
        return 0;
    }

    case NODE_FORMAT_STRING:
    {
        char buffer[1024];
        char *dest = buffer;
        const char *src = node->format_str.format;
        int arg_index = 0;

        while (*src && (dest - buffer) < sizeof(buffer) - 1)
        {
            if (*src == '@' && *(src + 1) != '\0' && *(src + 1) != '@' && 
                (*(src + 1) == 's' || *(src + 1) == 'd' || *(src + 1) == 'f'))
            {
                src++; // skip '@'
                if (arg_index >= node->format_str.arg_count)
                {
                    printf("Runtime error: Not enough arguments for format string\n");
                    exit(1);
                }

                ASTNode *arg = node->format_str.args[arg_index++];
                double val = 0;
                if (*src != 's' || arg->type != NODE_DICT_GET)
                    val = eval_expression(arg);

                switch (*src)
                {
                case 'd': // integer
                    dest += sprintf(dest, "%d", (int)val);
                    break;
                case 'f': // float
                    dest += sprintf(dest, "%g", val);
                    break;
                case 's': // string (from variable)
                    if (arg->type == NODE_TO_STR)
                    {
                        double result = eval_expression(arg);
                        if (result == -12345.6789)
                        { // Special marker for to_str
                            const char *str_result = get_variable("__to_str_result");
                            if (str_result)
                            {
                                dest += sprintf(dest, "%s", str_result);
                            }
                        }
                        else
                        {
                            dest += sprintf(dest, "%g", result);
                        }
                    }
                    else if (arg->type == NODE_STRING)
                    {
                        dest += sprintf(dest, "%s", arg->string);
                    }
                    else if (arg->type == NODE_VAR)
                    {
                        const char *str = get_variable(arg->varname);
                        if (str)
                        {
                            dest += sprintf(dest, "%s", str);
                        }
                        else
                        {
                            printf("Runtime error: Undefined string variable\n");
                            exit(1);
                        }
                    }
                    else if (arg->type == NODE_DICT_GET)
                    {
                        ASTNode *dict_node = arg->dict_get.dict;
                        if (dict_node->type == NODE_VAR)
                        {
                            dict_node = get_dict_variable(dict_node->varname);
                        }
                        if (dict_node && dict_node->type == NODE_DICT)
                        {
                            ASTNode *key = arg->dict_get.key;
                            int found = 0;
                            for (int i = 0; i < dict_node->dict.count; i++)
                            {
                                if (dict_node->dict.keys[i]->type == NODE_STRING && key->type == NODE_STRING)
                                {
                                    if (strcmp(dict_node->dict.keys[i]->string, key->string) == 0)
                                    {
                                        if (dict_node->dict.values[i]->type == NODE_STRING)
                                            dest += sprintf(dest, "%s", dict_node->dict.values[i]->string);
                                        else if (dict_node->dict.values[i]->type == NODE_NUMBER)
                                            dest += sprintf(dest, "%g", dict_node->dict.values[i]->number);
                                        found = 1;
                                        break;
                                    }
                                }
                            }
                            if (!found)
                            {
                                dest += sprintf(dest, "(not found)");
                            }
                        }
                    }
                    else if (arg->type == NODE_MEMBER_ACCESS)
                    {
                        ASTNode *object = arg->member_access.object;
                        const char *member_name = arg->member_access.member_name;
                        ObjectInstance *obj = NULL;

                        if (object->type == NODE_VAR && strcmp(object->varname, "self") == 0 && current_self)
                        {
                            obj = current_self;
                        }
                        else if (object->type == NODE_VAR)
                        {
                            const char *obj_ptr_str = get_variable(object->varname);
                            if (obj_ptr_str)
                            {
                                sscanf(obj_ptr_str, "%p", (void **)&obj);
                            }
                        }

                        if (obj)
                        {
                            FieldEntry *field = object_get_field(obj, member_name);
                            if (field && field->type == FIELD_STRING)
                            {
                                dest += sprintf(dest, "%s", field->string_value);
                            }
                            else if (field && field->type == FIELD_NUMBER)
                            {
                                dest += sprintf(dest, "%g", field->number_value);
                            }
                            else
                            {
                                dest += sprintf(dest, "(unknown)");
                            }
                        }
                        else
                        {
                            dest += sprintf(dest, "(null object)");
                        }
                    }
                    else if (arg->type == NODE_STACK_POP || arg->type == NODE_STACK_PEEK)
                    {
                        ASTNode *stack_node = arg->stack_op.stack;
                        if (stack_node->type == NODE_VAR)
                        {
                            stack_node = get_stack_variable(stack_node->varname);
                        }
                        if (stack_node && stack_node->type == NODE_STACK && stack_node->stack.count > 0)
                        {
                            ASTNode *top_element = stack_node->stack.elements[stack_node->stack.count - 1];
                            if (arg->type == NODE_STACK_POP)
                                stack_node->stack.count--;
                            if (top_element->type == NODE_STRING)
                            {
                                dest += sprintf(dest, "%s", top_element->string);
                            }
                            else if (top_element->type == NODE_NUMBER)
                            {
                                dest += sprintf(dest, "%g", top_element->number);
                            }
                        }
                    }
                    else if (arg->type == NODE_QUEUE_DEQUEUE || arg->type == NODE_QUEUE_FRONT)
                    {
                        ASTNode *queue_node = arg->queue_op.queue;
                        if (queue_node->type == NODE_VAR)
                        {
                            queue_node = get_queue_variable(queue_node->varname);
                        }
                        if (queue_node && queue_node->type == NODE_QUEUE && queue_node->queue.count > 0)
                        {
                            ASTNode *front_element = queue_node->queue.elements[0];
                            if (arg->type == NODE_QUEUE_DEQUEUE)
                            {
                                for (int i = 0; i < queue_node->queue.count - 1; i++)
                                {
                                    queue_node->queue.elements[i] = queue_node->queue.elements[i + 1];
                                }
                                queue_node->queue.count--;
                            }
                            if (front_element->type == NODE_STRING)
                            {
                                dest += sprintf(dest, "%s", front_element->string);
                            }
                            else if (front_element->type == NODE_NUMBER)
                            {
                                dest += sprintf(dest, "%g", front_element->number);
                            }
                        }
                    }
                    else
                    {
                        // For numbers, convert to string
                        char num_str[64];
                        snprintf(num_str, sizeof(num_str), "%g", val);
                        dest += sprintf(dest, "%s", num_str);
                    }
                    break;
                case '@': // literal '@'
                    *dest++ = '@';
                    break;
                default:
                    printf("Runtime error: Unknown format specifier @%c\n", *src);
                    exit(1);
                }
                src++;
            }
            else if (*src == '@' && *(src + 1) == '@')
            {
                // Handle @@ as literal @
                *dest++ = '@';
                src += 2;
            }
            else
            {
                *dest++ = *src++;
            }
        }
        *dest = '\0';
        printf("%s\n", buffer);
        return 0;
    }

    case NODE_CLASS_DEF:
        // Register the class (if not already done in interpret)
        register_class(node->class_def.class_name, node);
        return 0;
    case NODE_CLASS_INSTANCE:
        // Create a new object instance (if used as an expression, return 0 for now)
        // In interpret(), the actual instance is created and stored
        return 0;
    case NODE_METHOD_DEF:
        // Method definitions are handled during class registration, nothing to do at runtime
        return 0;
    case NODE_METHOD_CALL:
        // Evaluate a method call: object.method(args)
        // This is handled in interpret(), but if called here, do nothing
        interpret(node);
        return 0;

    case NODE_DICT:
        print_node(node);
        return 0;
    case NODE_DICT_GET:
    {
        ASTNode *dict_node = node->dict_get.dict;
        if (dict_node->type == NODE_VAR)
        {
            dict_node = get_dict_variable(dict_node->varname);
            if (!dict_node)
            {
                printf("Runtime error: Undefined dict variable\n");
                exit(1);
            }
        }
        if (dict_node->type != NODE_DICT)
        {
            printf("Runtime error: get() expects a dictionary\n");
            exit(1);
        }

        ASTNode *key = node->dict_get.key;
        for (int i = 0; i < dict_node->dict.count; i++)
        {
            if (dict_node->dict.keys[i]->type == NODE_STRING && key->type == NODE_STRING)
            {
                if (strcmp(dict_node->dict.keys[i]->string, key->string) == 0)
                {
                    if (dict_node->dict.values[i]->type == NODE_NUMBER)
                        return dict_node->dict.values[i]->number;
                    else if (dict_node->dict.values[i]->type == NODE_STRING)
                        return 0;
                    return 0;
                }
            }
        }
        printf("Runtime error: Key not found in dictionary\n");
        exit(1);
    }
    case NODE_DICT_SET:
    {
        ASTNode *dict_node = node->dict_set.dict;
        if (dict_node->type == NODE_VAR)
        {
            dict_node = get_dict_variable(dict_node->varname);
            if (!dict_node)
            {
                printf("Runtime error: Undefined dict variable\n");
                exit(1);
            }
        }
        if (dict_node->type != NODE_DICT)
        {
            printf("Runtime error: set() expects a dictionary\n");
            exit(1);
        }

        ASTNode *key = node->dict_set.key;
        ASTNode *value = node->dict_set.value;

        // Find existing key or add new one
        for (int i = 0; i < dict_node->dict.count; i++)
        {
            if (dict_node->dict.keys[i]->type == NODE_STRING && key->type == NODE_STRING)
            {
                if (strcmp(dict_node->dict.keys[i]->string, key->string) == 0)
                {
                    dict_node->dict.values[i] = value;
                    return 0;
                }
            }
        }
        ast_dict_add_pair(dict_node, key, value);
        return 0;
    }
    case NODE_DICT_KEYS:
    {
        ASTNode *dict_node = node->dict_get.dict;
        if (dict_node->type == NODE_VAR)
        {
            dict_node = get_dict_variable(dict_node->varname);
            if (!dict_node)
            {
                printf("Runtime error: Undefined dict variable\n");
                exit(1);
            }
        }
        if (dict_node->type != NODE_DICT)
        {
            printf("Runtime error: keys() expects a dictionary\n");
            exit(1);
        }
        printf("[");
        for (int i = 0; i < dict_node->dict.count; i++)
        {
            if (dict_node->dict.keys[i]->type == NODE_STRING)
                printf("%s", dict_node->dict.keys[i]->string);
            else if (dict_node->dict.keys[i]->type == NODE_NUMBER)
                printf("%g", dict_node->dict.keys[i]->number);
            if (i < dict_node->dict.count - 1)
                printf(", ");
        }
        printf("]\n");
        return 0;
    }
    case NODE_DICT_VALUES:
    {
        ASTNode *dict_node = node->dict_get.dict;
        if (dict_node->type == NODE_VAR)
        {
            dict_node = get_dict_variable(dict_node->varname);
            if (!dict_node)
            {
                printf("Runtime error: Undefined dict variable\n");
                exit(1);
            }
        }
        if (dict_node->type != NODE_DICT)
        {
            printf("Runtime error: values() expects a dictionary\n");
            exit(1);
        }
        printf("[");
        for (int i = 0; i < dict_node->dict.count; i++)
        {
            if (dict_node->dict.values[i]->type == NODE_STRING)
                printf("%s", dict_node->dict.values[i]->string);
            else if (dict_node->dict.values[i]->type == NODE_NUMBER)
                printf("%g", dict_node->dict.values[i]->number);
            if (i < dict_node->dict.count - 1)
                printf(", ");
        }
        printf("]\n");
        return 0;
    }
    case NODE_MEMBER_ACCESS:
    {
        ASTNode *object = node->member_access.object;
        const char *member_name = node->member_access.member_name;
        ObjectInstance *obj = NULL;

        if (object->type == NODE_VAR && strcmp(object->varname, "self") == 0 && current_self)
        {
            obj = current_self;
        }
        else if (object->type == NODE_VAR)
        {
            const char *obj_ptr_str = get_variable(object->varname);
            if (obj_ptr_str)
            {
                sscanf(obj_ptr_str, "%p", (void **)&obj);
            }
        }

        if (!obj)
        {
            printf("Runtime error: Member access on non-object\n");
            exit(1);
        }

        FieldEntry *field = object_get_field(obj, member_name);
        if (!field)
        {
            printf("Runtime error: Field '%s' not found\n", member_name);
            exit(1);
        }

        if (field->type == FIELD_NUMBER)
        {
            return field->number_value;
        }
        else if (field->type == FIELD_STRING)
        {
            // For string fields, we can't return the string directly in eval_expression
            // since it returns double. Return 0 to indicate successful access.
            return 0;
        }
        return 0;
    }
    case NODE_STACK:
        print_node(node);
        return 0;
    case NODE_STACK_PUSH:
    {
        ASTNode *stack_node = node->stack_push.stack;
        ASTNode *value_node = node->stack_push.value;

        if (stack_node->type == NODE_VAR)
        {
            ASTNode *stack = get_stack_variable(stack_node->varname);
            if (!stack)
            {
                printf("Runtime error: Undefined stack variable\n");
                exit(1);
            }
            stack_node = stack;
        }

        if (stack_node->type != NODE_STACK)
        {
            printf("Runtime error: push() expects a stack\n");
            exit(1);
        }

        ast_stack_add_element(stack_node, value_node);
        return 0;
    }
    case NODE_STACK_POP:
    {
        ASTNode *stack_node = node->stack_op.stack;

        if (stack_node->type == NODE_VAR)
        {
            ASTNode *stack = get_stack_variable(stack_node->varname);
            if (!stack)
            {
                printf("Runtime error: Undefined stack variable\n");
                exit(1);
            }
            stack_node = stack;
        }

        if (stack_node->type != NODE_STACK || stack_node->stack.count == 0)
        {
            printf("Runtime error: pop() expects a non-empty stack\n");
            exit(1);
        }

        ASTNode *top_element = stack_node->stack.elements[stack_node->stack.count - 1];
        stack_node->stack.count--;
        if (top_element->type == NODE_NUMBER)
        {
            return top_element->number;
        }
        return eval_expression(top_element);
    }
    case NODE_STACK_PEEK:
    {
        ASTNode *stack_node = node->stack_op.stack;

        if (stack_node->type == NODE_VAR)
        {
            ASTNode *stack = get_stack_variable(stack_node->varname);
            if (!stack)
            {
                printf("Runtime error: Undefined stack variable\n");
                exit(1);
            }
            stack_node = stack;
        }

        if (stack_node->type != NODE_STACK || stack_node->stack.count == 0)
        {
            printf("Runtime error: peek() expects a non-empty stack\n");
            exit(1);
        }

        ASTNode *top_element = stack_node->stack.elements[stack_node->stack.count - 1];
        if (top_element->type == NODE_STRING)
        {
            printf("%s\n", top_element->string);
            return 0;
        }
        else if (top_element->type == NODE_NUMBER)
        {
            printf("%g\n", top_element->number);
            return top_element->number;
        }
        return eval_expression(top_element);
    }
    case NODE_STACK_SIZE:
    {
        ASTNode *stack_node = node->stack_op.stack;

        if (stack_node->type == NODE_VAR)
        {
            ASTNode *stack = get_stack_variable(stack_node->varname);
            if (!stack)
            {
                printf("Runtime error: Undefined stack variable\n");
                exit(1);
            }
            stack_node = stack;
        }

        if (stack_node->type != NODE_STACK)
        {
            printf("Runtime error: size() expects a stack\n");
            exit(1);
        }

        return stack_node->stack.count;
    }
    case NODE_STACK_EMPTY:
    {
        ASTNode *stack_node = node->stack_op.stack;

        if (stack_node->type == NODE_VAR)
        {
            ASTNode *stack = get_stack_variable(stack_node->varname);
            if (!stack)
            {
                printf("Runtime error: Undefined stack variable\n");
                exit(1);
            }
            stack_node = stack;
        }

        if (stack_node->type != NODE_STACK)
        {
            printf("Runtime error: empty() expects a stack\n");
            exit(1);
        }

        return stack_node->stack.count == 0 ? 1 : 0;
    }
    case NODE_QUEUE:
        print_node(node);
        return 0;
    case NODE_QUEUE_ENQUEUE:
    {
        ASTNode *queue_node = node->queue_enqueue.queue;
        ASTNode *value_node = node->queue_enqueue.value;
        if (queue_node->type == NODE_VAR)
        {
            queue_node = get_queue_variable(queue_node->varname);
        }
        if (queue_node && queue_node->type == NODE_QUEUE)
        {
            ast_queue_add_element(queue_node, value_node);
        }
        return 0;
    }
    case NODE_QUEUE_DEQUEUE:
    {
        ASTNode *queue_node = node->queue_op.queue;
        if (queue_node->type == NODE_VAR)
        {
            queue_node = get_queue_variable(queue_node->varname);
        }
        if (queue_node && queue_node->type == NODE_QUEUE && queue_node->queue.count > 0)
        {
            ASTNode *front = queue_node->queue.elements[0];
            for (int i = 0; i < queue_node->queue.count - 1; i++)
            {
                queue_node->queue.elements[i] = queue_node->queue.elements[i + 1];
            }
            queue_node->queue.count--;
            if (front->type == NODE_STRING)
            {
                printf("%s\n", front->string);
                return 0;
            }
            else if (front->type == NODE_NUMBER)
            {
                return front->number;
            }
            return eval_expression(front);
        }
        return 0;
    }
    case NODE_QUEUE_FRONT:
    {
        ASTNode *queue_node = node->queue_op.queue;
        if (queue_node->type == NODE_VAR)
        {
            queue_node = get_queue_variable(queue_node->varname);
        }
        if (queue_node && queue_node->type == NODE_QUEUE && queue_node->queue.count > 0)
        {
            ASTNode *front = queue_node->queue.elements[0];
            if (front->type == NODE_STRING)
            {
                printf("%s\n", front->string);
                return 0;
            }
            else if (front->type == NODE_NUMBER)
            {
                return front->number;
            }
            return eval_expression(front);
        }
        return 0;
    }
    case NODE_QUEUE_BACK:
    {
        ASTNode *queue_node = node->queue_op.queue;
        if (queue_node->type == NODE_VAR)
        {
            queue_node = get_queue_variable(queue_node->varname);
        }
        if (queue_node && queue_node->type == NODE_QUEUE && queue_node->queue.count > 0)
        {
            return eval_expression(queue_node->queue.elements[queue_node->queue.count - 1]);
        }
        return 0;
    }
    case NODE_QUEUE_ISEMPTY:
    {
        ASTNode *queue_node = node->queue_op.queue;
        if (queue_node->type == NODE_VAR)
        {
            queue_node = get_queue_variable(queue_node->varname);
        }
        if (queue_node && queue_node->type == NODE_QUEUE)
        {
            return queue_node->queue.count == 0 ? 1 : 0;
        }
        return 1;
    }
    case NODE_QUEUE_SIZE:
    {
        ASTNode *queue_node = node->queue_op.queue;
        if (queue_node->type == NODE_VAR)
        {
            queue_node = get_queue_variable(queue_node->varname);
        }
        if (queue_node && queue_node->type == NODE_QUEUE)
        {
            return queue_node->queue.count;
        }
        return 0;
    }
    case NODE_LINKED_LIST:
        print_node(node);
        return 0;
    case NODE_LINKED_LIST_ADD:
    {
        ASTNode *list_node = node->linked_list_op.list;
        ASTNode *value_node = node->linked_list_op.value;

        if (list_node->type == NODE_VAR)
        {
            list_node = get_linked_list_variable(list_node->varname);
            if (!list_node)
            {
                printf("Runtime error: Undefined linked list variable\n");
                exit(1);
            }
        }

        if (list_node->type != NODE_LINKED_LIST)
        {
            printf("Runtime error: ladd() expects a linked list\n");
            exit(1);
        }

        ast_linked_list_add_element(list_node, value_node);
        return 0;
    }
    case NODE_LINKED_LIST_REMOVE:
    {
        ASTNode *list_node = node->linked_list_op.list;
        ASTNode *value_node = node->linked_list_op.value;

        if (list_node->type == NODE_VAR)
        {
            list_node = get_linked_list_variable(list_node->varname);
            if (!list_node)
            {
                printf("Runtime error: Undefined linked list variable\n");
                exit(1);
            }
        }

        if (list_node->type != NODE_LINKED_LIST)
        {
            printf("Runtime error: lremove() expects a linked list\n");
            exit(1);
        }

        // Handle string values specially
        if (value_node->type == NODE_STRING)
        {
            const char *str_value = value_node->string;
            int found = 0;

            for (int i = 0; i < list_node->linked_list.count; i++)
            {
                if (list_node->linked_list.elements[i]->type == NODE_STRING &&
                    strcmp(list_node->linked_list.elements[i]->string, str_value) == 0)
                {
                    found = 1;
                    // Shift elements to fill the gap
                    for (int j = i; j < list_node->linked_list.count - 1; j++)
                    {
                        list_node->linked_list.elements[j] = list_node->linked_list.elements[j + 1];
                    }
                    list_node->linked_list.count--;
                    break;
                }
            }

            if (!found)
            {
                printf("Runtime error: String value not found in linked list\n");
                exit(1);
            }
        }
        else
        {
            double value = eval_expression(value_node);
            int found = 0;

            for (int i = 0; i < list_node->linked_list.count; i++)
            {
                double element = eval_expression(list_node->linked_list.elements[i]);
                if (element == value)
                {
                    found = 1;
                    // Shift elements to fill the gap
                    for (int j = i; j < list_node->linked_list.count - 1; j++)
                    {
                        list_node->linked_list.elements[j] = list_node->linked_list.elements[j + 1];
                    }
                    list_node->linked_list.count--;
                    break;
                }
            }

            if (!found)
            {
                printf("Runtime error: Value not found in linked list\n");
                exit(1);
            }
        }

        return 0; // Return 0 for consistency
    }
    case NODE_LINKED_LIST_GET:
    {
        ASTNode *list_node = node->linked_list_get.list;
        ASTNode *index_node = node->linked_list_get.index;

        if (list_node->type == NODE_VAR)
        {
            list_node = get_linked_list_variable(list_node->varname);
            if (!list_node)
            {
                printf("Runtime error: Undefined linked list variable\n");
                exit(1);
            }
        }

        if (list_node->type != NODE_LINKED_LIST)
        {
            printf("Runtime error: lget() expects a linked list\n");
            exit(1);
        }

        int index = (int)eval_expression(index_node);
        if (index < 0 || index >= list_node->linked_list.count)
        {
            printf("Runtime error: Linked list index out of bounds\n");
            exit(1);
        }

        ASTNode *element = list_node->linked_list.elements[index];
        if (element->type == NODE_NUMBER)
        {
            return element->number;
        }
        else if (element->type == NODE_STRING)
        {
            // For string elements, return a special value and let print_node handle it
            return -999;
        }
        else
        {
            return eval_expression(element);
        }
    }
    case NODE_LINKED_LIST_SIZE:
    {
        ASTNode *list_node = node->linked_list_op.list;

        if (list_node->type == NODE_VAR)
        {
            list_node = get_linked_list_variable(list_node->varname);
            if (!list_node)
            {
                printf("Runtime error: Undefined linked list variable\n");
                exit(1);
            }
        }

        if (list_node->type != NODE_LINKED_LIST)
        {
            printf("Runtime error: lsize() expects a linked list\n");
            exit(1);
        }

        return list_node->linked_list.count;
    }
    case NODE_LINKED_LIST_ISEMPTY:
    {
        ASTNode *list_node = node->linked_list_op.list;

        if (list_node->type == NODE_VAR)
        {
            list_node = get_linked_list_variable(list_node->varname);
            if (!list_node)
            {
                printf("Runtime error: Undefined linked list variable\n");
                exit(1);
            }
        }

        if (list_node->type != NODE_LINKED_LIST)
        {
            printf("Runtime error: lisEmpty() expects a linked list\n");
            exit(1);
        }

        return list_node->linked_list.count == 0 ? 1 : 0;
    }
    case NODE_FILE_OPEN:
    {
        char *filename = get_string_value(node->file_open_stmt.filename);
        char *mode = get_string_value(node->file_open_stmt.mode);
        FILE *f = fopen(filename, mode);
        free(filename);
        free(mode);
        if (!f)
        {
            return -1; // Return -1 on failure
        }
        return add_file_handle(f);
    }
    case NODE_FILE_READ:
    {
        int handle = (int)eval_expression(node->file_read_stmt.file_handle);
        FILE *f = get_file_handle(handle);
        if (!f)
        {
            printf("Runtime error: Invalid file handle\n");
            exit(1);
        }
        char buffer[1024];
        if (fgets(buffer, sizeof(buffer), f))
        {
            buffer[strcspn(buffer, "\n")] = 0;
            set_variable("__last_file_read", buffer);
            return 1; // Success
        }
        return 0; // EOF or error
    }
    case NODE_FILE_WRITE:
    {
        int handle = (int)eval_expression(node->file_write_stmt.file_handle);
        char *content = get_string_value(node->file_write_stmt.content);
        FILE *f = get_file_handle(handle);
        if (!f)
        {
            printf("Runtime error: Invalid file handle\n");
            exit(1);
        }
        int result = fputs(content, f);
        free(content);
        return result >= 0;
    }
    case NODE_FILE_CLOSE:
    {
        int handle = (int)eval_expression(node->file_close_stmt.file_handle);
        remove_file_handle(handle);
        return 0;
    }
    case NODE_TO_STR:
    {
        double value = eval_expression(node->unop.operand);
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%g", value);
        set_variable("__to_str_result", buffer);

        // For format strings, we need to return the string directly
        // We'll use a special return value that the format string handler can detect
        return -12345.6789; // Special marker for to_str
    }
    case NODE_TO_INT:
    {
        if (node->unop.operand->type == NODE_STRING)
        {
            // Direct string to int conversion
            char *endptr;
            double value = strtod(node->unop.operand->string, &endptr);
            return value;
        }
        else if (node->unop.operand->type == NODE_VAR)
        {
            // Variable containing a string to int conversion
            const char *str_value = get_variable(node->unop.operand->varname);
            if (str_value)
            {
                char *endptr;
                double value = strtod(str_value, &endptr);
                return value;
            }
            else
            {
                printf("Runtime error: Variable '%s' is undefined\n", node->unop.operand->varname);
                exit(1);
            }
        }
        else
        {
            // For other types, just evaluate and return
            return eval_expression(node->unop.operand);
        }
    }

    case NODE_HTTP_GET:
    {
        char *url = get_string_value(node->http_get.url);
        if (!url)
        {
            printf("Runtime error: Invalid URL for HTTP GET\n");
            exit(1);
        }

        char *response = perform_http_request(url, "GET", NULL, node->http_get.headers);
        free(url);

        if (response)
        {
            // Store the response in a variable and print it directly
            printf("%s\n", response);
            set_variable("__http_response", response);
            free(response);
            return 0; // Success
        }
        return 0; // Failure
    }

    case NODE_HTTP_POST:
    {
        char *url = get_string_value(node->http_post.url);
        char *data = get_string_value(node->http_post.data);
        if (!url || !data)
        {
            if (url)
                free(url);
            if (data)
                free(data);
            printf("Runtime error: Invalid URL or data for HTTP POST\n");
            exit(1);
        }

        char *response = perform_http_request(url, "POST", data, node->http_post.headers);
        free(url);
        free(data);

        if (response)
        {
            // Print the response directly
            printf("%s\n", response);
            set_variable("__http_response", response);
            free(response);
            return 0; // Success
        }
        return 0; // Failure
    }

    case NODE_HTTP_PUT:
    {
        char *url = get_string_value(node->http_put.url);
        char *data = get_string_value(node->http_put.data);
        if (!url || !data)
        {
            if (url)
                free(url);
            if (data)
                free(data);
            printf("Runtime error: Invalid URL or data for HTTP PUT\n");
            exit(1);
        }

        char *response = perform_http_request(url, "PUT", data, node->http_put.headers);
        free(url);
        free(data);

        if (response)
        {
            // Print the response directly
            printf("%s\n", response);
            set_variable("__http_response", response);
            free(response);
            return 0; // Success
        }
        return 0; // Failure
    }

    case NODE_HTTP_DELETE:
    {
        char *url = get_string_value(node->http_delete.url);
        if (!url)
        {
            printf("Runtime error: Invalid URL for HTTP DELETE\n");
            exit(1);
        }

        char *response = perform_http_request(url, "DELETE", NULL, node->http_delete.headers);
        free(url);

        if (response)
        {
            // Print the response directly
            printf("%s\n", response);
            set_variable("__http_response", response);
            free(response);
            return 0; // Success
        }
        return 0; // Failure
    }
    case NODE_TERNARY:
    {
        double condition = eval_expression(node->ternary.condition);
        if (condition != 0)
        {
            return eval_expression(node->ternary.true_expr);
        }
        else
        {
            return eval_expression(node->ternary.false_expr);
        }
    }
    case NODE_TEMPORAL_VAR:
    {
        // If this node has a time_offset, it's accessing past values
        if (node->temporal_var.time_offset)
        {
            int time_offset = (int)eval_expression(node->temporal_var.time_offset);
            const char *val = get_temporal_variable(node->temporal_var.varname, time_offset);
            if (!val)
            {
                printf("Runtime error: Cannot access temporal variable '%s' at offset %d\n", 
                       node->temporal_var.varname, time_offset);
                exit(1);
            }
            char *endptr;
            double dval = strtod(val, &endptr);
            if (endptr == val)
                return 0.0;
            return dval;
        }
        else
        {
            // This is a temporal variable creation node, return the max_history
            return node->number;
        }
    }
    case NODE_TEMPORAL_AGGREGATE:
    {
        TemporalVariable *temp_var = get_temporal_var_struct(node->temporal_aggregate.varname);
        if (!temp_var)
        {
            printf("Runtime error: Variable '%s' is not a temporal variable\n", node->temporal_aggregate.varname);
            exit(1);
        }
        
        int window_size = (int)eval_expression(node->temporal_aggregate.window_size);
        if (window_size <= 0 || window_size > temp_var->count)
            window_size = temp_var->count;
        
        const char *operation = node->temporal_aggregate.operation;
        double result = 0.0;
        
        if (strcmp(operation, "sum") == 0)
        {
            for (int i = temp_var->count - window_size; i < temp_var->count; i++)
            {
                result += strtod(temp_var->history[i].value, NULL);
            }
        }
        else if (strcmp(operation, "avg") == 0)
        {
            for (int i = temp_var->count - window_size; i < temp_var->count; i++)
            {
                result += strtod(temp_var->history[i].value, NULL);
            }
            result /= window_size;
        }
        else if (strcmp(operation, "min") == 0)
        {
            result = strtod(temp_var->history[temp_var->count - window_size].value, NULL);
            for (int i = temp_var->count - window_size + 1; i < temp_var->count; i++)
            {
                double val = strtod(temp_var->history[i].value, NULL);
                if (val < result) result = val;
            }
        }
        else if (strcmp(operation, "max") == 0)
        {
            result = strtod(temp_var->history[temp_var->count - window_size].value, NULL);
            for (int i = temp_var->count - window_size + 1; i < temp_var->count; i++)
            {
                double val = strtod(temp_var->history[i].value, NULL);
                if (val > result) result = val;
            }
        }
        else
        {
            printf("Runtime error: Unknown aggregation operation '%s'\n", operation);
            exit(1);
        }
        
        return result;
    }
    case NODE_TEMPORAL_PATTERN:
    {
        TemporalVariable *temp_var = get_temporal_var_struct(node->temporal_pattern.varname);
        if (!temp_var)
        {
            printf("Runtime error: Variable '%s' is not a temporal variable\n", node->temporal_pattern.varname);
            exit(1);
        }
        
        if (temp_var->count < 3)
        {
            return 0; // Not enough data for pattern detection
        }
        
        double threshold = eval_expression(node->temporal_pattern.threshold);
        const char *pattern_type = node->temporal_pattern.pattern_type;
        
        if (strcmp(pattern_type, "trend") == 0)
        {
            // Detect upward or downward trend
            int increasing = 0, decreasing = 0;
            for (int i = 1; i < temp_var->count; i++)
            {
                double prev = strtod(temp_var->history[i-1].value, NULL);
                double curr = strtod(temp_var->history[i].value, NULL);
                double change = (curr - prev) / (prev == 0 ? 1 : prev) * 100; // percentage change
                
                if (change > threshold) increasing++;
                else if (change < -threshold) decreasing++;
            }
            
            if (increasing > decreasing) return 1; // upward trend
            else if (decreasing > increasing) return -1; // downward trend
            else return 0; // no clear trend
        }
        else if (strcmp(pattern_type, "cycle") == 0)
        {
            // Simple cycle detection - look for alternating increases/decreases
            if (temp_var->count < 4) return 0;
            
            int pattern_matches = 0;
            for (int i = 2; i < temp_var->count - 1; i++)
            {
                double val1 = strtod(temp_var->history[i-2].value, NULL);
                double val2 = strtod(temp_var->history[i-1].value, NULL);
                double val3 = strtod(temp_var->history[i].value, NULL);
                double val4 = strtod(temp_var->history[i+1].value, NULL);
                
                // Check for peak or valley pattern
                if ((val2 > val1 && val2 > val3) || (val2 < val1 && val2 < val3))
                {
                    pattern_matches++;
                }
            }
            
            return pattern_matches >= 2 ? 1 : 0; // cycle detected if multiple peaks/valleys
        }
        else if (strcmp(pattern_type, "anomaly") == 0)
        {
            // Detect values that deviate significantly from the mean
            double sum = 0, mean, variance = 0, std_dev;
            
            // Calculate mean
            for (int i = 0; i < temp_var->count; i++)
            {
                sum += strtod(temp_var->history[i].value, NULL);
            }
            mean = sum / temp_var->count;
            
            // Calculate standard deviation
            for (int i = 0; i < temp_var->count; i++)
            {
                double val = strtod(temp_var->history[i].value, NULL);
                variance += (val - mean) * (val - mean);
            }
            std_dev = sqrt(variance / temp_var->count);
            
            // Check if current value is an anomaly
            double current = strtod(temp_var->history[temp_var->count - 1].value, NULL);
            double z_score = (current - mean) / (std_dev == 0 ? 1 : std_dev);
            
            return fabs(z_score) > threshold ? 1 : 0; // anomaly if z-score exceeds threshold
        }
        else
        {
            printf("Runtime error: Unknown pattern type '%s'\n", pattern_type);
            exit(1);
        }
        
        return 0;
    }

    case NODE_REGEX_MATCH:
    {
        ASTNode *regex_node = node->regex_match.regex;
        ASTNode *text_node = node->regex_match.text;

        if (regex_node->type == NODE_VAR)
        {
            regex_node = get_regex_variable(regex_node->varname);
        }

        if (!regex_node || regex_node->type != NODE_REGEX)
        {
            printf("Runtime error: Invalid regex in match operation\n");
            exit(1);
        }

        char *text_str = get_string_value(text_node);
        if (!text_str)
        {
            printf("Runtime error: Invalid text in regex match\n");
            exit(1);
        }

        int result = regex_match_pattern(regex_node->regex.pattern, text_str, regex_node->regex.flags);
        free(text_str);
        return result;
    }
    case NODE_REGEX_FIND_ALL:
    {
        ASTNode *regex_node = node->regex_find_all.regex;
        ASTNode *text_node = node->regex_find_all.text;

        if (regex_node->type == NODE_VAR)
        {
            regex_node = get_regex_variable(regex_node->varname);
        }

        if (!regex_node || regex_node->type != NODE_REGEX)
        {
            printf("Runtime error: Invalid regex in find_all operation\n");
            exit(1);
        }

        char *text_str = get_string_value(text_node);
        if (!text_str)
        {
            printf("Runtime error: Invalid text in regex find_all\n");
            exit(1);
        }

        ASTNode *result_list = ast_new_list();
        regex_find_all_matches(regex_node->regex.pattern, text_str, regex_node->regex.flags, result_list);

        char *list_str = list_to_string(result_list);
        printf("%s\n", list_str);
        free(list_str);
        free(text_str);
        ast_free(result_list);
        return 0;
    }
    case NODE_REGEX_REPLACE:
    {
        ASTNode *regex_node = node->regex_replace.regex;
        ASTNode *text_node = node->regex_replace.text;
        ASTNode *replacement_node = node->regex_replace.replacement;

        if (regex_node->type == NODE_VAR)
        {
            regex_node = get_regex_variable(regex_node->varname);
        }

        if (!regex_node || regex_node->type != NODE_REGEX)
        {
            printf("Runtime error: Invalid regex in replace operation\n");
            exit(1);
        }

        char *text_str = get_string_value(text_node);
        char *replacement_str = get_string_value(replacement_node);

        if (!text_str || !replacement_str)
        {
            printf("Runtime error: Invalid text or replacement in regex replace\n");
            exit(1);
        }

        char *result = regex_replace_pattern(regex_node->regex.pattern, text_str, replacement_str, regex_node->regex.flags);
        printf("%s\n", result);
        free(result);
        free(text_str);
        free(replacement_str);
        return 0;
    }
    default:
        fprintf(stderr, "Runtime error: Unsupported AST node type %d\n", node->type);
        exit(1);
    }
}

static char *get_string_value(ASTNode *node)
{
    if (!node)
        return NULL;


    
    if (node->type == NODE_STRING)
    {

        return strdup(node->string);
    }
    else if (node->type == NODE_FORMAT_STRING)
    {
        // Handle format strings that are actually just literal strings with @
        return strdup(node->format_str.format);
    }
    else if (node->type == NODE_VAR)
    {
        // Check if it's a regex variable first
        ASTNode *regex = get_regex_variable(node->varname);
        if (regex)
        {
            return strdup(regex->regex.pattern);
        }

        const char *val = get_variable(node->varname);
        if (val)
        {

            return strdup(val);
        }
        else
        {
            printf("Runtime error: Variable '%s' is not a string or is undefined.\n", node->varname);
            exit(1);
        }
    }
    else
    {
        // For other expression types, evaluate them to a number and convert to string
        double val = eval_expression(node);

        char *buffer = malloc(64); // Sufficient for a double
        if (buffer)
        {
            snprintf(buffer, 64, "%g", val);
        }
        return buffer;
    }
}

// Function to convert a list to a string representation
static char *list_to_string(ASTNode *list)
{
    if (list->type != NODE_LIST)
    {
        return strdup("Not a list");
    }

    char *result = strdup("[");
    for (int i = 0; i < list->list.count; i++)
    {
        char buffer[256];
        ASTNode *element = list->list.elements[i];

        if (element->type == NODE_NUMBER)
        {
            snprintf(buffer, sizeof(buffer), "%g", element->number);
        }
        else if (element->type == NODE_STRING)
        {
            snprintf(buffer, sizeof(buffer), "%s", element->string);
        }
        else
        {
            snprintf(buffer, sizeof(buffer), "Unknown");
        }

        result = realloc(result, strlen(result) + strlen(buffer) + 3);
        strcat(result, buffer);

        if (i < list->list.count - 1)
        {
            strcat(result, ", ");
        }
    }

    strcat(result, "]");
    return result;
}

// Modify your print logic to handle lists
static void print_node(ASTNode *node)
{
    switch (node->type)
    {
    case NODE_NUMBER:
        printf("%g\n", node->number);
        break;
    case NODE_STRING:
        printf("%s\n", node->string);
        break;
    case NODE_LIST:
    {
        char *list_str = list_to_string(node);
        printf("%s\n", list_str);
        free(list_str);
        break;
    }
    case NODE_DICT:
    {
        printf("{");
        for (int i = 0; i < node->dict.count; i++)
        {
            if (node->dict.keys[i]->type == NODE_STRING)
                printf("\"%s\"", node->dict.keys[i]->string);
            else if (node->dict.keys[i]->type == NODE_NUMBER)
                printf("%g", node->dict.keys[i]->number);
            printf(" := ");
            if (node->dict.values[i]->type == NODE_STRING)
                printf("\"%s\"", node->dict.values[i]->string);
            else if (node->dict.values[i]->type == NODE_NUMBER)
                printf("%g", node->dict.values[i]->number);
            if (i < node->dict.count - 1)
                printf(", ");
        }
        printf("}\n");
        break;
    }
    case NODE_STACK:
    {
        printf("<");
        for (int i = 0; i < node->stack.count; i++)
        {
            if (node->stack.elements[i]->type == NODE_STRING)
                printf("%s", node->stack.elements[i]->string);
            else if (node->stack.elements[i]->type == NODE_NUMBER)
                printf("%g", node->stack.elements[i]->number);
            if (i < node->stack.count - 1)
                printf(", ");
        }
        printf(">\n");
        break;
    }
    case NODE_QUEUE:
    {
        printf("<");
        for (int i = 0; i < node->queue.count; i++)
        {
            if (node->queue.elements[i]->type == NODE_STRING)
                printf("%s", node->queue.elements[i]->string);
            else if (node->queue.elements[i]->type == NODE_NUMBER)
                printf("%g", node->queue.elements[i]->number);
            if (i < node->queue.count - 1)
                printf(", ");
        }
        printf(">\n");
        break;
    }
    case NODE_LINKED_LIST:
    {
        printf("[");
        for (int i = 0; i < node->linked_list.count; i++)
        {
            if (node->linked_list.elements[i]->type == NODE_STRING)
                printf("%s", node->linked_list.elements[i]->string);
            else if (node->linked_list.elements[i]->type == NODE_NUMBER)
                printf("%g", node->linked_list.elements[i]->number);
            if (i < node->linked_list.count - 1)
                printf(", ");
        }
        printf("]\n");
        break;
    }
    case NODE_REGEX:
    {
        printf("/%s/%s\n", node->regex.pattern, node->regex.flags);
        break;
    }

    case NODE_VAR:
    {
        // Check if it's a temporal variable first
        TemporalVariable *temp_var = get_temporal_var_struct(node->varname);
        if (temp_var)
        {
            const char *val = get_temporal_variable(node->varname, 0);
            if (val)
            {
                printf("%s\n", val);
            }
            else
            {
                printf("Runtime Error: Cannot access temporal variable '%s'\n", node->varname);
            }
        }
        // Check if it's a dict variable
        else if ((temp_var = NULL, get_dict_variable(node->varname)))
        {
            ASTNode *dict = get_dict_variable(node->varname);
            print_node(dict);
        }
        // Check if it's a list variable
        else if (get_list_variable(node->varname))
        {
            ASTNode *list = get_list_variable(node->varname);
            char *list_str = list_to_string(list);
            printf("%s\n", list_str);
            free(list_str);
        }
        // Check if it's a stack variable
        else if (get_stack_variable(node->varname))
        {
            ASTNode *stack = get_stack_variable(node->varname);
            print_node(stack);
        }
        // Check if it's a queue variable
        else if (get_queue_variable(node->varname))
        {
            ASTNode *queue = get_queue_variable(node->varname);
            print_node(queue);
        }
        // Check if it's a linked list variable
        else if (get_linked_list_variable(node->varname))
        {
            ASTNode *linked_list = get_linked_list_variable(node->varname);
            print_node(linked_list);
        }
        // Check if it's a regex variable
        else if (get_regex_variable(node->varname))
        {
            ASTNode *regex = get_regex_variable(node->varname);
            print_node(regex);
        }
        else
        {
            const char *val = get_variable(node->varname);
            if (val)
            {
                printf("%s\n", val);
            }
            else
            {
                printf("Runtime Error: Undefined variable '%s'\n", node->varname);
            }
        }

        break;
    }
    case NODE_TEMPORAL_VAR:
    {
        if (node->temporal_var.time_offset)
        {
            double result = eval_expression(node);
            printf("%g\n", result);
        }
        else
        {
            printf("<temp@%g>\n", node->number);
        }
        break;
    }
    case NODE_FORMAT_STRING:
        eval_expression(node);
        break;
    case NODE_TO_STR:
    {
        double result = eval_expression(node);
        if (result == -12345.6789)
        {
            const char *str_result = get_variable("__to_str_result");
            if (str_result)
            {
                printf("%s\n", str_result);
            }
        }
        else
        {
            printf("%g\n", result);
        }
        break;
    }
    case NODE_HTTP_GET:
    case NODE_HTTP_POST:
    case NODE_HTTP_PUT:
    case NODE_HTTP_DELETE:
    {
        // HTTP requests are handled directly in eval_expression
        // which will print the response
        eval_expression(node);
        break;
    }
    case NODE_DICT_GET:
    {
        ASTNode *dict_node = node->dict_get.dict;
        if (dict_node->type == NODE_VAR)
        {
            dict_node = get_dict_variable(dict_node->varname);
        }
        if (dict_node && dict_node->type == NODE_DICT)
        {
            ASTNode *key = node->dict_get.key;
            for (int i = 0; i < dict_node->dict.count; i++)
            {
                if (dict_node->dict.keys[i]->type == NODE_STRING && key->type == NODE_STRING)
                {
                    if (strcmp(dict_node->dict.keys[i]->string, key->string) == 0)
                    {
                        if (dict_node->dict.values[i]->type == NODE_STRING)
                            printf("%s\n", dict_node->dict.values[i]->string);
                        else if (dict_node->dict.values[i]->type == NODE_NUMBER)
                            printf("%g\n", dict_node->dict.values[i]->number);
                        return;
                    }
                }
            }
        }
        printf("(not found)\n");
        break;
    }
    case NODE_MEMBER_ACCESS:
    {
        ASTNode *object = node->member_access.object;
        const char *member_name = node->member_access.member_name;
        ObjectInstance *obj = NULL;

        // Handle self.member access
        if (object->type == NODE_VAR && strcmp(object->varname, "self") == 0 && current_self)
        {
            obj = current_self;
        }
        // Handle obj.member access
        else if (object->type == NODE_VAR)
        {
            const char *obj_ptr_str = get_variable(object->varname);
            if (obj_ptr_str)
            {
                sscanf(obj_ptr_str, "%p", (void **)&obj);
            }
        }

        if (obj)
        {
            FieldEntry *field = object_get_field(obj, member_name);
            if (field)
            {
                if (field->type == FIELD_STRING)
                {
                    printf("%s\n", field->string_value);
                    return;
                }
                else if (field->type == FIELD_NUMBER)
                {
                    printf("%g\n", field->number_value);
                    return;
                }
            }
        }
        printf("(unknown member)\n");
        break;
    }
    case NODE_LINKED_LIST_GET:
    {
        ASTNode *list_node = node->linked_list_get.list;
        int index = (int)eval_expression(node->linked_list_get.index);

        if (list_node->type == NODE_VAR)
        {
            list_node = get_linked_list_variable(list_node->varname);
        }

        if (list_node && list_node->type == NODE_LINKED_LIST &&
            index >= 0 && index < list_node->linked_list.count)
        {
            ASTNode *element = list_node->linked_list.elements[index];
            if (element->type == NODE_STRING)
            {
                printf("%s\n", element->string);
                return;
            }
            else if (element->type == NODE_NUMBER)
            {
                printf("%g\n", element->number);
                return;
            }
        }
        // If we get here, something went wrong
        double result = eval_expression(node);
        printf("%g\n", result);
        break;
    }
    default:
    {
        double result = eval_expression(node);
        // Handle ternary operator specially for printing
        if (node->type == NODE_TERNARY)
        {
            double condition = eval_expression(node->ternary.condition);
            ASTNode *result_expr = (condition != 0) ? node->ternary.true_expr : node->ternary.false_expr;
            
            if (result_expr->type == NODE_STRING)
            {
                printf("%s\n", result_expr->string);
            }
            else
            {
                double expr_result = eval_expression(result_expr);
                
                // Check if this is a boolean context (ternary with true/false literals)
                int is_boolean_context = 0;
                if ((node->ternary.true_expr->type == NODE_NUMBER && 
                     (node->ternary.true_expr->number == 1.0 || node->ternary.true_expr->number == 0.0)) ||
                    (node->ternary.false_expr->type == NODE_NUMBER && 
                     (node->ternary.false_expr->number == 1.0 || node->ternary.false_expr->number == 0.0)))
                {
                    is_boolean_context = 1;
                }
                
                // Also check if the condition is a boolean operation
                if ((node->ternary.condition->type == NODE_BINOP &&
                     (node->ternary.condition->binop.op == TOK_EQ || node->ternary.condition->binop.op == TOK_NEQ ||
                      node->ternary.condition->binop.op == TOK_LT || node->ternary.condition->binop.op == TOK_GT ||
                      node->ternary.condition->binop.op == TOK_LTE || node->ternary.condition->binop.op == TOK_GTE)) ||
                    node->ternary.condition->type == NODE_AND || node->ternary.condition->type == NODE_OR || node->ternary.condition->type == NODE_NOT)
                {
                    is_boolean_context = 1;
                }
                
                // Check if the result expression itself is a boolean operation
                if ((result_expr->type == NODE_BINOP &&
                     (result_expr->binop.op == TOK_EQ || result_expr->binop.op == TOK_NEQ ||
                      result_expr->binop.op == TOK_LT || result_expr->binop.op == TOK_GT ||
                      result_expr->binop.op == TOK_LTE || result_expr->binop.op == TOK_GTE)) ||
                    result_expr->type == NODE_AND || result_expr->type == NODE_OR || result_expr->type == NODE_NOT)
                {
                    is_boolean_context = 1;
                }
                
                if (is_boolean_context && (expr_result == 1.0 || expr_result == 0.0))
                {
                    printf("%s\n", bool_to_str((bool)expr_result));
                }
                else
                {
                    printf("%g\n", expr_result);
                }
            }
        }
        // Check if this is a boolean result (0 or 1 from boolean operations)
        else if ((result == 0.0 || result == 1.0) && 
            (node->type == NODE_AND || node->type == NODE_OR || node->type == NODE_NOT ||
             (node->type == NODE_BINOP && 
              (node->binop.op == TOK_EQ || node->binop.op == TOK_NEQ ||
               node->binop.op == TOK_LT || node->binop.op == TOK_GT ||
               node->binop.op == TOK_LTE || node->binop.op == TOK_GTE))))
        {
            printf("%s\n", bool_to_str((bool)result));
        }
        else
        {
            printf("%g\n", result);
        }
        break;
    }
    }
}

// Helper function to convert boolean to string
const char *bool_to_str(bool value)
{
    return value ? "true" : "false";
}

// Simple regex engine implementation
static int regex_match_char(char pattern_char, char text_char, int case_insensitive)
{
    if (case_insensitive)
    {
        return tolower(pattern_char) == tolower(text_char);
    }
    return pattern_char == text_char;
}

static int regex_match_at_position(const char *pattern, const char *text, int case_insensitive)
{
    int p = 0, t = 0;
    

    
    while (pattern[p])
    {
        // Handle escape sequences - check for double backslash from Tesseract parsing
        if ((pattern[p] == '\\' && pattern[p + 1] == '\\' && pattern[p + 2]) || 
            (pattern[p] == '\\' && pattern[p + 1]))
        {
            char next;
            int base_skip;
            if (pattern[p + 1] == '\\' && pattern[p + 2])
            {
                // Double backslash case: \\d
                next = pattern[p + 2];
                base_skip = 3;
            }
            else
            {
                // Single backslash case: \d
                next = pattern[p + 1];
                base_skip = 2;
            }
            
            if (next == 'd')
            {
                // Check for quantifier after \d
                if (pattern[p + base_skip] == '+')
                {
                    // \d+ - one or more digits
                    if (!text[t] || !isdigit(text[t])) return 0;
                    while (text[t] && isdigit(text[t])) t++;
                    p += base_skip + 1;
                }
                else if (pattern[p + base_skip] == '*')
                {
                    // \d* - zero or more digits
                    while (text[t] && isdigit(text[t])) t++;
                    p += base_skip + 1;
                }
                else if (pattern[p + base_skip] == '{')
                {
                    // \d{n} - exactly n digits
                    int brace_end = p + base_skip + 1;
                    while (pattern[brace_end] && pattern[brace_end] != '}') brace_end++;
                    if (!pattern[brace_end]) return 0;
                    
                    int count = 0;
                    for (int i = p + base_skip + 1; i < brace_end; i++)
                    {
                        if (isdigit(pattern[i]))
                            count = count * 10 + (pattern[i] - '0');
                        else
                            break;
                    }
                    
                    for (int i = 0; i < count; i++)
                    {
                        if (!text[t] || !isdigit(text[t])) return 0;
                        t++;
                    }
                    p = brace_end + 1;
                }
                else
                {
                    // \d - single digit
                    if (!text[t] || !isdigit(text[t])) return 0;
                    t++;
                    p += base_skip;
                }
            }
            else if (next == 'w')
            {
                if (pattern[p + base_skip] == '+')
                {
                    if (!text[t] || (!isalnum(text[t]) && text[t] != '_')) return 0;
                    while (text[t] && (isalnum(text[t]) || text[t] == '_')) t++;
                    p += base_skip + 1;
                }
                else
                {
                    if (!text[t] || (!isalnum(text[t]) && text[t] != '_')) return 0;
                    t++;
                    p += base_skip;
                }
            }
            else if (next == 's')
            {
                if (pattern[p + base_skip] == '+')
                {
                    if (!text[t] || !isspace(text[t])) return 0;
                    while (text[t] && isspace(text[t])) t++;
                    p += base_skip + 1;
                }
                else
                {
                    if (!text[t] || !isspace(text[t])) return 0;
                    t++;
                    p += base_skip;
                }
            }
            else if (next == '(' || next == ')')
            {
                // Handle escaped parentheses - check for optional quantifier
                if (pattern[p + base_skip] == '?')
                {
                    // Optional parenthesis - try with and without
                    if (text[t] && regex_match_char(next, text[t], case_insensitive))
                    {
                        if (regex_match_at_position(pattern + p + base_skip + 1, text + t + 1, case_insensitive))
                            return 1;
                    }
                    return regex_match_at_position(pattern + p + base_skip + 1, text + t, case_insensitive);
                }
                else
                {
                    // Literal parenthesis
                    if (!text[t] || !regex_match_char(next, text[t], case_insensitive)) return 0;
                    p += base_skip;
                    t++;
                }
            }
            else
            {
                // Literal escaped character
                if (!text[t] || !regex_match_char(next, text[t], case_insensitive)) return 0;
                p += base_skip;
                t++;
            }
        }
        // Handle character classes
        else if (pattern[p] == '[')
        {
            int class_end = p + 1;
            while (pattern[class_end] && pattern[class_end] != ']') class_end++;
            if (!pattern[class_end]) return 0; // Malformed class
            
            int match = 0;
            for (int i = p + 1; i < class_end; i++)
            {
                if (i + 2 < class_end && pattern[i + 1] == '-')
                {
                    // Range like a-z or 0-9
                    if (text[t] >= pattern[i] && text[t] <= pattern[i + 2])
                    {
                        match = 1;
                        break;
                    }
                    i += 2;
                }
                else
                {
                    if (regex_match_char(pattern[i], text[t], case_insensitive))
                    {
                        match = 1;
                        break;
                    }
                }
            }
            if (!match) return 0;
            
            // Check for quantifier after character class
            if (pattern[class_end + 1] == '+')
            {
                // One or more matches
                t++; // We already matched one
                while (text[t])
                {
                    int class_match = 0;
                    for (int i = p + 1; i < class_end; i++)
                    {
                        if (i + 2 < class_end && pattern[i + 1] == '-')
                        {
                            if (text[t] >= pattern[i] && text[t] <= pattern[i + 2])
                            {
                                class_match = 1;
                                break;
                            }
                            i += 2;
                        }
                        else
                        {
                            if (regex_match_char(pattern[i], text[t], case_insensitive))
                            {
                                class_match = 1;
                                break;
                            }
                        }
                    }
                    if (!class_match) break;
                    t++;
                }
                p = class_end + 2;
            }
            else if (pattern[class_end + 1] == '?')
            {
                // Optional character class - try with and without
                if (text[t])
                {
                    int class_match = 0;
                    for (int i = p + 1; i < class_end; i++)
                    {
                        if (i + 2 < class_end && pattern[i + 1] == '-')
                        {
                            if (text[t] >= pattern[i] && text[t] <= pattern[i + 2])
                            {
                                class_match = 1;
                                break;
                            }
                            i += 2;
                        }
                        else
                        {
                            if (regex_match_char(pattern[i], text[t], case_insensitive))
                            {
                                class_match = 1;
                                break;
                            }
                        }
                    }
                    if (class_match)
                    {
                        if (regex_match_at_position(pattern + class_end + 2, text + t + 1, case_insensitive))
                            return 1;
                    }
                }
                return regex_match_at_position(pattern + class_end + 2, text + t, case_insensitive);
            }
            else
            {
                p = class_end + 1;
                t++;
            }
        }
        else if (pattern[p] == '.')
        {
            // Match any character
            if (!text[t]) return 0;
            p++;
            t++;
        }
        else
        {
            // Check for quantifiers after literal characters
            if (pattern[p + 1] == '?')
            {
                // Optional character - try with and without
                if (text[t] && regex_match_char(pattern[p], text[t], case_insensitive))
                {
                    // Try matching with the character
                    if (regex_match_at_position(pattern + p + 2, text + t + 1, case_insensitive))
                        return 1;
                }
                // Try matching without the character
                return regex_match_at_position(pattern + p + 2, text + t, case_insensitive);
            }
            else if (pattern[p + 1] == '*')
            {
                // Zero or more - try all possibilities
                // First try without matching any
                if (regex_match_at_position(pattern + p + 2, text + t, case_insensitive))
                    return 1;
                // Then try matching one or more
                while (text[t] && regex_match_char(pattern[p], text[t], case_insensitive))
                {
                    t++;
                    if (regex_match_at_position(pattern + p + 2, text + t, case_insensitive))
                        return 1;
                }
                return 0;
            }
            else if (pattern[p + 1] == '+')
            {
                // One or more
                if (!text[t] || !regex_match_char(pattern[p], text[t], case_insensitive)) return 0;
                t++;
                while (text[t] && regex_match_char(pattern[p], text[t], case_insensitive))
                {
                    t++;
                }
                p += 2;
            }
            else if (pattern[p + 1] == '{')
            {
                // Quantifier like {3} or {2,4}
                int brace_end = p + 2;
                while (pattern[brace_end] && pattern[brace_end] != '}') brace_end++;
                if (!pattern[brace_end]) return 0; // Malformed quantifier
                
                // Extract number from {n}
                int count = 0;
                for (int i = p + 2; i < brace_end; i++)
                {
                    if (isdigit(pattern[i]))
                        count = count * 10 + (pattern[i] - '0');
                    else
                        break; // Stop at comma or other chars for now
                }
                
                // Match exactly 'count' occurrences
                for (int i = 0; i < count; i++)
                {
                    if (!text[t] || !regex_match_char(pattern[p], text[t], case_insensitive)) return 0;
                    t++;
                }
                p = brace_end + 1;
            }
            else
            {
                // Literal character match
                if (!text[t] || !regex_match_char(pattern[p], text[t], case_insensitive)) return 0;
                p++;
                t++;
            }
        }
    }
    

    return 1; // Successfully matched entire pattern
}

static int regex_match_pattern(const char *pattern, const char *text, const char *flags)
{
    int case_insensitive = (strchr(flags, 'i') != NULL);
    int text_len = strlen(text);
    
    // Try matching at each position in the text
    for (int i = 0; i <= text_len; i++)
    {
        if (regex_match_at_position(pattern, text + i, case_insensitive))
        {
            return 1;
        }
    }
    
    return 0;
}

static int regex_get_match_length(const char *pattern, const char *text, int case_insensitive)
{
    int p = 0, t = 0;
    
    while (pattern[p])
    {
        if ((pattern[p] == '\\' && pattern[p + 1] == '\\' && pattern[p + 2]) || 
            (pattern[p] == '\\' && pattern[p + 1]))
        {
            char next;
            int base_skip;
            if (pattern[p + 1] == '\\' && pattern[p + 2])
            {
                next = pattern[p + 2];
                base_skip = 3;
            }
            else
            {
                next = pattern[p + 1];
                base_skip = 2;
            }
            if (next == 'd')
            {
                if (pattern[p + base_skip] == '+')
                {
                    if (!text[t] || !isdigit(text[t])) return 0;
                    while (text[t] && isdigit(text[t])) t++;
                    p += base_skip + 1;
                }
                else if (pattern[p + base_skip] == '*')
                {
                    while (text[t] && isdigit(text[t])) t++;
                    p += base_skip + 1;
                }
                else if (pattern[p + base_skip] == '{')
                {
                    int brace_end = p + base_skip + 1;
                    while (pattern[brace_end] && pattern[brace_end] != '}') brace_end++;
                    if (!pattern[brace_end]) return 0;
                    
                    int count = 0;
                    for (int i = p + base_skip + 1; i < brace_end; i++)
                    {
                        if (isdigit(pattern[i]))
                            count = count * 10 + (pattern[i] - '0');
                        else
                            break;
                    }
                    
                    for (int i = 0; i < count; i++)
                    {
                        if (!text[t] || !isdigit(text[t])) return 0;
                        t++;
                    }
                    p = brace_end + 1;
                }
                else
                {
                    if (!text[t] || !isdigit(text[t])) return 0;
                    t++;
                    p += base_skip;
                }
            }
            else if (next == 'w')
            {
                if (pattern[p + base_skip] == '+')
                {
                    if (!text[t] || (!isalnum(text[t]) && text[t] != '_')) return 0;
                    while (text[t] && (isalnum(text[t]) || text[t] == '_')) t++;
                    p += base_skip + 1;
                }
                else
                {
                    if (!text[t] || (!isalnum(text[t]) && text[t] != '_')) return 0;
                    t++;
                    p += base_skip;
                }
            }
            else
            {
                if (!text[t] || !regex_match_char(next, text[t], case_insensitive)) return 0;
                p += base_skip;
                t++;
            }
        }
        else if (pattern[p] == '[')
        {
            int class_end = p + 1;
            while (pattern[class_end] && pattern[class_end] != ']') class_end++;
            if (!pattern[class_end]) return 0;
            
            if (pattern[class_end + 1] == '+')
            {
                // Character class with +
                int matched = 0;
                while (text[t])
                {
                    int class_match = 0;
                    for (int i = p + 1; i < class_end; i++)
                    {
                        if (i + 2 < class_end && pattern[i + 1] == '-')
                        {
                            if (text[t] >= pattern[i] && text[t] <= pattern[i + 2])
                            {
                                class_match = 1;
                                break;
                            }
                            i += 2;
                        }
                        else
                        {
                            if (regex_match_char(pattern[i], text[t], case_insensitive))
                            {
                                class_match = 1;
                                break;
                            }
                        }
                    }
                    if (!class_match) break;
                    t++;
                    matched = 1;
                }
                if (!matched) return 0;
                p = class_end + 2;
            }
            else
            {
                // Single character class match
                if (!text[t]) return 0;
                t++;
                p = class_end + 1;
            }
        }
        else
        {
            // Handle quantifiers for literal characters
            if (pattern[p + 1] == '+')
            {
                if (!text[t] || !regex_match_char(pattern[p], text[t], case_insensitive)) return 0;
                while (text[t] && regex_match_char(pattern[p], text[t], case_insensitive)) t++;
                p += 2;
            }
            else if (pattern[p + 1] == '*')
            {
                while (text[t] && regex_match_char(pattern[p], text[t], case_insensitive)) t++;
                p += 2;
            }
            else if (pattern[p + 1] == '?')
            {
                if (text[t] && regex_match_char(pattern[p], text[t], case_insensitive)) t++;
                p += 2;
            }
            else if (pattern[p + 1] == '{')
            {
                int brace_end = p + 2;
                while (pattern[brace_end] && pattern[brace_end] != '}') brace_end++;
                if (!pattern[brace_end]) return 0;
                
                int count = 0;
                for (int i = p + 2; i < brace_end; i++)
                {
                    if (isdigit(pattern[i]))
                        count = count * 10 + (pattern[i] - '0');
                    else
                        break;
                }
                
                for (int i = 0; i < count; i++)
                {
                    if (!text[t] || !regex_match_char(pattern[p], text[t], case_insensitive)) return 0;
                    t++;
                }
                p = brace_end + 1;
            }
            else
            {
                if (!text[t] || !regex_match_char(pattern[p], text[t], case_insensitive)) return 0;
                p++;
                t++;
            }
        }
    }
    
    return t; // Return length of match
}

static void regex_find_all_matches(const char *pattern, const char *text, const char *flags, ASTNode *result_list)
{
    int case_insensitive = (strchr(flags, 'i') != NULL);
    int text_len = strlen(text);
    
    for (int i = 0; i < text_len; i++)
    {
        if (regex_match_at_position(pattern, text + i, case_insensitive))
        {
            ast_list_add_element(result_list, ast_new_number(i));
            
            // Calculate match length to skip ahead properly
            int match_len = regex_get_match_length(pattern, text + i, case_insensitive);
            if (match_len > 1)
            {
                i += match_len - 1; // -1 because loop will increment
            }
        }
    }
}

static char *regex_replace_pattern(const char *pattern, const char *text, const char *replacement, const char *flags)
{
    int case_insensitive = (strchr(flags, 'i') != NULL);
    int global_flag = (strchr(flags, 'g') != NULL);
    int text_len = strlen(text);
    int replacement_len = strlen(replacement);
    
    // Allocate generous buffer for result
    char *result = malloc(text_len * 3 + 1);
    int result_pos = 0;
    int text_pos = 0;
    
    while (text_pos < text_len)
    {
        if (regex_match_at_position(pattern, text + text_pos, case_insensitive))
        {
            // Found a match, add replacement
            strcpy(result + result_pos, replacement);
            result_pos += replacement_len;
            
            // Skip the matched portion
            int match_len = regex_get_match_length(pattern, text + text_pos, case_insensitive);
            if (match_len <= 0) match_len = 1; // Ensure we advance at least 1 character
            text_pos += match_len;
            
            if (!global_flag) break;
        }
        else
        {
            // No match, copy character as-is
            result[result_pos++] = text[text_pos++];
        }
    }
    
    // Copy remaining text if we stopped early (non-global replace)
    if (text_pos < text_len)
    {
        strcpy(result + result_pos, text + text_pos);
        result_pos += text_len - text_pos;
    }
    
    result[result_pos] = '\0';
    return result;
}

// Modify your print function (if you have one)
void interpret_print(ASTNode *node)
{
    double value = eval_expression(node->binop.left);
    // Treat values from boolean operations and comparison operators as booleans
    bool is_bool_op = node->binop.left->type == NODE_AND ||
                      node->binop.left->type == NODE_OR ||
                      node->binop.left->type == NODE_NOT;

    bool is_comp_op = node->binop.left->type == NODE_BINOP &&
                      (node->binop.left->binop.op == TOK_EQ ||
                       node->binop.left->binop.op == TOK_NEQ ||
                       node->binop.left->binop.op == TOK_LT ||
                       node->binop.left->binop.op == TOK_GT ||
                       node->binop.left->binop.op == TOK_LTE ||
                       node->binop.left->binop.op == TOK_GTE);

    if ((is_bool_op || is_comp_op) && (value == 0 || value == 1))
    {
        printf("%s\n", bool_to_str((bool)value));
    }
    else
    {
        printf("%g\n", value);
    }
}