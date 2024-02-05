/*
 * Ryan Jacoby <ryjacoby@calpoly.edu>
 * fragaria/src/string.h
 *
 * Header for Fragaria C standard string functions
 */

#ifndef STRING_H
#define STRING_H                                1

#include <stddef.h>

void * memset(void * dst, int c, size_t n);
void * memcpy(void * dst, const void * src, size_t n);
size_t strlen(const char * s);
char * strcpy(char * dst, const char * src);
int strcmp(const char * s1, const char * s2);
const char * strchr(const char * s, int c);
char * strdup(const char * s);

#endif /* #ifndef STRING_H */
