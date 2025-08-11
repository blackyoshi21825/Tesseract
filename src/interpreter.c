#include "tesseract_pch.h"
#include "error.h"
#include "../packages/core/package_loader.h"
#include <ctype.h>

extern int debug_mode;

// Package initialization functions
void init_date_time_package();
void init_math_utils_package();
void init_string_utils_package();
void init_algorithms_package();
void init_crypto_utils_package();
void init_file_utils_package();
void init_json_utils_package();
void init_network_utils_package();
void init_random_utils_package();
void init_system_utils_package();
void init_database_package();
void init_console_utils_package();
void init_time_package();

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

// Loop control flags
static int break_flag = 0;
static int continue_flag = 0;
static int packages_initialized = 0;

static void initialize_builtin_functions() {
    // Register built-in functions
    
    // abs(x) - absolute value
    char abs_params[4][64] = {"x"};
    ASTNode *abs_body = ast_new_block();
    ASTNode *abs_condition = ast_new_binop(ast_new_var("x"), ast_new_number(0), TOK_LT);
    ASTNode *abs_neg = ast_new_binop(ast_new_number(0), ast_new_var("x"), TOK_MINUS);
    ASTNode *abs_ternary = ast_new_ternary(abs_condition, abs_neg, ast_new_var("x"));
    ast_block_add_statement(abs_body, abs_ternary);
    register_function("abs", abs_params, 1, abs_body);
    
    // max(a, b) - maximum of two values
    char max_params[4][64] = {"a", "b"};
    ASTNode *max_body = ast_new_block();
    ASTNode *max_condition = ast_new_binop(ast_new_var("a"), ast_new_var("b"), TOK_GT);
    ASTNode *max_ternary = ast_new_ternary(max_condition, ast_new_var("a"), ast_new_var("b"));
    ast_block_add_statement(max_body, max_ternary);
    register_function("max", max_params, 2, max_body);
    
    // min(a, b) - minimum of two values
    char min_params[4][64] = {"a", "b"};
    ASTNode *min_body = ast_new_block();
    ASTNode *min_condition = ast_new_binop(ast_new_var("a"), ast_new_var("b"), TOK_LT);
    ASTNode *min_ternary = ast_new_ternary(min_condition, ast_new_var("a"), ast_new_var("b"));
    ast_block_add_statement(min_body, min_ternary);
    register_function("min", min_params, 2, min_body);
    
    // pow(base, exp) - power function
    char pow_params[4][64] = {"base", "exp"};
    ASTNode *pow_body = ast_new_block();
    // Simple implementation: result = 1, loop exp times multiplying by base
    ast_block_add_statement(pow_body, ast_new_assign("result", ast_new_number(1)));
    ast_block_add_statement(pow_body, ast_new_assign("i", ast_new_number(0)));
    ASTNode *pow_loop_cond = ast_new_binop(ast_new_var("i"), ast_new_var("exp"), TOK_LT);
    ASTNode *pow_loop_body = ast_new_block();
    ASTNode *pow_mult = ast_new_binop(ast_new_var("result"), ast_new_var("base"), TOK_MUL);
    ast_block_add_statement(pow_loop_body, ast_new_assign("result", pow_mult));
    ast_block_add_statement(pow_loop_body, ast_new_increment("i", 0));
    ast_block_add_statement(pow_body, ast_new_while(pow_loop_cond, pow_loop_body));
    ast_block_add_statement(pow_body, ast_new_var("result"));
    register_function("pow", pow_params, 2, pow_body);
    
    // sqrt(x) - square root (Newton's method approximation)
    char sqrt_params[4][64] = {"x"};
    ASTNode *sqrt_body = ast_new_block();
    // Check for negative input
    ASTNode *sqrt_neg_check = ast_new_binop(ast_new_var("x"), ast_new_number(0), TOK_LT);
    ASTNode *sqrt_neg_return = ast_new_number(0); // Return 0 for negative input
    ASTNode *sqrt_pos_calc = ast_new_block();
    // Newton's method: guess = x/2, iterate guess = (guess + x/guess) / 2
    ast_block_add_statement(sqrt_pos_calc, ast_new_assign("guess", ast_new_binop(ast_new_var("x"), ast_new_number(2), TOK_DIV)));
    ast_block_add_statement(sqrt_pos_calc, ast_new_assign("i", ast_new_number(0)));
    ASTNode *sqrt_iter_cond = ast_new_binop(ast_new_var("i"), ast_new_number(10), TOK_LT); // 10 iterations
    ASTNode *sqrt_iter_body = ast_new_block();
    ASTNode *sqrt_div = ast_new_binop(ast_new_var("x"), ast_new_var("guess"), TOK_DIV);
    ASTNode *sqrt_add = ast_new_binop(ast_new_var("guess"), sqrt_div, TOK_PLUS);
    ASTNode *sqrt_new_guess = ast_new_binop(sqrt_add, ast_new_number(2), TOK_DIV);
    ast_block_add_statement(sqrt_iter_body, ast_new_assign("guess", sqrt_new_guess));
    ast_block_add_statement(sqrt_iter_body, ast_new_increment("i", 0));
    ast_block_add_statement(sqrt_pos_calc, ast_new_while(sqrt_iter_cond, sqrt_iter_body));
    ast_block_add_statement(sqrt_pos_calc, ast_new_var("guess"));
    ASTNode *sqrt_ternary = ast_new_ternary(sqrt_neg_check, sqrt_neg_return, sqrt_pos_calc);
    ast_block_add_statement(sqrt_body, sqrt_ternary);
    register_function("sqrt", sqrt_params, 1, sqrt_body);
    
    // factorial(n) - factorial function
    char fact_params[4][64] = {"n"};
    ASTNode *fact_body = ast_new_block();
    // Check for n <= 1
    ASTNode *fact_base_check = ast_new_binop(ast_new_var("n"), ast_new_number(1), TOK_LTE);
    ASTNode *fact_base_return = ast_new_number(1);
    ASTNode *fact_calc = ast_new_block();
    ast_block_add_statement(fact_calc, ast_new_assign("result", ast_new_number(1)));
    ast_block_add_statement(fact_calc, ast_new_assign("i", ast_new_number(2)));
    ASTNode *fact_loop_cond = ast_new_binop(ast_new_var("i"), ast_new_var("n"), TOK_LTE);
    ASTNode *fact_loop_body = ast_new_block();
    ASTNode *fact_mult = ast_new_binop(ast_new_var("result"), ast_new_var("i"), TOK_MUL);
    ast_block_add_statement(fact_loop_body, ast_new_assign("result", fact_mult));
    ast_block_add_statement(fact_loop_body, ast_new_increment("i", 0));
    ast_block_add_statement(fact_calc, ast_new_while(fact_loop_cond, fact_loop_body));
    ast_block_add_statement(fact_calc, ast_new_var("result"));
    ASTNode *fact_ternary = ast_new_ternary(fact_base_check, fact_base_return, fact_calc);
    ast_block_add_statement(fact_body, fact_ternary);

    
    // floor(x) - floor function
    char floor_params[4][64] = {"x"};
    ASTNode *floor_body = ast_new_block();
    // Simple floor: if x >= 0, truncate; if x < 0 and has decimal, subtract 1
    ASTNode *floor_int_part = ast_new_to_int(ast_new_var("x"));
    ASTNode *floor_is_negative = ast_new_binop(ast_new_var("x"), ast_new_number(0), TOK_LT);
    ASTNode *floor_has_decimal = ast_new_binop(ast_new_var("x"), floor_int_part, TOK_NEQ);
    ASTNode *floor_both_cond = ast_new_and(floor_is_negative, floor_has_decimal);
    ASTNode *floor_subtract = ast_new_binop(floor_int_part, ast_new_number(1), TOK_MINUS);
    ASTNode *floor_result = ast_new_ternary(floor_both_cond, floor_subtract, floor_int_part);
    ast_block_add_statement(floor_body, floor_result);
    register_function("floor", floor_params, 1, floor_body);
    
    // ceil(x) - ceiling function
    char ceil_params[4][64] = {"x"};
    ASTNode *ceil_body = ast_new_block();
    ASTNode *ceil_int_part = ast_new_to_int(ast_new_var("x"));
    ASTNode *ceil_is_positive = ast_new_binop(ast_new_var("x"), ast_new_number(0), TOK_GT);
    ASTNode *ceil_has_decimal = ast_new_binop(ast_new_var("x"), ceil_int_part, TOK_NEQ);
    ASTNode *ceil_both_cond = ast_new_and(ceil_is_positive, ceil_has_decimal);
    ASTNode *ceil_add = ast_new_binop(ceil_int_part, ast_new_number(1), TOK_PLUS);
    ASTNode *ceil_result = ast_new_ternary(ceil_both_cond, ceil_add, ceil_int_part);
    ast_block_add_statement(ceil_body, ceil_result);
    register_function("ceil", ceil_params, 1, ceil_result);
    
    // round(x) - rounding function
    char round_params[4][64] = {"x"};
    ASTNode *round_body = ast_new_block();
    ASTNode *round_plus_half = ast_new_binop(ast_new_var("x"), ast_new_number(0.5), TOK_PLUS);
    ASTNode *round_result = ast_new_to_int(round_plus_half);
    ast_block_add_statement(round_body, round_result);
    register_function("round", round_params, 1, round_body);
    
    // sign(x) - sign function (-1, 0, or 1)
    char sign_params[4][64] = {"x"};
    ASTNode *sign_body = ast_new_block();
    ASTNode *sign_is_zero = ast_new_binop(ast_new_var("x"), ast_new_number(0), TOK_EQ);
    ASTNode *sign_is_positive = ast_new_binop(ast_new_var("x"), ast_new_number(0), TOK_GT);
    ASTNode *sign_zero_or_pos = ast_new_ternary(sign_is_positive, ast_new_number(1), ast_new_number(-1));
    ASTNode *sign_result = ast_new_ternary(sign_is_zero, ast_new_number(0), sign_zero_or_pos);
    ast_block_add_statement(sign_body, sign_result);
    register_function("sign", sign_params, 1, sign_body);
    
    // clamp(x, min, max) - clamp value between min and max
    char clamp_params[4][64] = {"x", "min", "max"};
    ASTNode *clamp_body = ast_new_block();
    ASTNode *clamp_too_low = ast_new_binop(ast_new_var("x"), ast_new_var("min"), TOK_LT);
    ASTNode *clamp_too_high = ast_new_binop(ast_new_var("x"), ast_new_var("max"), TOK_GT);
    ASTNode *clamp_low_result = ast_new_ternary(clamp_too_low, ast_new_var("min"), ast_new_var("x"));
    ASTNode *clamp_result = ast_new_ternary(clamp_too_high, ast_new_var("max"), clamp_low_result);
    ast_block_add_statement(clamp_body, clamp_result);
    register_function("clamp", clamp_params, 3, clamp_body);
    
    // lerp(a, b, t) - linear interpolation
    char lerp_params[4][64] = {"a", "b", "t"};
    ASTNode *lerp_body = ast_new_block();
    ASTNode *lerp_diff = ast_new_binop(ast_new_var("b"), ast_new_var("a"), TOK_MINUS);
    ASTNode *lerp_mult = ast_new_binop(lerp_diff, ast_new_var("t"), TOK_MUL);
    ASTNode *lerp_result = ast_new_binop(ast_new_var("a"), lerp_mult, TOK_PLUS);
    ast_block_add_statement(lerp_body, lerp_result);
    register_function("lerp", lerp_params, 3, lerp_body);
}

