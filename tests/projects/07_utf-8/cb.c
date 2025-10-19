#define CB_IMPLEMENTATION
#include <cb/cb.h>
#include <cb_extensions/cb_assert.h>

/* Use UTF-8 strings
   NOTE: NOTE: .dlls can only contains ANSI characters.
*/

int main(void)
{
    const char* path = NULL;

    cb_init();

    /* Static library */
    {
        cb_project("foo_ぁ");
        cb_set(cb_BINARY_TYPE, cb_STATIC_LIBRARY);

        cb_add(cb_FILES, "src_ぁ/foo_ぁ.c");

        path = cb_bake();

        cb_assert_file_exists(path);
    }

    /* Shared library */
    {
        cb_project("bar"); /* NOTE: .dlls can only contains ANSI characters. */
        cb_set(cb_BINARY_TYPE, cb_SHARED_LIBRARY);

        cb_add(cb_FILES, "src_ぁ/bar_ぁ.c");

        cb_add(cb_DEFINES, "BAR_LIB_EXPORT");

        path = cb_bake();

        cb_assert_file_exists(path);
    }

    /* exe */
    {
        cb_project("exe_ぁ");
        cb_set(cb_BINARY_TYPE, cb_EXE);

        cb_add(cb_FILES, "src_ぁ/main_ぁ.c");

        cb_add(cb_LINK_PROJECTS, "foo_ぁ");
        cb_add(cb_LINK_PROJECTS, "bar");

        path = cb_bake();

        cb_assert_file_exists(path);

        cb_assert_run(path);
    }

    cb_destroy();

    return 0;
}