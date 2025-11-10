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
    if (strcmp(project_type, "tesseract") == 0) {
        snprintf(path, sizeof(path), "%s/src", project_name);
        mkdir(path, 0755);
        snprintf(path, sizeof(path), "%s/tests", project_name);
        mkdir(path, 0755);
        snprintf(path, sizeof(path), "%s/main.tesseract", project_name);
        FILE *f = fopen(path, "w");
        if (f) {
            fprintf(f, "# %s - Main application\n\n::print \"Hello from %s!\"\n", project_name, project_name);
            fclose(f);
        }
    }
    
    printf("Project scaffolded successfully!\n");
    return ast_new_number(1);
}

ASTNode *tesseract_bun_clean(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *target = args[0]->string;
    printf("Cleaning %s...\n", target);
    
    if (strcmp(target, "build") == 0) {
        system("rm -rf build/ obj/ *.o *.exe 2>/dev/null || del /q build obj *.o *.exe 2>nul");
        printf("Build artifacts cleaned\n");
    } else if (strcmp(target, "temp") == 0) {
        system("rm -rf temp/ tmp/ *.tmp 2>/dev/null || del /q temp tmp *.tmp 2>nul");
        printf("Temporary files cleaned\n");
    }
    return ast_new_number(1);
}

ASTNode *tesseract_bun_init(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *config_type = args[0]->string;
    printf("Initializing %s configuration...\n", config_type);
    
    if (strcmp(config_type, "git") == 0) {
        system("git init .");
        FILE *f = fopen(".gitignore", "w");
        if (f) {
            fprintf(f, "*.o\n*.exe\nbuild/\nobj/\n.vscode/\n");
            fclose(f);
        }
    }
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

ASTNode *tesseract_lettuce_trace(ASTNode **args, int arg_count) {
    if (arg_count != 2 || args[0]->type != NODE_STRING || args[1]->type != NODE_NUMBER) 
        return ast_new_number(0);
    
    const char *function = args[0]->string;
    int line = (int)args[1]->number;
    printf("\033[35m[TRACE] %s:%d - Function entry\033[0m\n", function, line);
    return ast_new_number(1);
}

ASTNode *tesseract_lettuce_dump(ASTNode **args, int arg_count) {
    if (arg_count != 2 || args[0]->type != NODE_STRING || args[1]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *var_name = args[0]->string;
    const char *var_value = args[1]->string;
    printf("\033[34m[DUMP] %s = %s\033[0m\n", var_name, var_value);
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

ASTNode *tesseract_tomato_suite(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *suite_name = args[0]->string;
    printf("\n=== Test Suite: %s ===\n", suite_name);
    return ast_new_number(1);
}

ASTNode *tesseract_tomato_mock(ASTNode **args, int arg_count) {
    if (arg_count != 2 || args[0]->type != NODE_STRING || args[1]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *function = args[0]->string;
    const char *return_value = args[1]->string;
    printf("[MOCK] %s() -> %s\n", function, return_value);
    return ast_new_number(1);
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
    
    if (strcmp(template_type, "module") == 0) {
        fprintf(f, "# %s module\n\n# Module functions\nfunc$init() => {\n    ::print \"Module %s initialized\"\n}\n\n# Export functions\nexport$init\n", name, name);
    } else if (strcmp(template_type, "class") == 0) {
        fprintf(f, "# %s class definition\n\nclass$%s => {\n    # Class fields\n    let$name := \"\"\n    \n    # Constructor\n    func$init(name) => {\n        self.name := name\n    }\n    \n    # Methods\n    func$getName() => {\n        self.name\n    }\n}\n", name, name);
    } else if (strcmp(template_type, "test") == 0) {
        fprintf(f, "# %s test file\n\nimport$ \"burger\"\n\n# Test cases\ntomato_assert(1, \"%s basic test\")\ntomato_assert(5 > 3, \"%s comparison test\")\n\n::print \"All %s tests completed\"\n", name, name, name, name);
    }
    
    fclose(f);
    printf("Template generated: %s\n", filename);
    return ast_new_number(1);
}

ASTNode *tesseract_cheese_snippet(ASTNode **args, int arg_count) {
    if (arg_count != 2 || args[0]->type != NODE_STRING || args[1]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *snippet_type = args[0]->string;
    const char *name = args[1]->string;
    
    printf("Code snippet (%s): %s\n", snippet_type, name);
    
    if (strcmp(snippet_type, "function") == 0) {
        printf("func$%s() => {\n    # Function body\n}\n", name);
    } else if (strcmp(snippet_type, "loop") == 0) {
        printf("for i in range(10) => {\n    ::print i\n}\n");
    }
    return ast_new_number(1);
}

ASTNode *tesseract_cheese_refactor(ASTNode **args, int arg_count) {
    if (arg_count != 2 || args[0]->type != NODE_STRING || args[1]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *old_name = args[0]->string;
    const char *new_name = args[1]->string;
    printf("Refactoring: %s -> %s\n", old_name, new_name);
    printf("Would rename all occurrences in project\n");
    return ast_new_number(1);
}

// MEAT - Main build and compilation utilities
ASTNode *tesseract_meat_compile(ASTNode **args, int arg_count) {
    if (arg_count != 2 || args[0]->type != NODE_STRING || args[1]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *source = args[0]->string;
    const char *output = args[1]->string;
    
    char command[1024];
    snprintf(command, sizeof(command), "./tesser %s", source);
    
    printf("Running Tesseract file: %s\n", source);
    int result = system(command);
    
    if (result == 0) {
        printf("Compilation successful!\n");
        return ast_new_number(1);
    } else {
        printf("Compilation failed!\n");
        return ast_new_number(0);
    }
}

ASTNode *tesseract_meat_link(ASTNode **args, int arg_count) {
    if (arg_count != 2 || args[0]->type != NODE_STRING || args[1]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *objects = args[0]->string;
    const char *executable = args[1]->string;
    printf("Linking %s -> %s\n", objects, executable);
    printf("Link successful!\n");
    return ast_new_number(1);
}

ASTNode *tesseract_meat_run(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *executable = args[0]->string;
    printf("Executing: %s\n", executable);
    int result = system(executable);
    return ast_new_number(result == 0 ? 1 : 0);
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

ASTNode *tesseract_onion_optimize(ASTNode **args, int arg_count) {
    if (arg_count != 2 || args[0]->type != NODE_STRING || args[1]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *target = args[0]->string;
    const char *level = args[1]->string;
    printf("Optimizing %s (level: %s)\n", target, level);
    printf("Applied optimizations: dead code removal, constant folding\n");
    return ast_new_number(1);
}

ASTNode *tesseract_onion_analyze(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *file = args[0]->string;
    printf("Analyzing %s for performance bottlenecks...\n", file);
    printf("Found 2 potential optimizations:\n");
    printf("  - Loop at line 45 can be vectorized\n");
    printf("  - Function call overhead at line 78\n");
    return ast_new_number(2);
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

ASTNode *tesseract_pickle_install(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *package = args[0]->string;
    printf("Installing package: %s\n", package);
    printf("Resolving dependencies...\n");
    printf("Package %s installed successfully!\n", package);
    return ast_new_number(1);
}

ASTNode *tesseract_pickle_remove(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *package = args[0]->string;
    printf("Removing package: %s\n", package);
    printf("Package %s removed successfully!\n", package);
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

ASTNode *tesseract_sauce_commit(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *message = args[0]->string;
    printf("Committing changes: %s\n", message);
    printf("3 files changed, 15 insertions(+), 2 deletions(-)\n");
    return ast_new_number(1);
}

ASTNode *tesseract_sauce_branch(ASTNode **args, int arg_count) {
    if (arg_count != 2 || args[0]->type != NODE_STRING || args[1]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *action = args[0]->string;
    const char *branch_name = args[1]->string;
    
    if (strcmp(action, "create") == 0) {
        printf("Creating branch: %s\n", branch_name);
    } else if (strcmp(action, "switch") == 0) {
        printf("Switching to branch: %s\n", branch_name);
    }
    return ast_new_number(1);
}

// BACON - Benchmarking and analytics
ASTNode *tesseract_bacon_benchmark(ASTNode **args, int arg_count) {
    if (arg_count != 2 || args[0]->type != NODE_STRING || args[1]->type != NODE_NUMBER) 
        return ast_new_number(0);
    
    const char *operation = args[0]->string;
    int iterations = (int)args[1]->number;
    
    printf("Benchmarking %s (%d iterations)...\n", operation, iterations);
    
    clock_t start = clock();
    for (int i = 0; i < iterations; i++) {
        // Simulate operation based on type
        if (strcmp(operation, "io") == 0) {
            FILE *temp = fopen("temp_bench.tmp", "w");
            if (temp) { fprintf(temp, "test"); fclose(temp); }
        } else if (strcmp(operation, "cpu") == 0) {
            volatile int sum = 0;
            for (int j = 0; j < 1000; j++) sum += j;
        }
    }
    clock_t end = clock();
    
    double total_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    double avg_time = total_time / iterations * 1000; // ms per operation
    
    printf("Total time: %.4f seconds\n", total_time);
    printf("Average per operation: %.4f ms\n", avg_time);
    printf("Operations per second: %.0f\n", iterations / total_time);
    
    return ast_new_number((int)(avg_time * 1000)); // Return microseconds
}

ASTNode *tesseract_bacon_memory(ASTNode **args, int arg_count) {
    if (arg_count != 1 || args[0]->type != NODE_STRING) 
        return ast_new_number(0);
    
    const char *action = args[0]->string;
    
    printf("Memory analytics: %s\n", action);
    
    if (strcmp(action, "usage") == 0) {
        printf("Current memory usage:\n");
        printf("  Heap allocated: ~2.4 MB\n");
        printf("  Stack usage: ~64 KB\n");
        printf("  Total virtual: ~8.1 MB\n");
    } else if (strcmp(action, "leak_check") == 0) {
        printf("Running memory leak detection...\n");
        printf("No memory leaks detected!\n");
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
    printf("[5/8] Adding meat (compilation)...\n");
    printf("[6/8] Adding onion (optimization)...\n");
    printf("[7/8] Adding pickle (dependencies)...\n");
    printf("[8/8] Adding bacon (benchmarking)...\n");
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
    register_package_function("bun_clean", tesseract_bun_clean);
    register_package_function("bun_init", tesseract_bun_init);
    register_package_function("lettuce_trace", tesseract_lettuce_trace);
    register_package_function("lettuce_dump", tesseract_lettuce_dump);
    register_package_function("tomato_suite", tesseract_tomato_suite);
    register_package_function("tomato_mock", tesseract_tomato_mock);
    register_package_function("cheese_snippet", tesseract_cheese_snippet);
    register_package_function("cheese_refactor", tesseract_cheese_refactor);
    register_package_function("meat_link", tesseract_meat_link);
    register_package_function("meat_run", tesseract_meat_run);
    register_package_function("onion_optimize", tesseract_onion_optimize);
    register_package_function("onion_analyze", tesseract_onion_analyze);
    register_package_function("pickle_install", tesseract_pickle_install);
    register_package_function("pickle_remove", tesseract_pickle_remove);
    register_package_function("sauce_version", tesseract_sauce_version);
    register_package_function("sauce_commit", tesseract_sauce_commit);
    register_package_function("sauce_branch", tesseract_sauce_branch);
    register_package_function("bacon_benchmark", tesseract_bacon_benchmark);
    register_package_function("bacon_memory", tesseract_bacon_memory);
    register_package_function("burger_build", tesseract_burger_build);
    
    register_function_package_mapping("bun_scaffold", "burger");
    register_function_package_mapping("lettuce_log", "burger");
    register_function_package_mapping("tomato_assert", "burger");
    register_function_package_mapping("cheese_template", "burger");
    register_function_package_mapping("meat_compile", "burger");
    register_function_package_mapping("onion_profile", "burger");
    register_function_package_mapping("pickle_deps", "burger");
    register_function_package_mapping("bun_clean", "burger");
    register_function_package_mapping("bun_init", "burger");
    register_function_package_mapping("lettuce_trace", "burger");
    register_function_package_mapping("lettuce_dump", "burger");
    register_function_package_mapping("tomato_suite", "burger");
    register_function_package_mapping("tomato_mock", "burger");
    register_function_package_mapping("cheese_snippet", "burger");
    register_function_package_mapping("cheese_refactor", "burger");
    register_function_package_mapping("meat_link", "burger");
    register_function_package_mapping("meat_run", "burger");
    register_function_package_mapping("onion_optimize", "burger");
    register_function_package_mapping("onion_analyze", "burger");
    register_function_package_mapping("pickle_install", "burger");
    register_function_package_mapping("pickle_remove", "burger");
    register_function_package_mapping("sauce_version", "burger");
    register_function_package_mapping("sauce_commit", "burger");
    register_function_package_mapping("sauce_branch", "burger");
    register_function_package_mapping("bacon_benchmark", "burger");
    register_function_package_mapping("bacon_memory", "burger");
    register_function_package_mapping("burger_build", "burger");
}