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

static char *skip_whitespace(char *str) {
    while (*str && isspace(*str)) str++;
    return str;
}

static ASTNode *parse_json_value(char **str);

static ASTNode *parse_json_string(char **str) {
    char *s = *str;
    if (*s != '"') return NULL;
    s++;
    
    char buffer[256] = {0};
    int pos = 0;
    
    while (*s && *s != '"' && pos < 255) {
        if (*s == '\\' && *(s+1)) {
            s++;
            switch (*s) {
                case 'n': buffer[pos++] = '\n'; break;
                case 't': buffer[pos++] = '\t'; break;
                case 'r': buffer[pos++] = '\r'; break;
                case '\\': buffer[pos++] = '\\'; break;
                case '"': buffer[pos++] = '"'; break;
                default: buffer[pos++] = *s; break;
            }
        } else {
            buffer[pos++] = *s;
        }
        s++;
    }
    
    if (*s == '"') s++;
    *str = s;
    return ast_new_string(buffer);
}

static ASTNode *parse_json_number(char **str) {
    char *s = *str;
    double num = strtod(s, &s);
    *str = s;
    return ast_new_number(num);
}

static ASTNode *parse_json_array(char **str) {
    char *s = *str;
    if (*s != '[') return NULL;
    s++;
    
    ASTNode *list = ast_new_list();
    s = skip_whitespace(s);
    
    if (*s == ']') {
        *str = s + 1;
        return list;
    }
    
    while (*s) {
        s = skip_whitespace(s);
        ASTNode *value = parse_json_value(&s);
        if (value) ast_list_add_element(list, value);
        
        s = skip_whitespace(s);
        if (*s == ',') s++;
        else if (*s == ']') break;
    }
    
    if (*s == ']') s++;
    *str = s;
    return list;
}

static ASTNode *parse_json_object(char **str) {
    char *s = *str;
    if (*s != '{') return NULL;
    s++;
    
    ASTNode *dict = ast_new_dict();
    s = skip_whitespace(s);
    
    if (*s == '}') {
        *str = s + 1;
        return dict;
    }
    
    while (*s) {
        s = skip_whitespace(s);
        ASTNode *key = parse_json_string(&s);
        if (!key) break;
        
        s = skip_whitespace(s);
        if (*s == ':') s++;
        
        s = skip_whitespace(s);
        ASTNode *value = parse_json_value(&s);
        if (value) ast_dict_add_pair(dict, key, value);
        
        s = skip_whitespace(s);
        if (*s == ',') s++;
        else if (*s == '}') break;
    }
    
    if (*s == '}') s++;
    *str = s;
    return dict;
}

static ASTNode *parse_json_value(char **str) {
    char *s = skip_whitespace(*str);
    
    if (*s == '"') return parse_json_string(str);
    if (*s == '[') return parse_json_array(str);
    if (*s == '{') return parse_json_object(str);
    if (isdigit(*s) || *s == '-') return parse_json_number(str);
    if (strncmp(s, "true", 4) == 0) { *str = s + 4; return ast_new_number(1); }
    if (strncmp(s, "false", 5) == 0) { *str = s + 5; return ast_new_number(0); }
    if (strncmp(s, "null", 4) == 0) { *str = s + 4; return ast_new_undef(); }
    
    return NULL;
}

ASTNode *tesseract_json_parse(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING)
        return ast_new_undef();
    
    char *json_str = strdup(args[0]->string);
    char *ptr = json_str;
    ASTNode *result = parse_json_value(&ptr);
    free(json_str);
    
    return result ? result : ast_new_undef();
}

static void stringify_value(ASTNode *node, char *buffer, int *pos, int max_len) {
    if (*pos >= max_len - 1) return;
    
    switch (node->type) {
        case NODE_STRING:
            buffer[(*pos)++] = '"';
            for (int i = 0; node->string[i] && *pos < max_len - 1; i++) {
                if (node->string[i] == '"' || node->string[i] == '\\') {
                    buffer[(*pos)++] = '\\';
                }
                buffer[(*pos)++] = node->string[i];
            }
            buffer[(*pos)++] = '"';
            break;
            
        case NODE_NUMBER: {
            char num_str[32];
            if (node->number == (int)node->number)
                sprintf(num_str, "%d", (int)node->number);
            else
                sprintf(num_str, "%.6g", node->number);
            for (int i = 0; num_str[i] && *pos < max_len - 1; i++)
                buffer[(*pos)++] = num_str[i];
            break;
        }
        
        case NODE_LIST:
            buffer[(*pos)++] = '[';
            for (int i = 0; i < node->list.count && *pos < max_len - 1; i++) {
                if (i > 0) buffer[(*pos)++] = ',';
                stringify_value(node->list.elements[i], buffer, pos, max_len);
            }
            buffer[(*pos)++] = ']';
            break;
            
        case NODE_DICT:
            buffer[(*pos)++] = '{';
            for (int i = 0; i < node->dict.count && *pos < max_len - 1; i++) {
                if (i > 0) buffer[(*pos)++] = ',';
                stringify_value(node->dict.keys[i], buffer, pos, max_len);
                buffer[(*pos)++] = ':';
                stringify_value(node->dict.values[i], buffer, pos, max_len);
            }
            buffer[(*pos)++] = '}';
            break;
            
        case NODE_UNDEF:
            strcpy(buffer + *pos, "null");
            *pos += 4;
            break;
            
        default:
            strcpy(buffer + *pos, "null");
            *pos += 4;
            break;
    }
}

ASTNode *tesseract_json_stringify(ASTNode **args, int arg_count) {
    if (arg_count != 1) return ast_new_string("");
    
    char buffer[2048] = {0};
    int pos = 0;
    stringify_value(args[0], buffer, &pos, 2048);
    buffer[pos] = '\0';
    
    return ast_new_string(buffer);
}

// Package initialization function
void init_json_utils_package()
{
    register_package_function("json_escape", tesseract_json_escape);
    register_package_function("json_unescape", tesseract_json_unescape);
    register_package_function("is_valid_json_string", tesseract_is_valid_json_string);
    register_package_function("json_format_number", tesseract_json_format_number);
    register_package_function("json_parse", tesseract_json_parse);
    register_package_function("json_stringify", tesseract_json_stringify);
}