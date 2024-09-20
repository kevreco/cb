#define CB_IMPLEMENTATION
#define CB_EXTENSIONS /* for cb_assert_xxx */
#include <cb/cb.h>

int main()
{
    cb_init();

    cb_project("exe");
    cb_set(cb_BINARY_TYPE, cb_EXE);

    cb_add(cb_FILES, "src/main.c");
    cb_add(cb_FILES, "src/value.c");

    cb_assert_file_exists(
        cb_bake_and_run("exe")
    );

    cb_destroy();

    return 0;
}