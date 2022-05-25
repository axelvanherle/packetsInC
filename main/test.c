#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	char str[] = "2.26465,   	5.654,7.654,8.654";
	double gyro[4];
	unsigned int whatever[4];
	double trash;


	sscanf(str, "%lf,%lf,%lf,%lf",&gyro[0],&gyro[1],&gyro[2],&gyro[3]);

	for (int i = 0; i < 4; i++)
	{
		printf("%lf\n",gyro[i]);
	}
	
}