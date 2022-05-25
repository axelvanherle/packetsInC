#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	char str[] = "232.75048, 3,  0.364, -0.278, 9.443, 4, -1.222, 0.001, -6.664, 5, -465.654, -654.654, 150.644";
	double gyro[20];
	unsigned int whatever[4];
	double trash;


	sscanf(str, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",&trash,&trash,&gyro[0],&gyro[1],&gyro[2],&trash,&gyro[3],&gyro[4],&gyro[5],&trash,&gyro[6],&gyro[7],&gyro[8]);

	for (int i = 0; i < 9; i++)
	{
		printf("%lf\n",gyro[i]);
	}
	
}