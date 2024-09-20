#define CB_IMPLEMENTATION
#define CB_EXTENSIONS /* for cb_assert_xxx */
#include <cb/cb.h>

int main()
{
    cb_init();

    cb_project("foo");
    cb_set(cbk_BINARY_TYPE, cbk_static_lib);

    cb_add(cbk_FILES, "src/int.c");
    cb_add(cbk_FILES, "src/string.c");

    cb_assert_file_exists(
        cb_bake("foo")
    );

    cb_destroy();

    return 0;
}

