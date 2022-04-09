// RUN: AArch64

// FUNC-DECL: long long test(long long,long long)
// TEST-CASE: test(0xffffffff, 1) -> 0x100000000
// TEST-CASE: test(0x100000000, 0x111111111) -> 0x211111111

long long test(long long l, long long r) {
    return l + r;
}
