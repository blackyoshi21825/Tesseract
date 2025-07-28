#include "tesseract_pch.h"
#include "error.h"

#define MAX_INPUT 1024

void run_repl()
{
    printf("Tesseract REPL v1.0 (Type 'exit' to quit, 'help' for commands)\n> ");
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
            
            parser_init(input);
            ASTNode *root = parse_program();
            if (root)
            {
                interpret(root);
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
    else if (argc == 2)
    {
        // File execution mode
        char *source = read_file(argv[1]);
        if (!source)
        {
            fprintf(stderr, "Error: Could not read file '%s'\n", argv[1]);
            return 1;
        }

        // Set current filename for error reporting
        error_set_current_file(argv[1]);
        
        parser_init(source);
        ASTNode *root = parse_program();
        interpret(root);

        free(source);
    }
    else
    {
        fprintf(stderr, "Usage: %s [script.tesseract]\n", argv[0]);
        return 1;
    }
    return 0;
}