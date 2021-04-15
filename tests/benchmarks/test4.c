#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main() 
{
    int *a = malloc(sizeof(int));
    int *b = malloc(sizeof(int));

    *a = 10;
    *b = 20;

    int r1 = rand();
    int r2 = rand();

    printf("r1: %d\n", r1);
    printf("r2: %d\n", r2);

    if ( r1 < r2)
    {
        printf("Val : %d\n", *a);
    }
    else 
    {
        printf("Val : %d\n", *b);
    }

    return *b;
}