typedef struct {
    int x;
    int y;
} Point;

char foo(Point *p) {
  return p->x + p->y;
}
