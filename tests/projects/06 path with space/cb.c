#define CB_IMPLEMENTATION
#define CB_EXTENSIONS /* for cb_assert_xxx */
#include <cb/cb.h>

/* Create an executable linked against a static library and a shared library. */

int main()
{
    cb_init();

    /* Static library */
    {
        cb_project("f o o");
        cb_set(cbk_BINARY_TYPE, cbk_static_lib);

        cb_add(cbk_FILES, "s r c/f o o.c");

        cb_assert_file_exists(
            cb_bake("f o o")
        );
    }

    /* Shared library */
    {
        cb_project("b a r");
        cb_set(cbk_BINARY_TYPE, cbk_shared_lib);

        cb_add(cbk_FILES, "s r c/b a r.c");

        cb_add(cbk_DEFINES, "BAR_LIB_EXPORT");

        cb_assert_file_exists(
            cb_bake("b a r")
        );
    }

    /* exe */
    {
        cb_project("e x e");
        cb_set(cbk_BINARY_TYPE, cbk_exe);

        cb_add(cbk_FILES, "s r c/m a i n.c");

        cb_add(cbk_LINK_PROJECT, "f o o");
        cb_add(cbk_LINK_PROJECT, "b a r");

        cb_assert_file_exists(
            cb_bake_and_run("e x e")
        );
    }

    cb_destroy();

    return 0;
}