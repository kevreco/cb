#define CB_IMPLEMENTATION
#define CB_EXTENSIONS /* for cb_assert_xxx */
#include <cb/cb.h>

int main(void)
{
    cb_init();

    cb_project("bar");
    cb_set(cb_BINARY_TYPE, cb_SHARED_LIBRARY);

    cb_add(cb_FILES, "src/int.c");
    cb_add(cb_FILES, "src/string.c");

    cb_assert_file_exists(
        cb_bake("bar")
    );

    cb_destroy();

    return 0;
}