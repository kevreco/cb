#define CB_IMPLEMENTATION
#include <cb/cb.h>

void assert_current_project(const char* project_name)
{
    cb_project_t* p = cb_current_project();
    if (!cb_strv_equals_str(p->name, project_name))
    {
        exit(1);
    }
}

int main(void)
{
    cb_init();

    {
        cb_project("foo");
        cb_set("key", "value");
    }
    assert_current_project("foo");

    {
        cb_project("bar");
        cb_set("key", "value");
    }
    assert_current_project("bar");

    cb_destroy();
    return 0;
}
