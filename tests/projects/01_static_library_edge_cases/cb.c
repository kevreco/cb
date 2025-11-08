#define CB_IMPLEMENTATION
#include <cb/cb.h>
#include <cb_extensions/cb_assert.h>

int main(void)
{
    cb_init();

    cb_project("foo");
    cb_set(cb_BINARY_TYPE, cb_STATIC_LIBRARY);

    cb_add(cb_FILES, "src/int.c");
    cb_add(cb_FILES, "src/string.c");

    cb_assert_file_exists(
        cb_bake()
    );

    cb_destroy();

    return 0;
}

