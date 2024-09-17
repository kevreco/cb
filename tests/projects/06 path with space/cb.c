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

        cb_add_file("s r c/f o o.c");

        const char* binary_path = cb_bake("f o o");

        cb_assert_file_exists(binary_path);
    }

    /* Shared library */
    {
        cb_project("b a r");
        cb_set(cbk_BINARY_TYPE, cbk_shared_lib);

        cb_add_file("s r c/b a r.c");

        cb_add(cbk_DEFINES, "BAR_LIB_EXPORT");

        const char* binary_path = cb_bake("b a r");

        cb_assert_file_exists(binary_path);
    }

    /* exe */
    {
        cb_project("e x e");
        cb_set(cbk_BINARY_TYPE, cbk_exe);

        cb_add_file("s r c/m a i n.c");

        cb_add(cbk_LINK_PROJECT, "f o o");
        cb_add(cbk_LINK_PROJECT, "b a r");
       
        const char* binary_path = cb_bake_and_run("e x e");

        cb_assert_file_exists(binary_path);
    }

    cb_destroy();

    return 0;
}