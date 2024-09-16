#define CB_IMPLEMENTATION
#define CB_EXTENSIONS /* for cb_assert_xxx */
#include <cb/cb.h>

int main()
{
    cb_init();

    cb_project("gui");
    cb_set(cbk_BINARY_TYPE, cbk_exe);

    cb_add_file("src/main.c");

    /* NOTE: We don't want to use cb_bake_and_run the executable since it displays a windows. */
    const char* binary_path = cb_bake("gui");

    cb_assert_file_exists(binary_path);

    cb_destroy();

    return 0;
}