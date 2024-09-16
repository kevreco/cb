#define CB_IMPLEMENTATION
#define CB_EXTENSIONS /* for cb_assert_xxx */
#include <cb/cb.h>

int main()
{
    cb_init();

    cb_project("foo");
    cb_set(cbk_BINARY_TYPE, cbk_static_lib);

    cb_add_file("src/int.c");
    cb_add_file("src/string.c");	

    const char* library_path = cb_bake("foo");

    cb_assert_file_exists(library_path);

    cb_destroy();

    return 0;
}