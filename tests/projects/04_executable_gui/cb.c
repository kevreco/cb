#define CB_IMPLEMENTATION
#include <cb/cb.h>
#include <cb_extensions/cb_extensions.h>

int main(void)
{

/* @FIXME Only compile this for windows for now, because the linux version require the X library. */
#ifdef _WIN32
    const char* path = NULL;

    cb_init();

    cb_project("gui");
    cb_set(cb_BINARY_TYPE, cb_EXE);

    cb_add(cb_FILES, "src/main.c");

    path = cb_bake("gui");

    cb_assert_file_exists(path);

    /* NOTE: We don't want to use cb_run since running this app displays a window. */
    /* cb_assert_run(path); */

    cb_destroy();

#endif /* _WIN32 */

    return 0;
}