#ifndef _STDLIB_H
#define _STDLIB_H

#include <stdint.h>

/* Call all functions registered with `atexit' and `on_exit',
   in the reverse of the order in which they were registered,
   perform stdio cleanup, and terminate program execution with STATUS.  */
void exit(int __status);

/* Return a random integer between 0 and RAND_MAX inclusive.  */
int rand(void);
/* Seed the random number generator with the given number.  */
void srand(unsigned int __seed);

/* Allocate SIZE bytes of memory.  */
void *malloc(size_t __size);
/* Allocate NMEMB elements of SIZE bytes each, all initialized to 0.  */
void *calloc(size_t __nmemb, size_t __size);

/* Free a block allocated by `malloc', `realloc' or `calloc'.  */
void free(void *__ptr);

#endif
