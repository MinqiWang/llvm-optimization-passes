int singleLoopWithLICM(int a, int b, int c)
{
	int i, ret = 0;

	do {
		c = b + 2;
		i++;
	} while (i < a);

	return ret + c;
}

int singleLoopWithOutLICM(int a, int b, int c)
{
	int i, ret = 0;

	do {
		c = b + c;
		i++;
	} while (i < a);

	return ret + c;
}

int nestedLoopWithLICM(int a, int b, int c)
{
	int i, j, ret = 0;

	do {
		do {
			c = b + 2;
			j++;
		} while (j < a);
		i++;
	} while (i < b);

	return ret + c;
}

int nestedLoopWithOutLICM(int a, int b, int c)
{
	int i, j, ret = 0;

	do {
		do {
			c = b + c;
			j++;
		} while (j < a);
		i++;
	} while (i < b);

	return ret + c;
}