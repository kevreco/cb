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
        cb_project("f o o");
        cb_set(cb_BINARY_TYPE, cb_STATIC_LIBRARY);

        cb_add(cb_FILES, "s r c/f o o.c");

        path = cb_bake("f o o");

        cb_assert_file_exists(path);
    }

    /* Shared library */
    {
        cb_project("b a r");
        cb_set(cb_BINARY_TYPE, cb_SHARED_LIBRARY);

        cb_add(cb_FILES, "s r c/b a r.c");

        cb_add(cb_DEFINES, "BAR_LIB_EXPORT");

        path = cb_bake("b a r");

        cb_assert_file_exists(path);
    }

    /* exe */
    {
        cb_project("e x e");
        cb_set(cb_BINARY_TYPE, cb_EXE);

        cb_add(cb_FILES, "s r c/m a i n.c");

        cb_add(cb_LINK_PROJECTS, "f o o");
        cb_add(cb_LINK_PROJECTS, "b a r");

        path = cb_bake("e x e");

        cb_assert_file_exists(path);

        cb_assert_run(path);
    }

    cb_destroy();

    return 0;
}