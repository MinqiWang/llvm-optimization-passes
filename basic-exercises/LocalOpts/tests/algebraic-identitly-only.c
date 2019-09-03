int noneOptimization(int a)
{
    int b = a + 1;
    int c = 6 * a;
    c = b + a;
    return b;
}

int oneOptimizationCase1(int a)
{
    int b = a + 0;
    return b;
}

int oneOptimizationCase2(int a)
{
    int b = 0 + a;
    return b;
}

int oneOptimizationCase3(int a)
{
    int b = a * 1;
    return b;
}

int oneOptimizationCase4(int a)
{
    int b = 1 * a;
    return b;
}

int multipleOptimizationConsecutive(int a)
{
    int b = a + 0;
    b = 0 + a;
    b = 1 * a;
    b = a * 1;
    return b;
}

int multipleOptimizationNonConsecutive(int a)
{
    int b = a + 0;
    int c = 3 * a + 1;
    b = c * 5;
    b = 1 * a;
    return b;
}

