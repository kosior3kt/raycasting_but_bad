#include <stdio.h>

struct point
{
	double x, y;
};

int main()
{

	struct point P, Q;
	P.x = 240;
	P.y = 320;
	Q.x = 119;
	Q.y = 331;

	double a = Q.y - P.y;
	double b = P.x - Q.x;
	double c = a * (double)P.x + b * (double)P.y;

	// ax + by = c
	// y = ( c - ax )/b
	double temp_y = (c - (a))/b;

	double x = 0;

	printf("a: %f, b: %f, c: %f\n", a, b, c);
	scanf("%d", &x);
	double y = (c - (a * x))/b;
	printf("y: %f, x: %d\n", y, x);

	return 0;
}
