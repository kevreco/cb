#include <stdio.h>

#include "foo.h"
#include "bar.h"

int main()
{
    printf("Hello exe with dependencies - %d - %d\n", foo_value(), bar_value(), bar_bar_value(), bar_b_ar_value());

    if (foo_value() != 42)
        return 42;
    
    if (bar_value() != 43)
        return 43;
    
    if (bar_b_ar_value() != 44)
        return 44;
    
    if (bar_bar_value() != 45)
        return 45;
    
    return 0;
}