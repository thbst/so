/**
 * Operating Sytems 2013 - Assignment 2
 *
 */

#ifndef _UTILS_H
#define _UTILS_H

#include <stdio.h>
#include "parser.h"

#define CHUNK_SIZE 100
#define ERR_ALLOCATION "unable to allocate memory"

#define SHELL_EXIT -100

 /* useful macro for handling error codes */
#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(EXIT_FAILURE);				\
		}							\
	} while(0)

/*
 * debugging macros
 *    heavily inspired by previous work and Internet resources
 *
 * uses C99 variadic macros
 * uses non-standard usage of the token-paste operator (##) for
 *   removing the comma symbol (,) when not followed by a token
 * uses non-standard __FUNCTION__ macro (MSVC doesn't support __func__)
 * tested on gcc 4.4.5 and Visual Studio 2008 (9.0), compiler version 15.00
 *
 * 2011, Razvan Deaconescu, razvan.deaconescu@cs.pub.ro
 */

/*
 * define DEBUG macro as a compiler option:
 *    -DDEBUG for GCC
 *    /DDEBUG for MSVC
 */

#if defined DEBUG
#define dprintf(format, ...)					\
	fprintf(stderr, " [%s(), %s:%u] " format,		\
			__FUNCTION__, __FILE__, __LINE__,	\
			##__VA_ARGS__)
#else
#define dprintf(format, ...)					\
	do {							\
	} while (0)
#endif

/**
 * Readline from mini-shell.
 */
char *read_line();

/**
 * Parse and execute a command.
 */
int parse_command(command_t *, int, command_t *);

#endif

