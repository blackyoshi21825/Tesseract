#ifndef OBJECT_H
#define OBJECT_H

#include "ast.h"

typedef enum
{
    FIELD_NUMBER,
    FIELD_STRING,
    FIELD_OBJECT
} FieldType;

typedef struct FieldEntry FieldEntry;

typedef struct FieldEntry
{
    char name[64];
    FieldType type;
    union
    {
        double number_value;
        char string_value[256];
        void *object_value;
    };
    struct FieldEntry *next;
} FieldEntry;

typedef struct ObjectInstance
{
    char class_name[64];
    FieldEntry *fields;
} ObjectInstance;

#endif