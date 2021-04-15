#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <assert.h>

typedef struct node {
    struct node *next;
    int val;
} Node;


int main()
{
    Node *l = NULL;
    int min = INT_MAX, max = -INT_MAX;
    int j = 0;
 
    while (j < 10000) {
        Node *p = malloc(sizeof(*p));
        p->val = 20000;

        p->next = l;
        l = p;
        
        if (min > p->val) {
            min = p->val;
        }
        if (max < p->val) {
            max = p->val;
        }
        j++;
    }

    return 0;
}
