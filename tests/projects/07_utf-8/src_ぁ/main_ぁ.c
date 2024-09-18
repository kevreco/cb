#include <stdio.h>

#include "foo_ぁ.h"
#include "bar_ぁ.h"

int main()
{
    printf("Hello utf-8 ぁ - %d - %d\n", foo_value(), bar_value());

    return 0;
}