/*
 * Ryan Jacoby <ryjacoby@calpoly.edu>
 * fragaria/src/string.c
 *
 * Fragaria C standard string functions
 * 
 */

#include <stddef.h>
#include <stdint.h>

/*
 *
 */
void * memset(void * dst, int c, size_t n)
{
        return NULL;
}

/**
 * memcpy() - copy memory area
 * @dst: pointer to destination memory region
 * @src: pointer to source memory region
 * @n: number of bytes to copy
 *
 * Copies n bytes from source memory area to dst.  Areas must not overlap.
 *
 * Return: pointer to dest on success
 */
void * memcpy(void * dst, const void * src, size_t n)
{
        int i;

        for(i = 0; i < n; i++)
                ((uint8_t *)dst)[i] = ((uint8_t *)src)[i];

        return dst;
}

/*
 *
 */
size_t strlen(const char * s)
{
        int ret;

        for(ret = 0; s[ret]; ret++);

        return ret;
}

/*
 *
 */
char * strcpy(char * dst, const char * src)
{
        return NULL;
}

/*
 *
 */
int strcmp(const char * s1, const char * s2)
{
        return 0;
}

/*
 *
 */
const char * strchr(const char * s, int c)
{
        return NULL;
}

/*
 *
 */
char * strdup(const char * s)
{
        return NULL;
}
