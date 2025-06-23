#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "interpreter.h"
#include "ast.h"

#define MAX_FILE_SIZE 1000000

// Read whole file into a dynamically allocated buffer
char* read_file(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    if (size <= 0 || size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size invalid or too large\n");
        fclose(f);
        return NULL;
    }
    char* buffer = malloc(size + 1);
    if (!buffer) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(f);
        return NULL;
    }
    fread(buffer, 1, size, f);
    buffer[size] = '\0';
    fclose(f);
    return buffer;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <source.tau>\n", argv[0]);
        return 1;
    }

    char* source = read_file(argv[1]);
    if (!source) return 1;

    parser_init(source);
    ASTNode* root = parse_program();

    interpret(root);

    // TODO: free AST & source buffer here if you want

    free(source);
    return 0;
}
