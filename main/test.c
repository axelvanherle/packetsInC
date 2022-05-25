#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	char buffer[] = "232.75048, 3,  0.364, -0.278, 9.443, 4, -1.222, 0.001, -6.664, 5, -465.654, -654.654, 150.644";
	double trash;
	double stats1[3];
	double stats2[3];
	double stats3[3];

	sscanf(buffer, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",&trash,&trash,&stats1[0],&stats1[1],&stats1[2],&trash,&stats2[0],&stats2[1],&stats2[2],&trash,&stats3[0],&stats3[1],&stats3[2]);

	for (int i = 0; i < 3; i++)
	{
		printf("%lf\n",stats1[i]);
	}
	for (int i = 0; i < 3; i++)
	{
		printf("%lf\n",stats2[i]);
	}
	for (int i = 0; i < 3; i++)
	{
		printf("%lf\n",stats3[i]);
	}
		
	
}