#define CB_IMPLEMENTATION
#define CB_EXTENSIONS /* for cb_assert_xxx */
#include <cb/cb.h>

/* Use UTF-8 strings
   NOTE: NOTE: .dlls can only contains ANSI characters.
*/

int main()
{
    cb_init();

    /* Static library */
    {
        cb_project("foo_ぁ");
        cb_set(cbk_BINARY_TYPE, cbk_static_lib);

        cb_add_file("src_ぁ/foo_ぁ.c");

        cb_assert_file_exists(
            cb_bake("foo_ぁ")
        );
    }

    /* Shared library */
    {
        cb_project("bar"); /* NOTE: .dlls can only contains ANSI characters. */
        cb_set(cbk_BINARY_TYPE, cbk_shared_lib);

        cb_add_file("src_ぁ/bar_ぁ.c");

        cb_add(cbk_DEFINES, "BAR_LIB_EXPORT");

        cb_assert_file_exists(
            cb_bake("bar")
        );
    }

    /* exe */
    {
        cb_project("exe_ぁ");
        cb_set(cbk_BINARY_TYPE, cbk_exe);

        cb_add_file("src_ぁ/main_ぁ.c");

        cb_add(cbk_LINK_PROJECT, "foo_ぁ");
        cb_add(cbk_LINK_PROJECT, "bar");

        cb_assert_file_exists(
            cb_bake_and_run("exe_ぁ")
        );
    }

    cb_destroy();

    return 0;
}