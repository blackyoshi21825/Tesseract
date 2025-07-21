#ifndef COMPAT_H
#define COMPAT_H

#include <string.h>

#if defined(_WIN32) || defined(_MSC_VER)
#include <string.h>
#define strdup _strdup
#else
/* For systems that don't have strdup in string.h */
#ifndef strdup
#include <stdlib.h>
char *strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *new = malloc(len);
    if (new == NULL) return NULL;
    return (char *)memcpy(new, s, len);
}
#endif
#endif

#endif /* COMPAT_H */