int noneOptimizationCase1(int a)
{
    int b = a + 1;
    int c = 6 * a;
    c = b + a;
    return b;
}

int noneOptimizationCase2(int a)
{
    int b = 3 * a - 1;
    int d = b - 1;
    b = a + 1;
    int c = 6 * a;
    c = b + a;
    return b;
}

int optimizationCase1(int a)
{
    int b = a + 1;
    int c = b - 1;
    int d = -1 + b;
    int e = b + (-1);
    return b;
}

int optimizationCase2(int a)
{
    int b = 1 + a;
    int c = b - 1;
    int d = -1 + b;
    int e = b + (-1);
    return b;
}

int optimizationCase3(int a)
{
    int b = a * 3;
    int c = b / 3;
    return c;
}

int optimizationCase4(int a)
{
    int b = 3 * a;
    int c = b / 3;
    return c;
}

int multipleOptimization(int a)
{
    int b = 1 + a;
    int bb = a * 3;
    int c = b - 1;
    int f = a + 6;
    int d = -1 + b;
    d = f - 1;
    int e = b + (-1);
    e = bb / 3;
    
    return e;
}
