#define CB_IMPLEMENTATION
#define CB_EXTENSIONS /* for cb_assert_xxx */
#include <cb/cb.h>

int main()
{
    cb_init();

    cb_project("exe");
    cb_set(cbk_BINARY_TYPE, cbk_exe);

    cb_add(cbk_FILES, "src/main.c");
    cb_add(cbk_FILES, "src/value.c");

    cb_assert_file_exists(
        cb_bake_and_run("exe")
    );

    cb_destroy();

    return 0;
}