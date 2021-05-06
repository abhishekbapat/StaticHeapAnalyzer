#include <stdlib.h>
#include <stdio.h>

int main()
{
    int *x = (int *)malloc(sizeof(int));
    int *y = (int *)malloc(sizeof(int));
    int *z;
    
    z = x;
    *y = 10;
    x = NULL;
    *z = 20;

    printf("Val: %d\n", *y);
    printf("Ptr: %p\n", z);
    
    return 0;
}
