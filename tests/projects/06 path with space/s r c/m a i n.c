#include <stdio.h>

#include "f o o.h"
#include "b a r.h"

int main()
{
    printf("Hello path with space - %d - %d\n", foo_value(), bar_value());

    return 0;
}