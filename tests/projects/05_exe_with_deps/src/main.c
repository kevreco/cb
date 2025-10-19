#include <stdio.h>

#include "foo.h"
#include "bar.h"

int main()
{
    printf("Hello exe with dependencies - %d - %d\n", bar_value(), foo_value());

    if (bar_value() != 42)
        return 42;
    
    if (bar_b_ar_value() != 43)
        return 43;
    
    if (bar_bar_value() != 44)
        return 44;
    
    if (foo_value() != 45)
        return 45;
    
    if (foo_f_oo_value() != 46)
        return 46;
    
    if (foo_foo_value() != 47)
        return 47;
    
    return 0;
}