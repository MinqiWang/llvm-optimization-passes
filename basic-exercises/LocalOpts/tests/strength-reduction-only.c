int noneOptimization(int a)
{
    int b = a + 1;
    int c = 6 * a;
    c = b + a;
    return b;
}

int oneOptimizationCase1(int a)
{
    int b = a * 2;
    return b;
}

int oneOptimizationCase2(int a)
{
    int b = 2 * a;
    return b;
}

int oneOptimizationCase3(unsigned a)
{
    int b = a / 2;
    return b;
}

int multipleOptimizationConsecutive(int a)
{
    int b = a * 2;
    b = 2 * a;
    b = a / 2;
    return b;
}

int multipleOptimizationNonConsecutive(int a)
{
    int b = a * 2;
    int c = 3 * a + 1;
    b = c * 5;
    b = a / 2;
    return b;
}
