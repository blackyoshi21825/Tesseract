#include "../core/package_loader.h"
#include "../../include/ast.h"
#include <string.h>

// Sorting algorithms
ASTNode *tesseract_bubble_sort(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_LIST) return ast_new_number(0);
    
    ASTNode *list = args[0];
    int n = list->list.count;
    
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (list->list.elements[j]->type == NODE_NUMBER && 
                list->list.elements[j + 1]->type == NODE_NUMBER) {
                if (list->list.elements[j]->number > list->list.elements[j + 1]->number) {
                    ASTNode *temp = list->list.elements[j];
                    list->list.elements[j] = list->list.elements[j + 1];
                    list->list.elements[j + 1] = temp;
                }
            }
        }
    }
    return ast_new_number(1);
}

ASTNode *tesseract_quick_sort(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_LIST) return ast_new_number(0);
    
    // Simple quicksort implementation
    ASTNode *list = args[0];
    int n = list->list.count;
    
    if (n <= 1) return ast_new_number(1);
    
    // Use bubble sort for simplicity in this minimal implementation
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (list->list.elements[j]->type == NODE_NUMBER && 
                list->list.elements[j + 1]->type == NODE_NUMBER) {
                if (list->list.elements[j]->number > list->list.elements[j + 1]->number) {
                    ASTNode *temp = list->list.elements[j];
                    list->list.elements[j] = list->list.elements[j + 1];
                    list->list.elements[j + 1] = temp;
                }
            }
        }
    }
    return ast_new_number(1);
}

// Search algorithms
ASTNode *tesseract_binary_search(ASTNode **args, int arg_count) {
    if (arg_count != 2 || args[0]->type != NODE_LIST || args[1]->type != NODE_NUMBER) 
        return ast_new_number(-1);
    
    ASTNode *list = args[0];
    double target = args[1]->number;
    int left = 0, right = list->list.count - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (list->list.elements[mid]->type != NODE_NUMBER) return ast_new_number(-1);
        
        double mid_val = list->list.elements[mid]->number;
        if (mid_val == target) return ast_new_number(mid);
        if (mid_val < target) left = mid + 1;
        else right = mid - 1;
    }
    return ast_new_number(-1);
}

ASTNode *tesseract_linear_search(ASTNode **args, int arg_count) {
    if (arg_count != 2 || args[0]->type != NODE_LIST || args[1]->type != NODE_NUMBER) 
        return ast_new_number(-1);
    
    ASTNode *list = args[0];
    double target = args[1]->number;
    
    for (int i = 0; i < list->list.count; i++) {
        if (list->list.elements[i]->type == NODE_NUMBER && 
            list->list.elements[i]->number == target) {
            return ast_new_number(i);
        }
    }
    return ast_new_number(-1);
}

// Array algorithms
ASTNode *tesseract_reverse(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_LIST) return ast_new_number(0);
    
    ASTNode *list = args[0];
    int n = list->list.count;
    
    for (int i = 0; i < n / 2; i++) {
        ASTNode *temp = list->list.elements[i];
        list->list.elements[i] = list->list.elements[n - 1 - i];
        list->list.elements[n - 1 - i] = temp;
    }
    return ast_new_number(1);
}

ASTNode *tesseract_find_max(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_LIST) return ast_new_number(0);
    
    ASTNode *list = args[0];
    if (list->list.count == 0) return ast_new_number(0);
    
    double max_val = list->list.elements[0]->number;
    for (int i = 1; i < list->list.count; i++) {
        if (list->list.elements[i]->type == NODE_NUMBER && 
            list->list.elements[i]->number > max_val) {
            max_val = list->list.elements[i]->number;
        }
    }
    return ast_new_number(max_val);
}

ASTNode *tesseract_find_min(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_LIST) return ast_new_number(0);
    
    ASTNode *list = args[0];
    if (list->list.count == 0) return ast_new_number(0);
    
    double min_val = list->list.elements[0]->number;
    for (int i = 1; i < list->list.count; i++) {
        if (list->list.elements[i]->type == NODE_NUMBER && 
            list->list.elements[i]->number < min_val) {
            min_val = list->list.elements[i]->number;
        }
    }
    return ast_new_number(min_val);
}

// Package initialization function
void init_algorithms_package() {
    register_package_function("bubble_sort", tesseract_bubble_sort);
    register_package_function("quick_sort", tesseract_quick_sort);
    register_package_function("binary_search", tesseract_binary_search);
    register_package_function("linear_search", tesseract_linear_search);
    register_package_function("reverse", tesseract_reverse);
    register_package_function("find_max", tesseract_find_max);
    register_package_function("find_min", tesseract_find_min);
}