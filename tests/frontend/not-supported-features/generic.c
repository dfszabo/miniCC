// RUN: AArch64

// FUNC-DECL: int test(int)
// COMPILE-FAIL

#define typecheck(T) _Generic( (T), char: 1, int: 2, long: 3, float: 4, default: 0)

int test() {
   typecheck(2353463456356465);
   return 0;
}
