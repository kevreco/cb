#define CB_IMPLEMENTATION
#define CB_EXTENSIONS /* for cb_assert_xxx */
#include <cb/cb.h>

int main()
{

/* @FIXME Only compile this for windows for now, because the linux version require the X library. */
#ifdef _WIN32

    cb_init();

    cb_project("gui");
    cb_set(cbk_BINARY_TYPE, cbk_exe);

    cb_add_file("src/main.c");

    /* NOTE: We don't want to use cb_bake_and_run the executable since it displays a windows. */
    cb_assert_file_exists(
        cb_bake("gui")
    );

#endif /* _WIN32 */

    return 0;
}