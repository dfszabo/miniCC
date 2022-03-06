#ifndef _ASSERT_H
#define _ASSERT_H


/* This prints an "Assertion failed" message and aborts.  */
void __assert_fail(const char *__assertion, const char *__file,
                   unsigned int __line, const char *__function);

// TODO: this would be "(void)" presumably to ignore compiler warnings
// which currenty is not emitted anyway, but if will be then modify this.
#define __ASSERT_VOID_CAST

// TODO: this is const char *, but it cannot be handled in the parser yet
#define __ASSERT_FUNCTION ((char *) 0)

// TODO: multiple line macro definitions not handled yet, that why
// it is a single line for now
#define assert(expr) ((expr) ? __ASSERT_VOID_CAST (0)	: __assert_fail (#expr, __FILE__, __LINE__, __ASSERT_FUNCTION))

#endif
