#define CB_IMPLEMENTATION
#include <cb/cb.h>
#include <cb_extensions/cb_assert.h>

int main(void)
{
    cb_init();

    cb_project("foo");
    cb_add("key", "value");

    cb_assert_true(cb_contains("key", "value"));

    cb_project("bar");

    cb_assert_false(cb_contains("key", "value"));

    cb_project_f("%s", "foo");

    cb_assert_true(cb_contains("key", "value"));

    cb_destroy();

    return 0;
}
