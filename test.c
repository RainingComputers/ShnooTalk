#include "stdio.h"

int main()
{
    char foo[1024];

    int bar = scanf("%1023s", foo);

    printf("%s\n", foo);
    printf("%d\n", bar);
    printf("%c\n", foo[4]);

    return 0;
}
