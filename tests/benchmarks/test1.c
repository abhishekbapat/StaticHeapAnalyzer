#include <stdlib.h>
#include <stdio.h>

int main()
{
    int *x = (int *)malloc(sizeof(int));
    int *y = (int *)malloc(sizeof(int));

    *y = 10;
    x = y;
    printf("Val: %d", *y);

    return 0;
}
