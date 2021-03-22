int test_ra(int a, int b, int c, int d, int i) {
    if (a + b > 10)
        c = a + b;
    else {
        while (i > 0) {
            c = c + a;
            c = c + b;
            c = c - d;
            i = i - 1;
        }
    }
    return c;
}
