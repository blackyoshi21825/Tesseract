#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "parser.h"
#include "interpreter.h"
#include "ast.h"

#define MAX_INPUT_SIZE 1000000

bool is_incomplete_input(const char *input)
{
    int len = strlen(input);
    if (len == 0)
        return false;

    // Check for trailing backslash
    if (input[len - 1] == '\\')
    {
        return true;
    }

    // Check for unbalanced braces
    int brace_level = 0;
    for (int i = 0; i < len; i++)
    {
        if (input[i] == '{')
            brace_level++;
        else if (input[i] == '}')
            brace_level--;
    }

    return brace_level > 0;
}

void process_input(const char *input)
{
    parser_init(input);
    ASTNode *root = parse_program();
    if (root)
    {
        interpret(root);
        ast_free(root);
    }
}

void start_repl()
{
    printf("Tesseract REPL (Type 'exit' to quit, 'help' for help)\n");

    char buffer[MAX_INPUT_SIZE] = {0};
    bool in_multiline = false;

    while (1)
    {
        printf(in_multiline ? "... " : "> ");
        fflush(stdout);

        char line[MAX_INPUT_SIZE];
        if (!fgets(line, sizeof(line), stdin))
        {
            break;
        }

        // Remove newline
        line[strcspn(line, "\n")] = 0;

        // Check for commands
        if (!in_multiline)
        {
            if (strcmp(line, "exit") == 0)
            {
                break;
            }
            if (strcmp(line, "help") == 0)
            {
                printf("Commands:\n");
                printf("  exit    - Exit the REPL\n");
                printf("  help    - Show this help\n");
                printf("Type Tesseract code to evaluate it\n");
                continue;
            }
        }

        // Handle multi-line input
        if (in_multiline)
        {
            strcat(buffer, "\n");
            strcat(buffer, line);
        }
        else
        {
            strcpy(buffer, line);
        }

        if (is_incomplete_input(buffer))
        {
            in_multiline = true;
            continue;
        }

        // Process complete input
        process_input(buffer);

        // Reset for next input
        buffer[0] = '\0';
        in_multiline = false;
    }

    printf("Goodbye!\n");
}
int main(int argc, char **argv)
{
    if (argc == 1)
    {
        // Start REPL mode
        start_repl();
    }
    else if (argc == 2)
    {
        // File execution mode
        char *source = read_file(argv[1]);
        if (!source)
        {
            return 1;
        }

        parser_init(source);
        ASTNode *root = parse_program();
        interpret(root);

        free(source);
        ast_free(root);
    }
    else
    {
        printf("Usage: %s [script.tesser]\n", argv[0]);
        return 1;
    }

    return 0;
}