#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "interpreter.h"
#include "ast.h"

#define MAX_INPUT 1024

void run_repl()
{
    printf("Tesseract REPL (Type 'exit' to quit)\n> ");
    fflush(stdout);

    char input[MAX_INPUT];
    while (fgets(input, MAX_INPUT, stdin))
    {
        // Remove newline
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;
        if (strlen(input) == 0)
        {
            printf("> ");
            continue;
        }

        parser_init(input);
        ASTNode *root = parse_program();
        if (root)
        {
            interpret(root);
        }
        printf("> ");
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