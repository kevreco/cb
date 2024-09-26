#define CB_IMPLEMENTATION
#include <cb/cb.h>
#include <cb_extensions/cb_assert.h>

/* Create an executable linked against a static library and a shared library. */

int main(void)
{
    const char* path = NULL;

    cb_init();

    /* Static library */
    {
        cb_project("foo");
        cb_set(cb_BINARY_TYPE, cb_STATIC_LIBRARY);

        cb_add(cb_FILES, "src/foo.c");

        path = cb_bake();

        cb_assert_file_exists(path);
    }

    /* Shared library */
    {
        cb_project("bar");
        cb_set(cb_BINARY_TYPE, cb_SHARED_LIBRARY);

        cb_add(cb_FILES, "src/bar.c");

        cb_add(cb_DEFINES, "BAR_LIB_EXPORT");

        path = cb_bake();

        cb_assert_file_exists(path);
    }

    /* exe */
    {
        cb_project("exe");
        cb_set(cb_BINARY_TYPE, cb_EXE);

        cb_add(cb_FILES, "src/main.c");

        cb_add(cb_LINK_PROJECTS, "foo");
        cb_add(cb_LINK_PROJECTS, "bar");

        path = cb_bake();

        cb_assert_file_exists(path);

        cb_assert_run(path);
    }

    cb_destroy();

    return 0;
}