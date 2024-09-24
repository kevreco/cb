#define CB_IMPLEMENTATION
#include <cb/cb.h>
#include <cb_extensions/cb_assert.h>

#define assert_property_values_count(count) \
    do { \
        cb_project_t* p = cb_current_project(); \
        cb_assert_int_equals((int)count, (int)cb_darrT_size(&p->mmap)); \
    } while(0)

int main(void)
{
    cb_init();

    cb_project("foo");
    cb_set("key", "value");

    assert_property_values_count(1);

    cb_add("key", "value2");

    assert_property_values_count(2);

    cb_add("key2", "value3");

    assert_property_values_count(3);

    cb_remove_all("key");

    assert_property_values_count(1);

    cb_remove_all("key2");

    assert_property_values_count(0);

    cb_add("key", "value4");
    cb_add("key", "value5");

    assert_property_values_count(2);

    cb_set("key", "value6");

    cb_destroy();

    return 0;
}
