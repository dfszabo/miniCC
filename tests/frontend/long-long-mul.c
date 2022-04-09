// RUN: AArch64

// FUNC-DECL: long long test(long long,long long)
// TEST-CASE: test(0xffffffffll, 2ll) -> 0x1fffffffell
// TEST-CASE: test(1000000ll, 1000000ll) -> 1000000000000ll

long long test(long long l, long long r) {
    return l * r;
}
