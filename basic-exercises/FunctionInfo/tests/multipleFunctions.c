int notBeingCalled(int a)
{
    return a + 1;
}

int beingCalledOnce(int a)
{
    int c = 2 * a - 1;
    return c;
}

int beingCalledMultiTimes(int a, int b)
{
    return a + b;
}

int caller(int a, int b)
{
    beingCalledOnce(a);
    beingCalledMultiTimes(a,b);
    beingCalledMultiTimes(a,b);
    beingCalledMultiTimes(a,b);
    return 0;
}
