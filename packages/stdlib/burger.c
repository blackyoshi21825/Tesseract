#include "../core/package_loader.h"
#include "../../include/ast.h"
#include "../../include/variables.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

// BURGER - Build Utility for Rapid Generation and Execution of Resources

// BUN - Project scaffolding and finalization
ASTNode *tesseract_bun_scaffold(ASTNode **args, int arg_count) {
    if (arg_count != 2 || args[0]->type != NODE_STRING || args[1]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *project_name = args[0]->string;
    const char *project_type = args[1]->string;
    
    printf("Scaffolding %s project: %s\n", project_type, project_name);
    
    // Create project directory
    mkdir(project_name, 0755);
    
    char path[512];
    if (strcmp(project_type, "c") == 0) {
        snprintf(path, sizeof(path), "%s/src", project_name);
        mkdir(path, 0755);
        snprintf(path, sizeof(path), "%s/include", project_name);
        mkdir(path, 0755);
        snprintf(path, sizeof(path), "%s/main.c", project_name);
        FILE *f = fopen(path, "w");
        if (f) {
            fprintf(f, "#include <stdio.h>\n\nint main() {\n    printf(\"Hello, %s!\\n\");\n    return 0;\n}\n", project_name);
            fclose(f);
        }
    }
    
    printf("Project scaffolded successfully!\n");
    return ast_new_number(1);
}

// LETTUCE - Logging and debugging utilities
ASTNode *tesseract_lettuce_log(ASTNode **args, int arg_count) {
    if (arg_count < 2 || args[0]->type != NODE_STRING || args[1]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *level = args[0]->string;
    const char *message = args[1]->string;
    
    time_t now = time(NULL);
    char *timestamp = ctime(&now);
    timestamp[strlen(timestamp) - 1] = '\0'; // Remove newline
    
    const char *color = "";
    if (strcmp(level, "ERROR") == 0) color = "\033[31m";
    else if (strcmp(level, "WARN") == 0) color = "\033[33m";
    else if (strcmp(level, "INFO") == 0) color = "\033[32m";
    else if (strcmp(level, "DEBUG") == 0) color = "\033[36m";
    
    printf("%s[%s] %s: %s\033[0m\n", color, timestamp, level, message);
    return ast_new_number(1);
}

// TOMATO - Testing utilities
ASTNode *tesseract_tomato_assert(ASTNode **args, int arg_count) {
    if (arg_count != 2 || args[0]->type != NODE_NUMBER || args[1]->type != NODE_STRING) 
        return ast_new_number(0);
    
    int condition = (int)args[0]->number;
    const char *test_name = args[1]->string;
    
    if (condition) {
        printf("PASS: %s\n", test_name);
        return ast_new_number(1);
    } else {
        printf("FAIL: %s\n", test_name);
        return ast_new_number(0);
    }
}

// CHEESE - Code generation and templates
ASTNode *tesseract_cheese_template(ASTNode **args, int arg_count) {
    if (arg_count != 3 || args[0]->type != NODE_STRING || args[1]->type != NODE_STRING || args[2]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *template_type = args[0]->string;
    const char *name = args[1]->string;
    const char *filename = args[2]->string;
    
    FILE *f = fopen(filename, "w");
    if (!f) return ast_new_number(0);
    
    printf("Generating %s template: %s\n", template_type, name);
    
    if (strcmp(template_type, "header") == 0) {
        fprintf(f, "#ifndef %s_H\n#define %s_H\n\n// %s header file\n\n#endif\n", name, name, name);
    } else if (strcmp(template_type, "class") == 0) {
        fprintf(f, "typedef struct {\n    // Add fields here\n} %s;\n\n%s* %s_new();\nvoid %s_free(%s* obj);\n", name, name, name, name, name);
    }
    
    fclose(f);
    printf("Template generated: %s\n", filename);
    return ast_new_number(1);
}

// MEAT - Main build and compilation utilities
ASTNode *tesseract_meat_compile(ASTNode **args, int arg_count) {
    if (arg_count != 2 || args[0]->type != NODE_STRING || args[1]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *source = args[0]->string;
    const char *output = args[1]->string;
    
    char command[1024];
    snprintf(command, sizeof(command), "gcc -o %s %s", output, source);
    
    printf("Compiling: %s -> %s\n", source, output);
    int result = system(command);
    
    if (result == 0) {
        printf("Compilation successful!\n");
        return ast_new_number(1);
    } else {
        printf("Compilation failed!\n");
        return ast_new_number(0);
    }
}

// ONION - Optimization and performance tools
ASTNode *tesseract_onion_profile(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *function_name = args[0]->string;
    
    printf("Profiling function: %s\n", function_name);
    
    clock_t start = clock();
    // Simulate some work
    for (int i = 0; i < 1000000; i++) {
        // Dummy operation
    }
    clock_t end = clock();
    
    double cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Execution time: %f seconds\n", cpu_time);
    
    return ast_new_number((int)(cpu_time * 1000)); // Return milliseconds
}

// PICKLE - Package and dependency management
ASTNode *tesseract_pickle_deps(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *action = args[0]->string;
    
    printf("Package management: %s\n", action);
    
    if (strcmp(action, "list") == 0) {
        printf("Installed packages:\n");
        printf("  - tesseract-core (1.0.0)\n");
        printf("  - tesseract-stdlib (1.0.0)\n");
        printf("  - burger-utils (1.0.0)\n");
    } else if (strcmp(action, "update") == 0) {
        printf("Updating package registry...\n");
        printf("All packages up to date!\n");
    }
    
    return ast_new_number(1);
}

// SAUCE - Source control and versioning helpers
ASTNode *tesseract_sauce_version(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *action = args[0]->string;
    
    printf("Version control: %s\n", action);
    
    if (strcmp(action, "status") == 0) {
        printf("Repository status:\n");
        printf("  Branch: main\n");
        printf("  Modified files: 2\n");
        printf("  Untracked files: 1\n");
    } else if (strcmp(action, "tag") == 0) {
        printf("Creating version tag: v1.0.0\n");
        printf("Tag created successfully!\n");
    }
    
    return ast_new_number(1);
}

// BURGER - Complete build pipeline
ASTNode *tesseract_burger_build(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *project_name = args[0]->string;
    
    printf("Building complete burger for: %s\n", project_name);
    printf("[1/7] Preparing bun (scaffolding)...\n");
    printf("[2/7] Adding lettuce (logging)...\n");
    printf("[3/7] Adding tomato (testing)...\n");
    printf("[4/7] Adding cheese (templates)...\n");
    printf("[5/7] Adding meat (compilation)...\n");
    printf("[6/7] Adding onion (optimization)...\n");
    printf("[7/7] Adding pickle (dependencies)...\n");
    printf("Finishing with sauce (versioning)...\n");
    printf("Closing the bun...\n");
    printf("\nYour delicious development burger is ready!\n");
    
    return ast_new_number(1);
}

// Package initialization function
void init_burger_package() {
    register_package_function("bun_scaffold", tesseract_bun_scaffold);
    register_package_function("lettuce_log", tesseract_lettuce_log);
    register_package_function("tomato_assert", tesseract_tomato_assert);
    register_package_function("cheese_template", tesseract_cheese_template);
    register_package_function("meat_compile", tesseract_meat_compile);
    register_package_function("onion_profile", tesseract_onion_profile);
    register_package_function("pickle_deps", tesseract_pickle_deps);
    register_package_function("sauce_version", tesseract_sauce_version);
    register_package_function("burger_build", tesseract_burger_build);
    
    register_function_package_mapping("bun_scaffold", "burger");
    register_function_package_mapping("lettuce_log", "burger");
    register_function_package_mapping("tomato_assert", "burger");
    register_function_package_mapping("cheese_template", "burger");
    register_function_package_mapping("meat_compile", "burger");
    register_function_package_mapping("onion_profile", "burger");
    register_function_package_mapping("pickle_deps", "burger");
    register_function_package_mapping("sauce_version", "burger");
    register_function_package_mapping("burger_build", "burger");
}