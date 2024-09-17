#include <stdio.h>

#include "foo.h"
#include "bar.h"

int main()
{
    printf("Hello exe with dependencies - %d - %d\n", foo_value(), bar_value());

    return 0;
}