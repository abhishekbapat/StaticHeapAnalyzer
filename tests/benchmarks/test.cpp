#include <stdlib.h>
#include <stdio.h>

typedef struct test
{
    struct test *t;
    int a;
    int b;
} Test;


int main()
{
    Test *p = (Test *)malloc(sizeof(Test));
    free(p);

    Test *a = (Test *)calloc(1, sizeof(Test));

    p = a;
    printf("Ptr: %d", p->a);

    return 0;
}
