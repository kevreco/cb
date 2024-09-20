#define CB_IMPLEMENTATION
#define CB_EXTENSIONS /* for cb_assert_xxx */
#include <cb/cb.h>

int main()
{
    const char* path = NULL;

    cb_init();

    cb_project("exe");
    cb_set(cb_BINARY_TYPE, cb_EXE);

    cb_add(cb_FILES, "src/main.c");
    cb_add(cb_FILES, "src/value.c");

    path = cb_bake("exe");

    cb_assert_file_exists(path);
    
    cb_assert_run(path);

    cb_destroy();

    return 0;
}