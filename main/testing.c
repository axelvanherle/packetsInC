#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char test[5][256] = {};
    strcpy(test[0],"TEST1");
    strcpy(test[1],"TEST2");
    strcpy(test[2],"TEST3");
    strcpy(test[3],"TEST4");
    strcpy(test[4],"TEST5");

    for (int i = 0; i <+ 5; i++)
    {
        printf("%s\n",test[i]);
    }
    

}