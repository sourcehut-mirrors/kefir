float arith1(float x, float y, float z, float w) {
    return -(x + y) * z / (w - 10.0f);
}

double arith2(double x, double y, double z, double w) {
    return -(x + y) * z / (w - 10.0);
}

void comparison1(float x, float y) {
    (void) (x == y);
    (void) (x > y);
    (void) (x < y);

    (void) (x != y);
    (void) (x >= y);
    (void) (x <= y);
}

void comparison2(double x, double y) {
    (void) (x == y);
    (void) (x > y);
    (void) (x < y);

    (void) (x != y);
    (void) (x >= y);
    (void) (x <= y);
}

void conversion(float x, double y) {
    (void) (double) x;
    (void) (float) y;
    (void) (long) x;
    (void) (long) y;
    (void) (float) (long) y;
    (void) (float) (unsigned long) y;
    (void) (double) (long) x;
    (void) (double) (unsigned long) x;
}