static void initialize_packages() {
    if (!packages_initialized) {
        init_date_time_package();
        init_math_utils_package();
        init_string_utils_package();
        init_algorithms_package();
        init_crypto_utils_package();
        init_file_utils_package();
        init_json_utils_package();
        init_network_utils_package();
        init_random_utils_package();
        init_system_utils_package();
        init_database_package();
        init_console_utils_package();
        init_time_package();
        initialize_builtin_functions();
        packages_initialized = 1;
    }
}

void interpret(ASTNode *root)
{
    initialize_packages();
    if (!root)
        return;
    
    if (debug_mode) {
        printf("[DEBUG] Interpreting node type: %d\n", root->type);
    }
    if (root->type == NODE_BLOCK)
    {
        for (int i = 0; i < root->block.count; i++)
        {
            if (root->block.statements[i]) // Only interpret non-NULL statements
            {
                interpret(root->block.statements[i]);
                // Check for break/continue flags and propagate them up
                if (break_flag || continue_flag)
                {
                    return;
                }
            }
        }
        return;
    }
    else if (root->type == NODE_ASSIGN)
    {
        if (debug_mode) {
            printf("[DEBUG] Assignment: %s\n", root->assign.varname);
        }
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
        else if (value_node->type == NODE_SET)
        {
            set_set_variable(root->assign.varname, value_node);
        }
        else if (value_node->type == NODE_TREE)
        {
            set_tree_variable(root->assign.varname, value_node);
        }
        else if (value_node->type == NODE_GRAPH)
        {
            set_graph_variable(root->assign.varname, value_node);
        }
        else if (value_node->type == NODE_UNDEF)
        {
            set_undef_variable(root->assign.varname);
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
        else if (value_node->type == NODE_TYPE)
        {
            double result = eval_expression(value_node);
            if (result == -12345.6789)
            {
                const char *str_result = get_variable("__type_result");
                if (str_result)
                {
                    set_variable(root->assign.varname, str_result);
                }
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
        else if (value_node->type == NODE_FUNC_CALL)
        {
            // Handle function calls in assignments
            double result = eval_expression(value_node);
            
            // Check if the function returned a string
            const char *str_result = get_variable("__function_return_str");
            if (str_result)
            {
                set_variable(root->assign.varname, str_result);
                // Clear the temporary variable
                set_variable("__function_return_str", "");
            }
            else
            {
                char buf[64];
                snprintf(buf, sizeof(buf), "%g", result);
                set_variable(root->assign.varname, buf);
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
        else if (value_node->type == NODE_ITERATOR)
        {
            // Handle iterator assignment
            interpret(value_node); // This will create the iterator and store it in __last_iterator
            Iterator *iter = get_iterator_variable("__last_iterator");
            if (iter)
            {
                set_iterator_variable(root->assign.varname, iter);
                // Clear the temporary variable
                set_iterator_variable("__last_iterator", NULL);
            }
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
    else if (root->type == NODE_COMPOUND_ASSIGN)
    {
        // Handle compound assignment operators (+=, -=, *=, /=, %=)
        const char *current_val = get_variable(root->compound_assign.varname);
        double current_num = current_val ? strtod(current_val, NULL) : 0.0;
        double new_val = eval_expression(root->compound_assign.value);
        double result;
        
        switch (root->compound_assign.op)
        {
        case TOK_PLUS_ASSIGN:
            result = current_num + new_val;
            break;
        case TOK_MINUS_ASSIGN:
            result = current_num - new_val;
            break;
        case TOK_MUL_ASSIGN:
            result = current_num * new_val;
            break;
        case TOK_DIV_ASSIGN:
            if (new_val == 0)
            {
                error_throw_at_line(ERROR_DIVISION_BY_ZERO, "Division by zero in compound assignment", root->line);
            }
            result = current_num / new_val;
            break;
        case TOK_MOD_ASSIGN:
            result = fmod(current_num, new_val);
            break;
        default:
            {
                char error_msg[256];
                snprintf(error_msg, sizeof(error_msg), "Unknown compound assignment operator %d", root->compound_assign.op);
                error_throw_at_line(ERROR_RUNTIME, error_msg, root->line);
            }
        }
        
        // Store the result back in the variable
        char buf[64];
        snprintf(buf, sizeof(buf), "%g", result);
        
        // Check if this is a temporal variable
        TemporalVariable *temp_var = get_temporal_var_struct(root->compound_assign.varname);
        if (temp_var)
        {
            set_temporal_variable(root->compound_assign.varname, buf, temp_var->max_history);
        }
        else
        {
            set_variable(root->compound_assign.varname, buf);
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
        if (debug_mode) {
            printf("[DEBUG] Print statement\n");
        }
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
        if (debug_mode) {
            printf("[DEBUG] If statement\n");
        }
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
            error_throw_at_line(ERROR_RUNTIME, "Loop increment cannot be zero", root->line);
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
                
                if (break_flag)
                {
                    break_flag = 0;
                    break;
                }
                if (continue_flag)
                {
                    continue_flag = 0;
                    continue;
                }
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
                
                if (break_flag)
                {
                    break_flag = 0;
                    break;
                }
                if (continue_flag)
                {
                    continue_flag = 0;
                    continue;
                }
            }
        }
    }
    else if (root->type == NODE_WHILE)
    {
        while (eval_expression(root->while_stmt.condition) != 0)
        {
            interpret(root->while_stmt.body);
            
            if (break_flag)
            {
                break_flag = 0;
                break;
            }
            if (continue_flag)
            {
                continue_flag = 0;
                continue;
            }
        }
    }
    else if (root->type == NODE_FOREACH)
    {
        ASTNode *iterable_node = root->foreach_stmt.iterable;
        
        // Handle variable reference to list
        if (iterable_node->type == NODE_VAR)
        {
            ASTNode *list = get_list_variable(iterable_node->varname);
            if (list && list->type == NODE_LIST)
            {
                // Iterate through list elements
                for (int i = 0; i < list->list.count; i++)
                {
                    ASTNode *element = list->list.elements[i];
                    if (element->type == NODE_STRING)
                    {
                        set_variable(root->foreach_stmt.varname, element->string);
                    }
                    else if (element->type == NODE_NUMBER)
                    {
                        char buf[64];
                        snprintf(buf, sizeof(buf), "%g", element->number);
                        set_variable(root->foreach_stmt.varname, buf);
                    }
                    else if (element->type == NODE_LIST)
                    {
                        // For list elements, store the list as a variable
                        set_list_variable(root->foreach_stmt.varname, element);
                    }
                    interpret(root->foreach_stmt.body);
                    
                    if (break_flag)
                    {
                        break_flag = 0;
                        break;
                    }
                    if (continue_flag)
                    {
                        continue_flag = 0;
                        continue;
                    }
                }
            }
            else
            {
                error_throw_at_line(ERROR_TYPE_MISMATCH, "foreach expects a list variable", root->line);
            }
        }
        // Handle direct list literal
        else if (iterable_node->type == NODE_LIST)
        {
            for (int i = 0; i < iterable_node->list.count; i++)
            {
                ASTNode *element = iterable_node->list.elements[i];
                if (element->type == NODE_STRING)
                {
                    set_variable(root->foreach_stmt.varname, element->string);
                }
                else if (element->type == NODE_NUMBER)
                {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "%g", element->number);
                    set_variable(root->foreach_stmt.varname, buf);
                }
                else if (element->type == NODE_LIST)
                {
                    // For list elements, store the list as a variable
                    set_list_variable(root->foreach_stmt.varname, element);
                }
                interpret(root->foreach_stmt.body);
                
                if (break_flag)
                {
                    break_flag = 0;
                    break;
                }
                if (continue_flag)
                {
                    continue_flag = 0;
                    continue;
                }
            }
        }
        else
        {
            error_throw_at_line(ERROR_TYPE_MISMATCH, "foreach expects a list", root->line);
        }
    }
    else if (root->type == NODE_TEMPORAL_LOOP)
    {
        TemporalVariable *temp_var = get_temporal_var_struct(root->temporal_loop.temporal_var);
        if (!temp_var)
        {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), "Variable '%s' is not a temporal variable", root->temporal_loop.temporal_var);
            error_throw_at_line(ERROR_TYPE_MISMATCH, error_msg, root->line);
        }
        
        // Iterate through the temporal history
        for (int i = 0; i < temp_var->count; i++)
        {
            // Set the loop variable to current history entry
            set_variable(root->temporal_loop.varname, temp_var->history[i].value);
            interpret(root->temporal_loop.body);
            
            if (break_flag)
            {
                break_flag = 0;
                break;
            }
            if (continue_flag)
            {
                continue_flag = 0;
                continue;
            }
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
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), "Could not open file '%s'", root->string);
            error_throw_at_line(ERROR_FILE_NOT_FOUND, error_msg, root->line);
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
    else if (root->type == NODE_GENERATOR)
    {
        register_generator(root->generator.name, root->generator.params, root->generator.param_count, root->generator.body);
    }
    else if (root->type == NODE_ITERATOR)
    {
        // Create iterator from generator call
        if (root->iterator.generator_call->type == NODE_FUNC_CALL)
        {
            ASTNode *gen_call = root->iterator.generator_call;
            Generator *gen = find_generator(gen_call->func_call.name);
            if (!gen)
            {
                printf("Runtime error: Generator '%s' not found\n", gen_call->func_call.name);
                exit(1);
            }
            
            Iterator *iter = create_iterator(gen, gen_call->func_call.args, gen_call->func_call.arg_count);
            if (!iter)
            {
                printf("Runtime error: Failed to create iterator\n");
                exit(1);
            }
            
            // Store iterator in a temporary variable for assignment
            set_iterator_variable("__last_iterator", iter);
        }
        else
        {
            printf("Runtime error: Iterator expects a generator call\n");
            exit(1);
        }
    }
    else if (root->type == NODE_YIELD)
    {
        // Yield statements are handled during generator execution
        // For now, just evaluate the value
        eval_expression(root->yield_stmt.value);
    }
    else if (root->type == NODE_NEXT)
    {
        // Get next value from iterator
        if (root->next_stmt.iterator->type == NODE_VAR)
        {
            Iterator *iter = get_iterator_variable(root->next_stmt.iterator->varname);
            if (!iter)
            {
                printf("Runtime error: Variable '%s' is not an iterator\n", root->next_stmt.iterator->varname);
                exit(1);
            }
            
            ASTNode *next_value = iterator_next(iter);
            if (next_value)
            {
                if (next_value->type == NODE_STRING)
                {
                    printf("%s\n", next_value->string);
                }
                else if (next_value->type == NODE_NUMBER)
                {
                    printf("%g\n", next_value->number);
                }
                else
                {
                    double val = eval_expression(next_value);
                    printf("%g\n", val);
                }
            }
            else
            {
                printf("Iterator exhausted\n");
            }
        }
        else
        {
            printf("Runtime error: next$ expects an iterator variable\n");
            exit(1);
        }
    }
    else if (root->type == NODE_FUNC_CALL)
    {
        if (debug_mode) {
            printf("[DEBUG] Function call: %s\n", root->func_call.name);
        }
        
        // Try package functions first
        ASTNode *package_result = call_package_function(root->func_call.name, root->func_call.args, root->func_call.arg_count);
        if (package_result) {
            // Package function found and executed
            return;
        }
        
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
             root->type == NODE_REGEX_FIND_ALL || root->type == NODE_TREE ||
             root->type == NODE_TREE_INSERT || root->type == NODE_TREE_SEARCH ||
             root->type == NODE_TREE_DELETE || root->type == NODE_TREE_INORDER ||
             root->type == NODE_TREE_PREORDER || root->type == NODE_TREE_POSTORDER ||
             root->type == NODE_GRAPH || root->type == NODE_GRAPH_ADD_VERTEX ||
             root->type == NODE_GRAPH_ADD_EDGE || root->type == NODE_GRAPH_REMOVE_VERTEX ||
             root->type == NODE_GRAPH_REMOVE_EDGE || root->type == NODE_GRAPH_HAS_EDGE ||
             root->type == NODE_GRAPH_NEIGHBORS || root->type == NODE_GRAPH_DFS ||
             root->type == NODE_GRAPH_BFS || root->type == NODE_SET_UNION ||
             root->type == NODE_SET_INTERSECTION || root->type == NODE_SET_DIFFERENCE ||
             root->type == NODE_SET_SYMMETRIC_DIFF || root->type == NODE_SET_ADD ||
             root->type == NODE_SET_REMOVE || root->type == NODE_SET_CONTAINS ||
             root->type == NODE_SET_SIZE || root->type == NODE_SET_EMPTY ||
             root->type == NODE_SET_CLEAR || root->type == NODE_SET_COPY)
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
    else if (root->type == NODE_TRY)
    {
        // Save current exception state
        int prev_exception_active = exception_active;
        jmp_buf prev_env;
        if (exception_active) {
            memcpy(prev_env, exception_env, sizeof(jmp_buf));
        }
        
        exception_active = 1;
        
        if (setjmp(exception_env) == 0) {
            // Execute try block
            interpret(root->try_stmt.try_body);
        } else {
            // Exception occurred, find matching catch block
            int handled = 0;
            for (int i = 0; i < root->try_stmt.catch_count; i++) {
                ASTNode *catch_block = root->try_stmt.catch_blocks[i];
                // For now, catch all exceptions (could add type matching later)
                set_variable(catch_block->catch_stmt.variable_name, current_error.message);
                interpret(catch_block->catch_stmt.catch_body);
                handled = 1;
                break;
            }
            
            if (!handled && prev_exception_active) {
                // Re-throw if not handled and there's an outer handler
                memcpy(exception_env, prev_env, sizeof(jmp_buf));
                longjmp(exception_env, 1);
            }
        }
        
        // Execute finally block if present
        if (root->try_stmt.finally_block) {
            interpret(root->try_stmt.finally_block);
        }
        
        // Restore previous exception state
        exception_active = prev_exception_active;
        if (prev_exception_active) {
            memcpy(exception_env, prev_env, sizeof(jmp_buf));
        }
    }
    else if (root->type == NODE_THROW)
    {
        char *error_msg = "Custom exception";
        if (root->throw_stmt.exception_expr->type == NODE_STRING) {
            error_msg = root->throw_stmt.exception_expr->string;
        }
        THROW(ERROR_CUSTOM, error_msg);
    }
    else if (root->type == NODE_FILE_OPEN || root->type == NODE_FILE_READ ||
             root->type == NODE_FILE_WRITE || root->type == NODE_FILE_CLOSE ||
             root->type == NODE_TO_STR || root->type == NODE_TO_INT ||
             root->type == NODE_HTTP_GET || root->type == NODE_HTTP_POST ||
             root->type == NODE_HTTP_PUT || root->type == NODE_HTTP_DELETE ||
             root->type == NODE_TEMPORAL_AGGREGATE || root->type == NODE_TEMPORAL_PATTERN ||
             root->type == NODE_TEMPORAL_CONDITION || root->type == NODE_SLIDING_WINDOW_STATS ||
             root->type == NODE_SENSITIVITY_THRESHOLD || root->type == NODE_TEMPORAL_QUERY ||
             root->type == NODE_TEMPORAL_CORRELATE || root->type == NODE_TEMPORAL_INTERPOLATE ||
             root->type == NODE_STRING_SPLIT || root->type == NODE_STRING_JOIN ||
             root->type == NODE_STRING_REPLACE || root->type == NODE_STRING_SUBSTRING ||
             root->type == NODE_STRING_LENGTH || root->type == NODE_STRING_UPPER ||
             root->type == NODE_STRING_LOWER || root->type == NODE_RANDOM)
    {
        eval_expression(root);
    }
    else if (root->type == NODE_BREAK)
    {
        break_flag = 1;
    }
    else if (root->type == NODE_CONTINUE)
    {
        continue_flag = 1;
    }
    else if (root->type == NODE_INCREMENT)
    {
        const char *val = get_variable(root->inc_dec.varname);
        double current = val ? strtod(val, NULL) : 0.0;
        current += 1.0;
        char buf[64];
        snprintf(buf, sizeof(buf), "%g", current);
        set_variable(root->inc_dec.varname, buf);
    }
    else if (root->type == NODE_DECREMENT)
    {
        const char *val = get_variable(root->inc_dec.varname);
        double current = val ? strtod(val, NULL) : 0.0;
        current -= 1.0;
        char buf[64];
        snprintf(buf, sizeof(buf), "%g", current);
        set_variable(root->inc_dec.varname, buf);
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
        error_throw_at_line(ERROR_RUNTIME, "Null AST node in eval", 0);
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
        
        // Check if it's an UNDEF variable
        if (is_undef_variable(node->varname))
        {
            return 0.0; // UNDEF evaluates to 0
        }
        
        const char *val = get_variable(node->varname);
        if (!val)
        {
            return 0.0; // Undefined variables are treated as UNDEF (0)
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
                error_throw_at_line(ERROR_DIVISION_BY_ZERO, "Division by zero", node->line);
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
            {
                char error_msg[256];
                snprintf(error_msg, sizeof(error_msg), "Unknown binary operator %d", node->binop.op);
                error_throw_at_line(ERROR_RUNTIME, error_msg, node->line);
            }
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
            error_throw_at_line(ERROR_TYPE_MISMATCH, "List access only supported on list nodes", node->line);
        }

        if (i < 0 || i >= list_node->list.count)
        {
            error_throw_at_line(ERROR_INDEX_OUT_OF_BOUNDS, "List index out of bounds", node->line);
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
            error_throw_at_line(ERROR_TYPE_MISMATCH, "Unsupported list element type", node->line);
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
            error_throw_at_line(ERROR_TYPE_MISMATCH, "len() expects a list", node->line);
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
            error_throw_at_line(ERROR_TYPE_MISMATCH, "append() expects a list", node->line);
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
            error_throw_at_line(ERROR_TYPE_MISMATCH, "prepend() expects a list", node->line);
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
            error_throw_at_line(ERROR_TYPE_MISMATCH, "pop() expects a non-empty list", node->line);
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
            error_throw_at_line(ERROR_TYPE_MISMATCH, "insert() expects a list", node->line);
        }

        int index = (int)eval_expression(index_node);
        if (index < 0 || index > list_node->list.count)
        {
            error_throw_at_line(ERROR_INDEX_OUT_OF_BOUNDS, "Index out of bounds in insert()", node->line);
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
            error_throw_at_line(ERROR_TYPE_MISMATCH, "remove() expects a list", node->line);
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
            error_throw_at_line(ERROR_RUNTIME, "Value not found in list", node->line);
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
            error_throw_at_line(ERROR_TYPE_MISMATCH, "get() expects a dictionary", node->line);
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
        error_throw_at_line(ERROR_RUNTIME, "Key not found in dictionary", node->line);
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
            error_throw_at_line(ERROR_TYPE_MISMATCH, "set() expects a dictionary", node->line);
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
            error_throw_at_line(ERROR_TYPE_MISMATCH, "keys() expects a dictionary", node->line);
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
            error_throw_at_line(ERROR_TYPE_MISMATCH, "values() expects a dictionary", node->line);
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
            error_throw_at_line(ERROR_RUNTIME, "Member access on non-object", node->line);
        }

        FieldEntry *field = object_get_field(obj, member_name);
        if (!field)
        {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), "Field '%s' not found", member_name);
            error_throw_at_line(ERROR_RUNTIME, error_msg, node->line);
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
            error_throw_at_line(ERROR_TYPE_MISMATCH, "push() expects a stack", node->line);
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
            error_throw_at_line(ERROR_TYPE_MISMATCH, "pop() expects a non-empty stack", node->line);
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
            error_throw_at_line(ERROR_TYPE_MISMATCH, "peek() expects a non-empty stack", node->line);
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
            error_throw_at_line(ERROR_TYPE_MISMATCH, "size() expects a stack", node->line);
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
            error_throw_at_line(ERROR_TYPE_MISMATCH, "empty() expects a stack", node->line);
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
    case NODE_SET:
        print_node(node);
        return 0;
    case NODE_SET_UNION:
    {
        ASTNode *set1_node = node->set_binop.set1;
        ASTNode *set2_node = node->set_binop.set2;
        
        if (set1_node->type == NODE_VAR)
        {
            set1_node = get_set_variable(set1_node->varname);
        }
        if (set2_node->type == NODE_VAR)
        {
            set2_node = get_set_variable(set2_node->varname);
        }
        
        if (!set1_node || !set2_node || set1_node->type != NODE_SET || set2_node->type != NODE_SET)
        {
            printf("Runtime error: Union operation expects two sets\n");
            exit(1);
        }
        
        ASTNode *result = ast_new_set();
        
        // Add all elements from set1
        for (int i = 0; i < set1_node->set.count; i++)
        {
            ast_set_add_element(result, set1_node->set.elements[i]);
        }
        
        // Add all elements from set2 (duplicates will be filtered by ast_set_add_element)
        for (int i = 0; i < set2_node->set.count; i++)
        {
            ast_set_add_element(result, set2_node->set.elements[i]);
        }
        
        print_node(result);
        ast_free(result);
        return 0;
    }
    case NODE_SET_INTERSECTION:
    {
        ASTNode *set1_node = node->set_binop.set1;
        ASTNode *set2_node = node->set_binop.set2;
        
        if (set1_node->type == NODE_VAR)
        {
            set1_node = get_set_variable(set1_node->varname);
        }
        if (set2_node->type == NODE_VAR)
        {
            set2_node = get_set_variable(set2_node->varname);
        }
        
        if (!set1_node || !set2_node || set1_node->type != NODE_SET || set2_node->type != NODE_SET)
        {
            printf("Runtime error: Intersection operation expects two sets\n");
            exit(1);
        }
        
        ASTNode *result = ast_new_set();
        
        // Add elements that exist in both sets
        for (int i = 0; i < set1_node->set.count; i++)
        {
            ASTNode *elem1 = set1_node->set.elements[i];
            for (int j = 0; j < set2_node->set.count; j++)
            {
                ASTNode *elem2 = set2_node->set.elements[j];
                if (elem1->type == elem2->type)
                {
                    if ((elem1->type == NODE_NUMBER && elem1->number == elem2->number) ||
                        (elem1->type == NODE_STRING && strcmp(elem1->string, elem2->string) == 0))
                    {
                        ast_set_add_element(result, elem1);
                        break;
                    }
                }
            }
        }
        
        print_node(result);
        ast_free(result);
        return 0;
    }
    case NODE_SET_DIFFERENCE:
    {
        ASTNode *set1_node = node->set_binop.set1;
        ASTNode *set2_node = node->set_binop.set2;
        
        if (set1_node->type == NODE_VAR)
        {
            set1_node = get_set_variable(set1_node->varname);
        }
        if (set2_node->type == NODE_VAR)
        {
            set2_node = get_set_variable(set2_node->varname);
        }
        
        if (!set1_node || !set2_node || set1_node->type != NODE_SET || set2_node->type != NODE_SET)
        {
            printf("Runtime error: Difference operation expects two sets\n");
            exit(1);
        }
        
        ASTNode *result = ast_new_set();
        
        // Add elements from set1 that are not in set2
        for (int i = 0; i < set1_node->set.count; i++)
        {
            ASTNode *elem1 = set1_node->set.elements[i];
            int found = 0;
            for (int j = 0; j < set2_node->set.count; j++)
            {
                ASTNode *elem2 = set2_node->set.elements[j];
                if (elem1->type == elem2->type)
                {
                    if ((elem1->type == NODE_NUMBER && elem1->number == elem2->number) ||
                        (elem1->type == NODE_STRING && strcmp(elem1->string, elem2->string) == 0))
                    {
                        found = 1;
                        break;
                    }
                }
            }
            if (!found)
            {
                ASTNode *elem_copy = (elem1->type == NODE_NUMBER) ? ast_new_number(elem1->number) : ast_new_string(elem1->string);
                ast_set_add_element(result, elem_copy);
            }
        }
        
        print_node(result);
        ast_free(result);
        return 0;
    }
    case NODE_SET_SYMMETRIC_DIFF:
    {
        ASTNode *set1_node = node->set_binop.set1;
        ASTNode *set2_node = node->set_binop.set2;
        
        if (set1_node->type == NODE_VAR)
        {
            set1_node = get_set_variable(set1_node->varname);
        }
        if (set2_node->type == NODE_VAR)
        {
            set2_node = get_set_variable(set2_node->varname);
        }
        
        if (!set1_node || !set2_node || set1_node->type != NODE_SET || set2_node->type != NODE_SET)
        {
            printf("Runtime error: Symmetric difference operation expects two sets\n");
            exit(1);
        }
        
        ASTNode *result = ast_new_set();
        
        // Add elements from set1 that are not in set2
        for (int i = 0; i < set1_node->set.count; i++)
        {
            ASTNode *elem1 = set1_node->set.elements[i];
            int found = 0;
            for (int j = 0; j < set2_node->set.count; j++)
            {
                ASTNode *elem2 = set2_node->set.elements[j];
                if (elem1->type == elem2->type)
                {
                    if ((elem1->type == NODE_NUMBER && elem1->number == elem2->number) ||
                        (elem1->type == NODE_STRING && strcmp(elem1->string, elem2->string) == 0))
                    {
                        found = 1;
                        break;
                    }
                }
            }
            if (!found)
            {
                ast_set_add_element(result, elem1);
            }
        }
        
        // Add elements from set2 that are not in set1
        for (int i = 0; i < set2_node->set.count; i++)
        {
            ASTNode *elem2 = set2_node->set.elements[i];
            int found = 0;
            for (int j = 0; j < set1_node->set.count; j++)
            {
                ASTNode *elem1 = set1_node->set.elements[j];
                if (elem1->type == elem2->type)
                {
                    if ((elem1->type == NODE_NUMBER && elem1->number == elem2->number) ||
                        (elem1->type == NODE_STRING && strcmp(elem1->string, elem2->string) == 0))
                    {
                        found = 1;
                        break;
                    }
                }
            }
            if (!found)
            {
                ast_set_add_element(result, elem2);
            }
        }
        
        print_node(result);
        ast_free(result);
        return 0;
    }
    case NODE_SET_ADD:
    {
        ASTNode *set_node = node->set_element_op.set;
        ASTNode *element = node->set_element_op.element;
        
        if (set_node->type == NODE_VAR)
        {
            set_node = get_set_variable(set_node->varname);
        }
        
        if (!set_node || set_node->type != NODE_SET)
        {
            printf("Runtime error: Set add operation expects a set\n");
            exit(1);
        }
        
        ast_set_add_element(set_node, element);
        return 0;
    }
    case NODE_SET_REMOVE:
    {
        ASTNode *set_node = node->set_element_op.set;
        ASTNode *element = node->set_element_op.element;
        
        if (set_node->type == NODE_VAR)
        {
            set_node = get_set_variable(set_node->varname);
        }
        
        if (!set_node || set_node->type != NODE_SET)
        {
            printf("Runtime error: Set remove operation expects a set\n");
            exit(1);
        }
        
        // Find and remove the element
        for (int i = 0; i < set_node->set.count; i++)
        {
            ASTNode *elem = set_node->set.elements[i];
            if (elem->type == element->type)
            {
                if ((elem->type == NODE_NUMBER && elem->number == element->number) ||
                    (elem->type == NODE_STRING && strcmp(elem->string, element->string) == 0))
                {
                    // Shift elements to fill the gap
                    for (int j = i; j < set_node->set.count - 1; j++)
                    {
                        set_node->set.elements[j] = set_node->set.elements[j + 1];
                    }
                    set_node->set.count--;
                    return 1; // Element removed
                }
            }
        }
        return 0; // Element not found
    }
    case NODE_SET_CONTAINS:
    {
        ASTNode *set_node = node->set_element_op.set;
        ASTNode *element = node->set_element_op.element;
        
        if (set_node->type == NODE_VAR)
        {
            set_node = get_set_variable(set_node->varname);
        }
        
        if (!set_node || set_node->type != NODE_SET)
        {
            printf("Runtime error: Set contains operation expects a set\n");
            exit(1);
        }
        
        // Check if element exists in set
        for (int i = 0; i < set_node->set.count; i++)
        {
            ASTNode *elem = set_node->set.elements[i];
            if (elem->type == element->type)
            {
                if ((elem->type == NODE_NUMBER && elem->number == element->number) ||
                    (elem->type == NODE_STRING && strcmp(elem->string, element->string) == 0))
                {
                    return 1; // Element found
                }
            }
        }
        return 0; // Element not found
    }
    case NODE_SET_SIZE:
    {
        ASTNode *set_node = node->set_op.set;
        
        if (set_node->type == NODE_VAR)
        {
            set_node = get_set_variable(set_node->varname);
        }
        
        if (!set_node || set_node->type != NODE_SET)
        {
            printf("Runtime error: Set size operation expects a set\n");
            exit(1);
        }
        
        return set_node->set.count;
    }
    case NODE_SET_EMPTY:
    {
        ASTNode *set_node = node->set_op.set;
        
        if (set_node->type == NODE_VAR)
        {
            set_node = get_set_variable(set_node->varname);
        }
        
        if (!set_node || set_node->type != NODE_SET)
        {
            printf("Runtime error: Set empty operation expects a set\n");
            exit(1);
        }
        
        return set_node->set.count == 0 ? 1 : 0;
    }
    case NODE_SET_CLEAR:
    {
        ASTNode *set_node = node->set_op.set;
        
        if (set_node->type == NODE_VAR)
        {
            set_node = get_set_variable(set_node->varname);
        }
        
        if (!set_node || set_node->type != NODE_SET)
        {
            printf("Runtime error: Set clear operation expects a set\n");
            exit(1);
        }
        
        // Clear elements array only
        set_node->set.count = 0;
        
        return 0;
    }
    case NODE_SET_COPY:
    {
        ASTNode *set_node = node->set_op.set;
        
        if (set_node->type == NODE_VAR)
        {
            set_node = get_set_variable(set_node->varname);
        }
        
        if (!set_node || set_node->type != NODE_SET)
        {
            printf("Runtime error: Set copy operation expects a set\n");
            exit(1);
        }
        
        // Create a new set and copy all elements
        ASTNode *new_set = ast_new_set();
        for (int i = 0; i < set_node->set.count; i++)
        {
            // Create copies of the elements
            ASTNode *elem_copy;
            if (set_node->set.elements[i]->type == NODE_NUMBER)
            {
                elem_copy = ast_new_number(set_node->set.elements[i]->number);
            }
            else if (set_node->set.elements[i]->type == NODE_STRING)
            {
                elem_copy = ast_new_string(set_node->set.elements[i]->string);
            }
            else
            {
                // For other types, just reference the same node (shallow copy)
                elem_copy = set_node->set.elements[i];
            }
            ast_set_add_element(new_set, elem_copy);
        }
        
        print_node(new_set);
        ast_free(new_set);
        return 0;
    }
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
    case NODE_TEMPORAL_CONDITION:
    {
        TemporalVariable *temp_var = get_temporal_var_struct(node->temporal_condition.varname);
        if (!temp_var)
        {
            printf("Runtime error: Variable '%s' is not a temporal variable\n", node->temporal_condition.varname);
            exit(1);
        }
        
        int start_index = (int)eval_expression(node->temporal_condition.start_index);
        int window_size = (int)eval_expression(node->temporal_condition.window_size);
        const char *condition = node->temporal_condition.condition;
        
        // Validate indices
        if (start_index < 0 || start_index >= temp_var->count)
            return 0; // Invalid start index
        if (window_size <= 0 || start_index + window_size > temp_var->count)
            window_size = temp_var->count - start_index; // Adjust window size
        
        // Parse condition string
        if (strncmp(condition, "> ", 2) == 0)
        {
            double threshold = strtod(condition + 2, NULL);
            for (int i = start_index; i < start_index + window_size; i++)
            {
                double val = strtod(temp_var->history[temp_var->count - 1 - i].value, NULL);
                if (val <= threshold) return 0;
            }
            return 1;
        }
        else if (strncmp(condition, "< ", 2) == 0)
        {
            double threshold = strtod(condition + 2, NULL);
            for (int i = start_index; i < start_index + window_size; i++)
            {
                double val = strtod(temp_var->history[temp_var->count - 1 - i].value, NULL);
                if (val >= threshold) return 0;
            }
            return 1;
        }
        else if (strncmp(condition, "== ", 3) == 0)
        {
            double target = strtod(condition + 3, NULL);
            for (int i = start_index; i < start_index + window_size; i++)
            {
                double val = strtod(temp_var->history[temp_var->count - 1 - i].value, NULL);
                if (val == target) return 1; // Any match returns true
            }
            return 0;
        }
        else if (strncmp(condition, "between ", 8) == 0)
        {
            char *rest = (char *)(condition + 8);
            double min_val = strtod(rest, &rest);
            while (*rest == ' ') rest++; // Skip spaces
            double max_val = strtod(rest, NULL);
            
            for (int i = start_index; i < start_index + window_size; i++)
            {
                double val = strtod(temp_var->history[temp_var->count - 1 - i].value, NULL);
                if (val < min_val || val > max_val) return 0;
            }
            return 1;
        }
        else if (strcmp(condition, "increasing") == 0)
        {
            if (window_size < 2) return 0;
            for (int i = start_index; i < start_index + window_size - 1; i++)
            {
                double curr = strtod(temp_var->history[temp_var->count - 1 - i].value, NULL);
                double next = strtod(temp_var->history[temp_var->count - 1 - (i + 1)].value, NULL);
                if (curr <= next) return 0; // Not strictly increasing
            }
            return 1;
        }
        else if (strncmp(condition, "stable ", 7) == 0)
        {
            double variance_threshold = strtod(condition + 7, NULL);
            if (window_size < 2) return 1;
            
            // Calculate variance
            double sum = 0, mean, variance = 0;
            for (int i = start_index; i < start_index + window_size; i++)
            {
                sum += strtod(temp_var->history[temp_var->count - 1 - i].value, NULL);
            }
            mean = sum / window_size;
            
            for (int i = start_index; i < start_index + window_size; i++)
            {
                double val = strtod(temp_var->history[temp_var->count - 1 - i].value, NULL);
                variance += (val - mean) * (val - mean);
            }
            variance /= window_size;
            
            return variance < variance_threshold ? 1 : 0;
        }
        else
        {
            printf("Runtime error: Unknown temporal condition '%s'\n", condition);
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
    case NODE_SLIDING_WINDOW_STATS:
    {
        TemporalVariable *temp_var = get_temporal_var_struct(node->sliding_window_stats.varname);
        if (!temp_var)
        {
            printf("Runtime error: Variable '%s' is not a temporal variable\n", node->sliding_window_stats.varname);
            exit(1);
        }
        
        int window_size = (int)eval_expression(node->sliding_window_stats.window_size);
        if (window_size <= 0 || window_size > temp_var->count)
            window_size = temp_var->count;
        
        const char *stat_type = node->sliding_window_stats.stat_type;
        
        if (strcmp(stat_type, "variance") == 0)
        {
            double sum = 0, mean, variance = 0;
            for (int i = temp_var->count - window_size; i < temp_var->count; i++)
            {
                sum += strtod(temp_var->history[i].value, NULL);
            }
            mean = sum / window_size;
            
            for (int i = temp_var->count - window_size; i < temp_var->count; i++)
            {
                double val = strtod(temp_var->history[i].value, NULL);
                variance += (val - mean) * (val - mean);
            }
            return variance / window_size;
        }
        else if (strcmp(stat_type, "stddev") == 0)
        {
            double sum = 0, mean, variance = 0;
            for (int i = temp_var->count - window_size; i < temp_var->count; i++)
            {
                sum += strtod(temp_var->history[i].value, NULL);
            }
            mean = sum / window_size;
            
            for (int i = temp_var->count - window_size; i < temp_var->count; i++)
            {
                double val = strtod(temp_var->history[i].value, NULL);
                variance += (val - mean) * (val - mean);
            }
            return sqrt(variance / window_size);
        }
        else if (strcmp(stat_type, "range") == 0)
        {
            double min_val = strtod(temp_var->history[temp_var->count - window_size].value, NULL);
            double max_val = min_val;
            
            for (int i = temp_var->count - window_size + 1; i < temp_var->count; i++)
            {
                double val = strtod(temp_var->history[i].value, NULL);
                if (val < min_val) min_val = val;
                if (val > max_val) max_val = val;
            }
            return max_val - min_val;
        }
        else if (strcmp(stat_type, "median") == 0)
        {
            double values[window_size];
            for (int i = 0; i < window_size; i++)
            {
                values[i] = strtod(temp_var->history[temp_var->count - window_size + i].value, NULL);
            }
            
            // Simple bubble sort
            for (int i = 0; i < window_size - 1; i++)
            {
                for (int j = 0; j < window_size - i - 1; j++)
                {
                    if (values[j] > values[j + 1])
                    {
                        double temp = values[j];
                        values[j] = values[j + 1];
                        values[j + 1] = temp;
                    }
                }
            }
            
            if (window_size % 2 == 0)
            {
                return (values[window_size / 2 - 1] + values[window_size / 2]) / 2.0;
            }
            else
            {
                return values[window_size / 2];
            }
        }
        else
        {
            printf("Runtime error: Unknown statistic type '%s'\n", stat_type);
            exit(1);
        }
        
        return 0;
    }
    case NODE_SENSITIVITY_THRESHOLD:
    {
        TemporalVariable *temp_var = get_temporal_var_struct(node->sensitivity_threshold.varname);
        if (!temp_var)
        {
            printf("Runtime error: Variable '%s' is not a temporal variable\n", node->sensitivity_threshold.varname);
            exit(1);
        }
        
        if (temp_var->count == 0)
        {
            return 0; // No data to check
        }
        
        double threshold_value = eval_expression(node->sensitivity_threshold.threshold_value);
        double sensitivity_percent = eval_expression(node->sensitivity_threshold.sensitivity_percent);
        
        // Get current value
        double current_value = strtod(temp_var->history[temp_var->count - 1].value, NULL);
        
        // Calculate upper and lower bounds
        double upper_bound = threshold_value + (threshold_value * sensitivity_percent / 100.0);
        double lower_bound = threshold_value - (threshold_value * sensitivity_percent / 100.0);
        
        if (current_value > upper_bound)
        {
            return 1; // Above threshold + sensitivity
        }
        else if (current_value < lower_bound)
        {
            return -1; // Below threshold - sensitivity
        }
        else
        {
            return 0; // Within acceptable range
        }
    }
    case NODE_TEMPORAL_QUERY:
    {
        TemporalVariable *temp_var = get_temporal_var_struct(node->temporal_query.varname);
        if (!temp_var)
        {
            printf("Runtime error: Variable '%s' is not a temporal variable\n", node->temporal_query.varname);
            exit(1);
        }
        
        const char *time_window = node->temporal_query.time_window;
        const char *condition = node->temporal_query.condition;
        
        // Simple time window parsing
        if (strncmp(time_window, "last ", 5) == 0)
        {
            int count = atoi(time_window + 5);
            if (count <= 0 || count > temp_var->count) count = temp_var->count;
            
            // Check condition on last N values
            int matches = 0;
            for (int i = temp_var->count - count; i < temp_var->count; i++)
            {
                double val = strtod(temp_var->history[i].value, NULL);
                
                if (strncmp(condition, "> ", 2) == 0)
                {
                    double threshold = strtod(condition + 2, NULL);
                    if (val > threshold) matches++;
                }
                else if (strncmp(condition, "< ", 2) == 0)
                {
                    double threshold = strtod(condition + 2, NULL);
                    if (val < threshold) matches++;
                }
                else if (strncmp(condition, "== ", 3) == 0)
                {
                    double target = strtod(condition + 3, NULL);
                    if (val == target) matches++;
                }
            }
            return matches;
        }
        else if (strncmp(time_window, "between ", 8) == 0)
        {
            // Simple between parsing - check all history
            int matches = 0;
            for (int i = 0; i < temp_var->count; i++)
            {
                double val = strtod(temp_var->history[i].value, NULL);
                
                if (strncmp(condition, "> ", 2) == 0)
                {
                    double threshold = strtod(condition + 2, NULL);
                    if (val > threshold) matches++;
                }
                else if (strncmp(condition, "< ", 2) == 0)
                {
                    double threshold = strtod(condition + 2, NULL);
                    if (val < threshold) matches++;
                }
                else if (strncmp(condition, "== ", 3) == 0)
                {
                    double target = strtod(condition + 3, NULL);
                    if (val == target) matches++;
                }
            }
            return matches;
        }
        
        return 0;
    }
    case NODE_TEMPORAL_CORRELATE:
    {
        TemporalVariable *var1 = get_temporal_var_struct(node->temporal_correlate.var1);
        TemporalVariable *var2 = get_temporal_var_struct(node->temporal_correlate.var2);
        
        if (!var1 || !var2)
        {
            printf("Runtime error: One or both variables are not temporal variables\n");
            exit(1);
        }
        
        int window_size = (int)eval_expression(node->temporal_correlate.window_size);
        if (window_size <= 0) window_size = 5; // default
        
        int min_count = (var1->count < var2->count) ? var1->count : var2->count;
        if (window_size > min_count) window_size = min_count;
        
        if (window_size < 2) return 0; // Need at least 2 points for correlation
        
        // Calculate Pearson correlation coefficient
        double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0, sum_y2 = 0;
        
        for (int i = 0; i < window_size; i++)
        {
            double x = strtod(var1->history[var1->count - window_size + i].value, NULL);
            double y = strtod(var2->history[var2->count - window_size + i].value, NULL);
            
            sum_x += x;
            sum_y += y;
            sum_xy += x * y;
            sum_x2 += x * x;
            sum_y2 += y * y;
        }
        
        double n = window_size;
        double numerator = n * sum_xy - sum_x * sum_y;
        double denominator = sqrt((n * sum_x2 - sum_x * sum_x) * (n * sum_y2 - sum_y * sum_y));
        
        if (denominator == 0) return 0; // No correlation if denominator is 0
        
        return numerator / denominator;
    }
    case NODE_TEMPORAL_INTERPOLATE:
    {
        TemporalVariable *temp_var = get_temporal_var_struct(node->temporal_interpolate.varname);
        if (!temp_var)
        {
            printf("Runtime error: Variable '%s' is not a temporal variable\n", node->temporal_interpolate.varname);
            exit(1);
        }
        
        int missing_index = (int)eval_expression(node->temporal_interpolate.missing_index);
        
        if (missing_index < 0 || missing_index >= temp_var->count)
        {
            printf("Runtime error: Invalid index for interpolation\n");
            exit(1);
        }
        
        // Simple linear interpolation
        if (missing_index == 0)
        {
            // Use next value if first is missing
            if (temp_var->count > 1)
                return strtod(temp_var->history[1].value, NULL);
            return strtod(temp_var->history[0].value, NULL);
        }
        else if (missing_index == temp_var->count - 1)
        {
            // Use previous value if last is missing
            return strtod(temp_var->history[missing_index - 1].value, NULL);
        }
        else
        {
            // Return the actual value at the index (not interpolating missing data)
            return strtod(temp_var->history[missing_index].value, NULL);
        }
    }
    case NODE_LAMBDA:
    {
        // Lambdas are treated as anonymous functions
        // For now, just return 0 (could store lambda in a variable later)
        return 0;
    }
    case NODE_STRING_INTERPOLATION:
    {
        char result[1024] = "";
        char *template = node->string_interp.template;
        int expr_idx = 0;
        
        for (int i = 0; template[i]; i++)
        {
            if (template[i] == '$' && i + 1 < strlen(template) && template[i+1] == '{')
            {
                // Find the end of the expression
                int j = i + 2;
                while (j < strlen(template) && template[j] != '}') j++;
                
                if (expr_idx < node->string_interp.expr_count)
                {
                    const char *val = get_variable(node->string_interp.expressions[expr_idx]->varname);
                    if (val) strcat(result, val);
                    expr_idx++;
                }
                
                i = j; // Skip to after }, will be incremented by loop
            }
            else
            {
                char temp[2] = {template[i], '\0'};
                strcat(result, temp);
            }
        }
        
        // Store result for print_node
        set_variable("__string_interp_result", result);
        return 0;
    }
    case NODE_TYPE:
    {
        ASTNode *value = node->type_check.value;
        const char *type_name = "unknown";
        
        if (value->type == NODE_VAR)
        {
            // Check variable type
            if (is_undef_variable(value->varname))
            {
                type_name = "undef";
            }
            else if (get_list_variable(value->varname))
            {
                type_name = "list";
            }
            else if (get_dict_variable(value->varname))
            {
                type_name = "dict";
            }
            else if (get_stack_variable(value->varname))
            {
                type_name = "stack";
            }
            else if (get_queue_variable(value->varname))
            {
                type_name = "queue";
            }
            else if (get_linked_list_variable(value->varname))
            {
                type_name = "linked_list";
            }
            else if (get_regex_variable(value->varname))
            {
                type_name = "regex";
            }
            else if (get_set_variable(value->varname))
            {
                type_name = "set";
            }
            else if (get_tree_variable(value->varname))
            {
                type_name = "tree";
            }
            else if (get_graph_variable(value->varname))
            {
                type_name = "graph";
            }
            else if (get_temporal_var_struct(value->varname))
            {
                type_name = "temporal";
            }
            else
            {
                const char *str_val = get_variable(value->varname);
                if (str_val)
                {
                    // Check if it's a number or string
                    char *endptr;
                    strtod(str_val, &endptr);
                    if (endptr != str_val && *endptr == '\0')
                    {
                        type_name = "number";
                    }
                    else
                    {
                        type_name = "string";
                    }
                }
                else
                {
                    type_name = "undef";
                }
            }
        }
        else if (value->type == NODE_NUMBER)
        {
            type_name = "number";
        }
        else if (value->type == NODE_STRING)
        {
            type_name = "string";
        }
        else if (value->type == NODE_LIST)
        {
            type_name = "list";
        }
        else if (value->type == NODE_DICT)
        {
            type_name = "dict";
        }
        else if (value->type == NODE_STACK)
        {
            type_name = "stack";
        }
        else if (value->type == NODE_QUEUE)
        {
            type_name = "queue";
        }
        else if (value->type == NODE_LINKED_LIST)
        {
            type_name = "linked_list";
        }
        else if (value->type == NODE_REGEX)
        {
            type_name = "regex";
        }
        else if (value->type == NODE_SET)
        {
            type_name = "set";
        }
        else if (value->type == NODE_TREE)
        {
            type_name = "tree";
        }
        else if (value->type == NODE_GRAPH)
        {
            type_name = "graph";
        }
        else if (value->type == NODE_UNDEF)
        {
            type_name = "undef";
        }
        
        // Store result for string return
        set_variable("__type_result", type_name);
        return -12345.6789; // Special marker for string return
    }
    case NODE_UNDEF:
    {
        return 0; // UNDEF evaluates to 0
    }
    case NODE_STRING_SPLIT:
    {
        char *string_val = get_string_value(node->string_split.string);
        char *delimiter_val = get_string_value(node->string_split.delimiter);
        
        if (!string_val || !delimiter_val) {
            if (string_val) free(string_val);
            if (delimiter_val) free(delimiter_val);
            error_throw_at_line(ERROR_RUNTIME, "Invalid arguments for split", node->line);
        }
        
        ASTNode *result_list = ast_new_list();
        
        if (strlen(delimiter_val) == 0) {
            // Split into individual characters
            for (int i = 0; string_val[i]; i++) {
                char char_str[2] = {string_val[i], '\0'};
                ast_list_add_element(result_list, ast_new_string(char_str));
            }
        } else {
            char *str_copy = strdup(string_val);
            char *token = strtok(str_copy, delimiter_val);
            while (token) {
                ast_list_add_element(result_list, ast_new_string(token));
                token = strtok(NULL, delimiter_val);
            }
            free(str_copy);
        }
        
        // Store result in a temporary variable for printing
        char *list_str = list_to_string(result_list);
        printf("%s\n", list_str);
        free(list_str);
        
        free(string_val);
        free(delimiter_val);
        ast_free(result_list);
        return 0;
    }
    case NODE_STRING_JOIN:
    {
        ASTNode *list_node = node->string_join.list;
        if (list_node->type == NODE_VAR) {
            list_node = get_list_variable(list_node->varname);
        }
        
        if (!list_node || list_node->type != NODE_LIST) {
            error_throw_at_line(ERROR_TYPE_MISMATCH, "join() expects a list", node->line);
        }
        
        char *separator = get_string_value(node->string_join.separator);
        if (!separator) {
            error_throw_at_line(ERROR_RUNTIME, "Invalid separator for join", node->line);
        }
        
        char result[1024] = "";
        for (int i = 0; i < list_node->list.count; i++) {
            if (list_node->list.elements[i]->type == NODE_STRING) {
                strcat(result, list_node->list.elements[i]->string);
            } else if (list_node->list.elements[i]->type == NODE_NUMBER) {
                char num_str[64];
                snprintf(num_str, sizeof(num_str), "%g", list_node->list.elements[i]->number);
                strcat(result, num_str);
            }
            if (i < list_node->list.count - 1) {
                strcat(result, separator);
            }
        }
        
        printf("%s\n", result);
        set_variable("__string_result", result);
        
        free(separator);
        return 0;
    }
    case NODE_STRING_REPLACE:
    {
        char *string_val = get_string_value(node->string_replace.string);
        char *old_str = get_string_value(node->string_replace.old_str);
        char *new_str = get_string_value(node->string_replace.new_str);
        
        if (!string_val || !old_str || !new_str) {
            if (string_val) free(string_val);
            if (old_str) free(old_str);
            if (new_str) free(new_str);
            error_throw_at_line(ERROR_RUNTIME, "Invalid arguments for replace", node->line);
        }
        
        char result[1024] = "";
        char *pos = string_val;
        char *found;
        
        while ((found = strstr(pos, old_str)) != NULL) {
            // Copy part before the match
            int len = found - pos;
            strncat(result, pos, len);
            // Add replacement
            strcat(result, new_str);
            // Move past the match
            pos = found + strlen(old_str);
        }
        // Copy remaining part
        strcat(result, pos);
        
        printf("%s\n", result);
        set_variable("__string_result", result);
        
        free(string_val);
        free(old_str);
        free(new_str);
        return 0;
    }
    case NODE_STRING_SUBSTRING:
    {
        char *string_val = get_string_value(node->string_substring.string);
        if (!string_val) {
            error_throw_at_line(ERROR_RUNTIME, "Invalid string for substring", node->line);
        }
        
        int start = (int)eval_expression(node->string_substring.start);
        int length = (int)eval_expression(node->string_substring.length);
        int str_len = strlen(string_val);
        
        if (start < 0 || start >= str_len) {
            free(string_val);
            error_throw_at_line(ERROR_INDEX_OUT_OF_BOUNDS, "Substring start index out of bounds", node->line);
        }
        
        if (length < 0 || start + length > str_len) {
            length = str_len - start;
        }
        
        char result[1024];
        strncpy(result, string_val + start, length);
        result[length] = '\0';
        
        printf("%s\n", result);
        set_variable("__string_result", result);
        
        free(string_val);
        return 0;
    }
    case NODE_STRING_LENGTH:
    {
        char *string_val = get_string_value(node->string_op.string);
        if (!string_val) {
            error_throw_at_line(ERROR_RUNTIME, "Invalid string for length", node->line);
        }
        
        int length = strlen(string_val);
        printf("%d\n", length);
        free(string_val);
        return length;
    }
    case NODE_STRING_UPPER:
    {
        char *string_val = get_string_value(node->string_op.string);
        if (!string_val) {
            error_throw_at_line(ERROR_RUNTIME, "Invalid string for upper", node->line);
        }
        
        char result[1024];
        strcpy(result, string_val);
        for (int i = 0; result[i]; i++) {
            result[i] = toupper(result[i]);
        }
        
        printf("%s\n", result);
        set_variable("__string_result", result);
        
        free(string_val);
        return 0;
    }
    case NODE_STRING_LOWER:
    {
        char *string_val = get_string_value(node->string_op.string);
        if (!string_val) {
            error_throw_at_line(ERROR_RUNTIME, "Invalid string for lower", node->line);
        }
        
        char result[1024];
        strcpy(result, string_val);
        for (int i = 0; result[i]; i++) {
            result[i] = tolower(result[i]);
        }
        
        printf("%s\n", result);
        set_variable("__string_result", result);
        
        free(string_val);
        return 0;
    }
    case NODE_RANDOM:
    {
        double start = eval_expression(node->random_op.start);
        double end = eval_expression(node->random_op.end);
        double increment = 1.0;
        
        if (node->random_op.increment) {
            increment = eval_expression(node->random_op.increment);
        }
        
        if (increment <= 0) {
            error_throw_at_line(ERROR_RUNTIME, "Random increment must be positive", node->line);
        }
        
        if (start > end) {
            double temp = start;
            start = end;
            end = temp;
        }
        
        // Calculate number of possible values
        int num_values = (int)((end - start) / increment) + 1;
        
        // Generate random index
        int random_index = rand() % num_values;
        
        // Calculate the random value
        double result = start + (random_index * increment);
        
        printf("%g\n", result);
        return result;
    }
    case NODE_INCREMENT:
    {
        const char *val = get_variable(node->inc_dec.varname);
        double current = val ? strtod(val, NULL) : 0.0;
        if (node->inc_dec.is_prefix)
        {
            current += 1.0;
            char buf[64];
            snprintf(buf, sizeof(buf), "%g", current);
            set_variable(node->inc_dec.varname, buf);
            return current;
        }
        else
        {
            char buf[64];
            snprintf(buf, sizeof(buf), "%g", current + 1.0);
            set_variable(node->inc_dec.varname, buf);
            return current;
        }
    }
    case NODE_DECREMENT:
    {
        const char *val = get_variable(node->inc_dec.varname);
        double current = val ? strtod(val, NULL) : 0.0;
        if (node->inc_dec.is_prefix)
        {
            current -= 1.0;
            char buf[64];
            snprintf(buf, sizeof(buf), "%g", current);
            set_variable(node->inc_dec.varname, buf);
            return current;
        }
        else
        {
            char buf[64];
            snprintf(buf, sizeof(buf), "%g", current - 1.0);
            set_variable(node->inc_dec.varname, buf);
            return current;
        }
    }
    case NODE_FUNC_CALL:
    {
        // Try package functions first
        ASTNode *package_result = call_package_function(node->func_call.name, node->func_call.args, node->func_call.arg_count);
        if (package_result) {
            if (package_result->type == NODE_NUMBER) {
                return package_result->number;
            } else {
                return eval_expression(package_result);
            }
        }
        
        Function *fn = find_function(node->func_call.name);
        if (!fn)
        {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), "Undefined function '%s'", node->func_call.name);
            error_throw_at_line(ERROR_UNDEFINED_VARIABLE, error_msg, node->line);
        }
        if (fn->param_count != node->func_call.arg_count)
        {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), "Function '%s' expects %d args but got %d",
                   node->func_call.name, fn->param_count, node->func_call.arg_count);
            error_throw_at_line(ERROR_RUNTIME, error_msg, node->line);
        }

        // Save current variable state to restore later
        // This is a simplified approach - in a full implementation you'd want proper scoping
        
        // Set function parameters
        for (int i = 0; i < fn->param_count; i++)
        {
            ASTNode *arg = node->func_call.args[i];
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

        // Execute function body and capture the last expression result
        // In Tesseract, the last expression in a function is the return value
        if (fn->body->type == NODE_BLOCK && fn->body->block.count > 0)
        {
            // Execute all statements except the last one
            for (int i = 0; i < fn->body->block.count - 1; i++)
            {
                if (fn->body->block.statements[i])
                    interpret(fn->body->block.statements[i]);
            }
            
            // Evaluate the last statement as the return value
            ASTNode *last_stmt = fn->body->block.statements[fn->body->block.count - 1];
            if (last_stmt)
            {
                if (last_stmt->type == NODE_VAR)
                {
                    // If it's a variable, get its value
                    const char *val = get_variable(last_stmt->varname);
                    if (val)
                    {
                        // Store the return value for string results
                        set_variable("__function_return_str", val);
                        char *endptr;
                        double dval = strtod(val, &endptr);
                        if (endptr == val)
                            return 0.0;
                        return dval;
                    }
                }
                else if (last_stmt->type == NODE_STRING)
                {
                    set_variable("__function_return_str", last_stmt->string);
                    return 0.0; // String return
                }
                else
                {
                    return eval_expression(last_stmt);
                }
            }
        }
        else
        {
            // Single statement function
            if (fn->body->type == NODE_VAR)
            {
                const char *val = get_variable(fn->body->varname);
                if (val)
                {
                    set_variable("__function_return_str", val);
                    char *endptr;
                    double dval = strtod(val, &endptr);
                    if (endptr == val)
                        return 0.0;
                    return dval;
                }
            }
            else
            {
                return eval_expression(fn->body);
            }
        }
        
        return 0; // Default return value
    }
    case NODE_TREE:
        print_node(node);
        return 0;
    case NODE_TREE_INSERT:
    {
        ASTNode *tree_node = node->tree_insert.tree;
        if (tree_node->type == NODE_VAR)
        {
            tree_node = get_tree_variable(tree_node->varname);
        }
        if (tree_node && tree_node->type == NODE_TREE)
        {
            ast_tree_add_element(tree_node, node->tree_insert.value);
        }
        return 0;
    }
    case NODE_TREE_SEARCH:
    {
        ASTNode *tree_node = node->tree_search.tree;
        if (tree_node->type == NODE_VAR)
        {
            tree_node = get_tree_variable(tree_node->varname);
        }
        if (tree_node && tree_node->type == NODE_TREE)
        {
            double search_val = eval_expression(node->tree_search.value);
            for (int i = 0; i < tree_node->tree.count; i++)
            {
                if (tree_node->tree.elements[i]->type == NODE_NUMBER &&
                    tree_node->tree.elements[i]->number == search_val)
                {
                    return 1; // Found
                }
            }
        }
        return 0; // Not found
    }
    case NODE_TREE_DELETE:
    {
        ASTNode *tree_node = node->tree_delete.tree;
        if (tree_node->type == NODE_VAR)
        {
            tree_node = get_tree_variable(tree_node->varname);
        }
        if (tree_node && tree_node->type == NODE_TREE)
        {
            double delete_val = eval_expression(node->tree_delete.value);
            for (int i = 0; i < tree_node->tree.count; i++)
            {
                if (tree_node->tree.elements[i]->type == NODE_NUMBER &&
                    tree_node->tree.elements[i]->number == delete_val)
                {
                    for (int j = i; j < tree_node->tree.count - 1; j++)
                    {
                        tree_node->tree.elements[j] = tree_node->tree.elements[j + 1];
                    }
                    tree_node->tree.count--;
                    return 1; // Deleted
                }
            }
        }
        return 0; // Not found
    }
    case NODE_TREE_INORDER:
    {
        ASTNode *tree_node = node->tree_traversal.tree;
        if (tree_node->type == NODE_VAR)
        {
            tree_node = get_tree_variable(tree_node->varname);
        }
        if (tree_node && tree_node->type == NODE_TREE)
        {
            // Sort elements for inorder traversal (BST property)
            double values[tree_node->tree.count];
            int count = 0;
            for (int i = 0; i < tree_node->tree.count; i++)
            {
                if (tree_node->tree.elements[i]->type == NODE_NUMBER)
                {
                    values[count++] = tree_node->tree.elements[i]->number;
                }
            }
            // Simple bubble sort
            for (int i = 0; i < count - 1; i++)
            {
                for (int j = 0; j < count - i - 1; j++)
                {
                    if (values[j] > values[j + 1])
                    {
                        double temp = values[j];
                        values[j] = values[j + 1];
                        values[j + 1] = temp;
                    }
                }
            }
            printf("[");
            for (int i = 0; i < count; i++)
            {
                printf("%g", values[i]);
                if (i < count - 1)
                    printf(", ");
            }
            printf("]\n");
        }
        return 0;
    }
    case NODE_TREE_PREORDER:
    {
        ASTNode *tree_node = node->tree_traversal.tree;
        if (tree_node->type == NODE_VAR)
        {
            tree_node = get_tree_variable(tree_node->varname);
        }
        if (tree_node && tree_node->type == NODE_TREE)
        {
            // Preorder: root, left subtree, right subtree
            // For simplicity, assume first element is root, then left subtree, then right subtree
            printf("[");
            if (tree_node->tree.count > 0)
            {
                // Print root first (element at index 0 - the first inserted)
                if (tree_node->tree.elements[0]->type == NODE_NUMBER)
                    printf("%g", tree_node->tree.elements[0]->number);
                
                // Print remaining elements
                for (int i = 1; i < tree_node->tree.count; i++)
                {
                    printf(", ");
                    if (tree_node->tree.elements[i]->type == NODE_NUMBER)
                        printf("%g", tree_node->tree.elements[i]->number);
                }
            }
            printf("]\n");
        }
        return 0;
    }
    case NODE_TREE_POSTORDER:
    {
        ASTNode *tree_node = node->tree_traversal.tree;
        if (tree_node->type == NODE_VAR)
        {
            tree_node = get_tree_variable(tree_node->varname);
        }
        if (tree_node && tree_node->type == NODE_TREE)
        {
            // Postorder: left subtree, right subtree, root
            // Print all elements except root, then root last
            printf("[");
            if (tree_node->tree.count > 1)
            {
                // Print all elements except the first (root)
                for (int i = 1; i < tree_node->tree.count; i++)
                {
                    if (tree_node->tree.elements[i]->type == NODE_NUMBER)
                        printf("%g", tree_node->tree.elements[i]->number);
                    if (i < tree_node->tree.count - 1)
                        printf(", ");
                }
                if (tree_node->tree.count > 1) printf(", ");
            }
            // Print root last
            if (tree_node->tree.count > 0 && tree_node->tree.elements[0]->type == NODE_NUMBER)
                printf("%g", tree_node->tree.elements[0]->number);
            printf("]\n");
        }
        return 0;
    }
    case NODE_GRAPH:
        print_node(node);
        return 0;
    case NODE_GRAPH_ADD_VERTEX:
    {
        ASTNode *graph_node = node->graph_vertex_op.graph;
        if (graph_node->type == NODE_VAR)
        {
            graph_node = get_graph_variable(graph_node->varname);
        }
        if (graph_node && graph_node->type == NODE_GRAPH)
        {
            ast_graph_add_vertex(graph_node, node->graph_vertex_op.vertex);
        }
        return 0;
    }
    case NODE_GRAPH_ADD_EDGE:
    {
        ASTNode *graph_node = node->graph_edge_op.graph;
        if (graph_node->type == NODE_VAR)
        {
            graph_node = get_graph_variable(graph_node->varname);
        }
        if (graph_node && graph_node->type == NODE_GRAPH)
        {
            ast_graph_add_edge(graph_node, node->graph_edge_op.from, node->graph_edge_op.to);
        }
        return 0;
    }
    case NODE_GRAPH_REMOVE_VERTEX:
    {
        ASTNode *graph_node = node->graph_vertex_op.graph;
        if (graph_node->type == NODE_VAR)
        {
            graph_node = get_graph_variable(graph_node->varname);
        }
        if (graph_node && graph_node->type == NODE_GRAPH)
        {
            double vertex_val = eval_expression(node->graph_vertex_op.vertex);
            for (int i = 0; i < graph_node->graph.vertex_count; i++)
            {
                if (graph_node->graph.vertices[i]->type == NODE_NUMBER &&
                    graph_node->graph.vertices[i]->number == vertex_val)
                {
                    for (int j = i; j < graph_node->graph.vertex_count - 1; j++)
                    {
                        graph_node->graph.vertices[j] = graph_node->graph.vertices[j + 1];
                    }
                    graph_node->graph.vertex_count--;
                    return 1;
                }
            }
        }
        return 0;
    }
    case NODE_GRAPH_HAS_EDGE:
    {
        ASTNode *graph_node = node->graph_edge_op.graph;
        if (graph_node->type == NODE_VAR)
        {
            graph_node = get_graph_variable(graph_node->varname);
        }
        if (graph_node && graph_node->type == NODE_GRAPH)
        {
            double from_val = eval_expression(node->graph_edge_op.from);
            double to_val = eval_expression(node->graph_edge_op.to);
            for (int i = 0; i < graph_node->graph.edge_count; i++)
            {
                ASTNode *edge = graph_node->graph.edges[i];
                if (edge->type == NODE_LIST && edge->list.count >= 2)
                {
                    double edge_from = eval_expression(edge->list.elements[0]);
                    double edge_to = eval_expression(edge->list.elements[1]);
                    if (edge_from == from_val && edge_to == to_val)
                    {
                        return 1;
                    }
                }
            }
        }
        return 0;
    }
    case NODE_GRAPH_NEIGHBORS:
    {
        ASTNode *graph_node = node->graph_neighbors.graph;
        if (graph_node->type == NODE_VAR)
        {
            graph_node = get_graph_variable(graph_node->varname);
        }
        if (graph_node && graph_node->type == NODE_GRAPH)
        {
            double vertex_val = eval_expression(node->graph_neighbors.vertex);
            printf("[");
            int first = 1;
            for (int i = 0; i < graph_node->graph.edge_count; i++)
            {
                ASTNode *edge = graph_node->graph.edges[i];
                if (edge->type == NODE_LIST && edge->list.count >= 2)
                {
                    double edge_from = eval_expression(edge->list.elements[0]);
                    double edge_to = eval_expression(edge->list.elements[1]);
                    
                    // Check both directions for undirected graph
                    if (edge_from == vertex_val)
                    {
                        if (!first) printf(", ");
                        printf("%g", edge_to);
                        first = 0;
                    }
                    else if (edge_to == vertex_val)
                    {
                        if (!first) printf(", ");
                        printf("%g", edge_from);
                        first = 0;
                    }
                }
            }
            printf("]\n");
        }
        return 0;
    }
    case NODE_GRAPH_DFS:
    {
        ASTNode *graph_node = node->graph_traversal.graph;
        if (graph_node->type == NODE_VAR)
        {
            graph_node = get_graph_variable(graph_node->varname);
        }
        if (graph_node && graph_node->type == NODE_GRAPH)
        {
            double start_vertex = eval_expression(node->graph_traversal.start);
            int visited[graph_node->graph.vertex_count];
            double result[graph_node->graph.vertex_count];
            int result_count = 0;
            
            // Initialize visited array
            for (int i = 0; i < graph_node->graph.vertex_count; i++)
                visited[i] = 0;
            
            // Simple DFS implementation using recursion simulation with stack
            double stack[graph_node->graph.vertex_count];
            int stack_top = 0;
            stack[stack_top++] = start_vertex;
            
            while (stack_top > 0)
            {
                double current = stack[--stack_top];
                
                // Find vertex index
                int vertex_idx = -1;
                for (int i = 0; i < graph_node->graph.vertex_count; i++)
                {
                    if (graph_node->graph.vertices[i]->type == NODE_NUMBER &&
                        graph_node->graph.vertices[i]->number == current)
                    {
                        vertex_idx = i;
                        break;
                    }
                }
                
                if (vertex_idx != -1 && !visited[vertex_idx])
                {
                    visited[vertex_idx] = 1;
                    result[result_count++] = current;
                    
                    // Add neighbors to stack (in reverse order for correct DFS order)
                    for (int i = graph_node->graph.edge_count - 1; i >= 0; i--)
                    {
                        ASTNode *edge = graph_node->graph.edges[i];
                        if (edge->type == NODE_LIST && edge->list.count >= 2)
                        {
                            double edge_from = eval_expression(edge->list.elements[0]);
                            double edge_to = eval_expression(edge->list.elements[1]);
                            
                            if (edge_from == current)
                            {
                                // Check if neighbor is not visited
                                int neighbor_idx = -1;
                                for (int j = 0; j < graph_node->graph.vertex_count; j++)
                                {
                                    if (graph_node->graph.vertices[j]->type == NODE_NUMBER &&
                                        graph_node->graph.vertices[j]->number == edge_to)
                                    {
                                        neighbor_idx = j;
                                        break;
                                    }
                                }
                                if (neighbor_idx != -1 && !visited[neighbor_idx])
                                {
                                    stack[stack_top++] = edge_to;
                                }
                            }
                        }
                    }
                }
            }
            
            printf("[");
            for (int i = 0; i < result_count; i++)
            {
                printf("%g", result[i]);
                if (i < result_count - 1)
                    printf(", ");
            }
            printf("]\n");
        }
        return 0;
    }
    case NODE_GRAPH_BFS:
    {
        ASTNode *graph_node = node->graph_traversal.graph;
        if (graph_node->type == NODE_VAR)
        {
            graph_node = get_graph_variable(graph_node->varname);
        }
        if (graph_node && graph_node->type == NODE_GRAPH)
        {
            double start_vertex = eval_expression(node->graph_traversal.start);
            int visited[graph_node->graph.vertex_count];
            double result[graph_node->graph.vertex_count];
            int result_count = 0;
            
            // Initialize visited array
            for (int i = 0; i < graph_node->graph.vertex_count; i++)
                visited[i] = 0;
            
            // BFS implementation using queue
            double queue[graph_node->graph.vertex_count];
            int queue_front = 0, queue_rear = 0;
            queue[queue_rear++] = start_vertex;
            
            // Mark start vertex as visited
            for (int i = 0; i < graph_node->graph.vertex_count; i++)
            {
                if (graph_node->graph.vertices[i]->type == NODE_NUMBER &&
                    graph_node->graph.vertices[i]->number == start_vertex)
                {
                    visited[i] = 1;
                    break;
                }
            }
            
            while (queue_front < queue_rear)
            {
                double current = queue[queue_front++];
                result[result_count++] = current;
                
                // Add neighbors to queue
                for (int i = 0; i < graph_node->graph.edge_count; i++)
                {
                    ASTNode *edge = graph_node->graph.edges[i];
                    if (edge->type == NODE_LIST && edge->list.count >= 2)
                    {
                        double edge_from = eval_expression(edge->list.elements[0]);
                        double edge_to = eval_expression(edge->list.elements[1]);
                        
                        if (edge_from == current)
                        {
                            // Check if neighbor is not visited
                            int neighbor_idx = -1;
                            for (int j = 0; j < graph_node->graph.vertex_count; j++)
                            {
                                if (graph_node->graph.vertices[j]->type == NODE_NUMBER &&
                                    graph_node->graph.vertices[j]->number == edge_to)
                                {
                                    neighbor_idx = j;
                                    break;
                                }
                            }
                            if (neighbor_idx != -1 && !visited[neighbor_idx])
                            {
                                visited[neighbor_idx] = 1;
                                queue[queue_rear++] = edge_to;
                            }
                        }
                    }
                }
            }
            
            printf("[");
            for (int i = 0; i < result_count; i++)
            {
                printf("%g", result[i]);
                if (i < result_count - 1)
                    printf(", ");
            }
            printf("]\n");
        }
        return 0;
    }
    default:
        {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), "Unsupported AST node type %d", node->type);
            error_throw_at_line(ERROR_RUNTIME, error_msg, node->line);
        }
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
    case NODE_SET:
    {
        printf("{");
        for (int i = 0; i < node->set.count; i++)
        {
            if (node->set.elements[i]->type == NODE_STRING)
                printf("%s", node->set.elements[i]->string);
            else if (node->set.elements[i]->type == NODE_NUMBER)
                printf("%g", node->set.elements[i]->number);
            if (i < node->set.count - 1)
                printf(", ");
        }
        printf("}\n");
        break;
    }
    case NODE_TREE:
    {
        printf("<tree: ");
        for (int i = 0; i < node->tree.count; i++)
        {
            if (node->tree.elements[i]->type == NODE_STRING)
                printf("%s", node->tree.elements[i]->string);
            else if (node->tree.elements[i]->type == NODE_NUMBER)
                printf("%g", node->tree.elements[i]->number);
            if (i < node->tree.count - 1)
                printf(", ");
        }
        printf(">\n");
        break;
    }
    case NODE_GRAPH:
    {
        printf("<graph: vertices[");
        for (int i = 0; i < node->graph.vertex_count; i++)
        {
            if (node->graph.vertices[i]->type == NODE_STRING)
                printf("%s", node->graph.vertices[i]->string);
            else if (node->graph.vertices[i]->type == NODE_NUMBER)
                printf("%g", node->graph.vertices[i]->number);
            if (i < node->graph.vertex_count - 1)
                printf(", ");
        }
        printf("] edges[");
        for (int i = 0; i < node->graph.edge_count; i++)
        {
            ASTNode *edge = node->graph.edges[i];
            if (edge->type == NODE_LIST && edge->list.count >= 2)
            {
                printf("(");
                if (edge->list.elements[0]->type == NODE_NUMBER)
                    printf("%g", edge->list.elements[0]->number);
                else if (edge->list.elements[0]->type == NODE_STRING)
                    printf("%s", edge->list.elements[0]->string);
                printf(",");
                if (edge->list.elements[1]->type == NODE_NUMBER)
                    printf("%g", edge->list.elements[1]->number);
                else if (edge->list.elements[1]->type == NODE_STRING)
                    printf("%s", edge->list.elements[1]->string);
                printf(")");
            }
            if (i < node->graph.edge_count - 1)
                printf(", ");
        }
        printf("]>\n");
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
        // Check if it's a set variable
        else if (get_set_variable(node->varname))
        {
            ASTNode *set = get_set_variable(node->varname);
            print_node(set);
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
        // Check if it's a set variable
        else if (get_set_variable(node->varname))
        {
            ASTNode *set = get_set_variable(node->varname);
            print_node(set);
        }
        // Check if it's a tree variable
        else if (get_tree_variable(node->varname))
        {
            ASTNode *tree = get_tree_variable(node->varname);
            print_node(tree);
        }
        // Check if it's a graph variable
        else if (get_graph_variable(node->varname))
        {
            ASTNode *graph = get_graph_variable(node->varname);
            print_node(graph);
        }
        // Check if it's an UNDEF variable
        else if (is_undef_variable(node->varname))
        {
            printf("UNDEF\n");
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
                printf("UNDEF\n"); // Undefined variables are UNDEF
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
    case NODE_TYPE:
    {
        double result = eval_expression(node);
        if (result == -12345.6789)
        {
            const char *str_result = get_variable("__type_result");
            if (str_result)
            {
                printf("%s\n", str_result);
            }
        }
        break;
    }
    case NODE_UNDEF:
    {
        printf("UNDEF\n");
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
    case NODE_STRING_INTERPOLATION:
    {
        eval_expression(node);
        const char *result = get_variable("__string_interp_result");
        if (result) {
            printf("%s\n", result);
        }
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
             node->type == NODE_TEMPORAL_CONDITION ||
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

    // Also treat tree and linked list operations as boolean operations
    bool is_tree_or_list_op = node->binop.left->type == NODE_TREE_SEARCH ||
                              node->binop.left->type == NODE_TREE_DELETE ||
                              node->binop.left->type == NODE_LINKED_LIST_ISEMPTY ||
                              node->binop.left->type == NODE_STACK_EMPTY ||
                              node->binop.left->type == NODE_QUEUE_ISEMPTY ||
                              node->binop.left->type == NODE_GRAPH_HAS_EDGE;

    if ((is_bool_op || is_comp_op || is_tree_or_list_op) && (value == 0 || value == 1))
    {
        printf("%s\n", bool_to_str((bool)value));
    }
    else
    {
        printf("%g\n", value);
    }
}