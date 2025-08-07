#include "../package_loader.h"
#include "../../include/ast.h"
#include <math.h>

// Math package functions for Tesseract
ASTNode *tesseract_factorial(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_NUMBER) return ast_new_number(0);
    
    double n = args[0]->number;
    if (n <= 1) return ast_new_number(1);
    
    double result = 1;
    for (int i = 2; i <= (int)n; i++) {
        result *= i;
    }
    return ast_new_number(result);
}

ASTNode *tesseract_power(ASTNode **args, int arg_count) {
    if (arg_count != 2 || args[0]->type != NODE_NUMBER || args[1]->type != NODE_NUMBER) 
        return ast_new_number(0);
    
    double base = args[0]->number;
    double exp = args[1]->number;
    return ast_new_number(pow(base, exp));
}

ASTNode *tesseract_sqrt(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_NUMBER) return ast_new_number(0);
    
    double n = args[0]->number;
    if (n < 0) return ast_new_number(0); // Invalid for negative numbers
    
    return ast_new_number(sqrt(n));
}

ASTNode *tesseract_abs(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_NUMBER) return ast_new_number(0);
    
    double n = args[0]->number;
    return ast_new_number(fabs(n));
}

ASTNode *tesseract_sin(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_NUMBER) return ast_new_number(0);
    
    double n = args[0]->number;
    return ast_new_number(sin(n));
}

ASTNode *tesseract_cos(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_NUMBER) return ast_new_number(0);
    
    double n = args[0]->number;
    return ast_new_number(cos(n));
}

ASTNode *tesseract_tan(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_NUMBER) return ast_new_number(0);
    
    double n = args[0]->number;
    return ast_new_number(tan(n));
}

ASTNode *tesseract_is_prime(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_NUMBER) return ast_new_number(0);
    
    int n = (int)args[0]->number;
    if (n <= 1) return ast_new_number(0);
    if (n <= 3) return ast_new_number(1);
    if (n % 2 == 0 || n % 3 == 0) return ast_new_number(0);
    
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return ast_new_number(0);
    }
    return ast_new_number(1);
}

ASTNode *tesseract_gcd(ASTNode **args, int arg_count) {
    if (arg_count != 2 || args[0]->type != NODE_NUMBER || args[1]->type != NODE_NUMBER) 
        return ast_new_number(0);
    
    int a = (int)args[0]->number;
    int b = (int)args[1]->number;
    
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return ast_new_number(a);
}

ASTNode *tesseract_lcm(ASTNode **args, int arg_count) {
    if (arg_count != 2 || args[0]->type != NODE_NUMBER || args[1]->type != NODE_NUMBER) 
        return ast_new_number(0);
    
    int a = (int)args[0]->number;
    int b = (int)args[1]->number;
    
    // Calculate GCD first
    int gcd_val = a;
    int temp_b = b;
    while (temp_b != 0) {
        int temp = temp_b;
        temp_b = gcd_val % temp_b;
        gcd_val = temp;
    }
    
    return ast_new_number((a * b) / gcd_val);
}

// Package initialization function
void init_math_utils_package() {
    register_package_function("factorial", tesseract_factorial);
    register_package_function("power", tesseract_power);
    register_package_function("sqrt", tesseract_sqrt);
    register_package_function("abs", tesseract_abs);
    register_package_function("sin", tesseract_sin);
    register_package_function("cos", tesseract_cos);
    register_package_function("tan", tesseract_tan);
    register_package_function("is_prime", tesseract_is_prime);
    register_package_function("gcd", tesseract_gcd);
    register_package_function("lcm", tesseract_lcm);
}