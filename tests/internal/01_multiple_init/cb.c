#define CB_IMPLEMENTATION
#include <cb/cb.h>

void create(void)
{
    cb_init();
   
    cb_project("foo");
    cb_set("key", "value");

    cb_destroy();
}

int main(void)
{
    int i = 0;
    for (; i < 3; ++i)
    {
        create();
    }

    return 0;
}
