#define CB_IMPLEMENTATION
#define CB_EXTENSIONS /* for cb_assert_xxx */
#include <cb/cb.h>

/* Create an executable linked against a static library and a shared library. */

int main()
{
    cb_init();

    /* Static library */
    {
        cb_project("foo");
        cb_set(cbk_BINARY_TYPE, cbk_static_lib);

        cb_add_file("src/foo.c");

        const char* binary_path = cb_bake("foo");

        cb_assert_file_exists(binary_path);
    }

    /* Shared library */
    {
        cb_project("bar");
        cb_set(cbk_BINARY_TYPE, cbk_shared_lib);

        cb_add_file("src/bar.c");

        cb_add(cbk_DEFINES, "BAR_LIB_EXPORT");

        const char* binary_path = cb_bake("bar");

        cb_assert_file_exists(binary_path);
    }

    /* exe */
    {
        cb_project("exe");
        cb_set(cbk_BINARY_TYPE, cbk_exe);

        cb_add_file("src/main.c");

        cb_add(cbk_LINK_PROJECT, "foo");
        cb_add(cbk_LINK_PROJECT, "bar");
       
        const char* binary_path = cb_bake_and_run("exe");

        cb_assert_file_exists(binary_path);
    }

    cb_destroy();

    return 0;
}