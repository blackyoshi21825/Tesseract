#include "tesseract_pch.h"
#include "error.h"

#define MAX_INPUT 1024

// Global debug flag
int debug_mode = 0;

void run_repl()
{
    printf("Tesseract REPL v1.0%s (Type 'exit' to quit, 'help' for commands)\n> ", 
           debug_mode ? " [DEBUG]" : "");
    fflush(stdout);

    char input[MAX_INPUT];
    while (fgets(input, MAX_INPUT, stdin))
    {
        // Remove newline
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;
        if (strcmp(input, "help") == 0) {
            printf("Commands:\n");
            printf("  exit - Exit the REPL\n");
            printf("  help - Show this help\n");
            printf("  clear - Clear variables (not implemented)\n");
            if (debug_mode) printf("  [DEBUG MODE ACTIVE]\n");
            printf("> ");
            continue;
        }
        if (strlen(input) == 0)
        {
            printf("> ");
            continue;
        }

        // Initialize error handling
        error_init();
        exception_active = 1;
        
        if (TRY()) {
            // Set current filename for REPL
            error_set_current_file("<repl>");
            
            if (debug_mode) printf("[DEBUG] Parsing input: %s\n", input);
            parser_init(input);
            ASTNode *root = parse_program();
            if (root)
            {
                if (debug_mode) printf("[DEBUG] Interpreting AST...\n");
                interpret(root);
                if (debug_mode) printf("[DEBUG] Execution completed\n");
            }
        } CATCH() {
            // Error occurred, print it and continue
            error_print(&current_error);
        }
        
        exception_active = 0;
        printf("> ");
        fflush(stdout);
    }
}

char *read_file(const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (!f)
        return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *buffer = malloc(size + 1);
    if (!buffer)
    {
        fclose(f);
        return NULL;
    }

    fread(buffer, 1, size, f);
    buffer[size] = '\0';
    fclose(f);
    return buffer;
}

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        // REPL mode
        run_repl();
    }
    else if (argc >= 2)
    {
        // Check for debug flag
        int file_arg = 1;
        if (argc >= 3 && strcmp(argv[1], "--debug") == 0) {
            debug_mode = 1;
            file_arg = 2;
            printf("[DEBUG] Debug mode enabled\n");
        } else if (argc >= 3 && strcmp(argv[2], "--debug") == 0) {
            debug_mode = 1;
            printf("[DEBUG] Debug mode enabled\n");
        }
        
        // File execution mode
        char *source = read_file(argv[file_arg]);
        if (!source)
        {
            fprintf(stderr, "Error: Could not read file '%s'\n", argv[file_arg]);
            return 1;
        }

        if (debug_mode) printf("[DEBUG] File loaded: %s (%ld bytes)\n", argv[file_arg], strlen(source));

        // Set current filename for error reporting
        error_set_current_file(argv[file_arg]);
        
        if (debug_mode) printf("[DEBUG] Starting parse...\n");
        parser_init(source);
        ASTNode *root = parse_program();
        if (debug_mode) printf("[DEBUG] Parse completed, starting interpretation...\n");
        interpret(root);
        if (debug_mode) printf("[DEBUG] Execution finished\n");

        free(source);
    }
    else
    {
        fprintf(stderr, "Usage: %s [--debug] [script.tesseract]\n", argv[0]);
        fprintf(stderr, "       %s --debug (for debug REPL)\n", argv[0]);
        return 1;
    }
    return 0;
}