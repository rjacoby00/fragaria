/*
 * Ryan Jacoby <ryjacoby@calpoly.edu>
 * fragaria/src/printk.h
 *
 * Header for printk and future related functions
 */

#ifndef PRINTK_H
#define PRINTK_H                                1

int printk(const char * fmt, ...) __attribute__((format (printf, 1, 2)));

#endif /* #ifndef PRINTK_H */
