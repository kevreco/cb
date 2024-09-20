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
        cb_set(cb_BINARY_TYPE, cb_STATIC_LIBRARY);

        cb_add(cb_FILES, "src/foo.c");

        cb_assert_file_exists(
            cb_bake("foo")
        );
    }

    /* Shared library */
    {
        cb_project("bar");
        cb_set(cb_BINARY_TYPE, cb_SHARED_LIBRARY);

        cb_add(cb_FILES, "src/bar.c");

        cb_add(cb_DEFINES, "BAR_LIB_EXPORT");

        cb_assert_file_exists(
            cb_bake("bar")
        );
    }

    /* exe */
    {
        cb_project("exe");
        cb_set(cb_BINARY_TYPE, cb_EXE);

        cb_add(cb_FILES, "src/main.c");

        cb_add(cb_LINK_PROJECT, "foo");
        cb_add(cb_LINK_PROJECT, "bar");

        cb_assert_file_exists(
            cb_bake_and_run("exe")
        );

    }

    cb_destroy();

    return 0;
}