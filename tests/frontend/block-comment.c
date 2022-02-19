// RUN: AArch64

// FUNC-DECL: int test()
// TEST-CASE: test() -> 1

/* multi
 * line
 * comment
 */
unsigned test(/* foo */) {
  return 1; /* foo */
}
