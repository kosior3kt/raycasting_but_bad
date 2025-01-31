#include <stdio.h>
#include <math.h>

struct point
{
	float x, y;
};


static float deg2rad(float _deg)
{
	return (_deg * (M_PI/180));
}
int main()
{
	struct point p1;
	//struct point p2;
	p1.x = 1.0;
	p1.y = 0.0;

	//float theta = 1.5708;
	float theta = deg2rad(-90);

	float cs = cos(theta);
	float sn = sin(theta);

	float x = p1.x * cs - p1.y * sn;
	float y = p1.x * sn + p1.y * cs;

	printf("before x:%f, y:%f\n", p1.x, p1.y);
	printf("after x:%f, y:%f\n", x, y);


	return 0;
}
