#define CB_IMPLEMENTATION
#include <cb/cb.h>
#include <cb_extensions/cb_assert.h>

int main(void)
{
    const char* path = NULL;

    cb_init();

    cb_project("exe");
    cb_set(cb_BINARY_TYPE, cb_EXE);

    cb_add(cb_FILES, "src/main.c");
    cb_add(cb_FILES, "src/value.c");

    path = cb_bake();

    cb_assert_file_exists(path);
    
    cb_assert_run(path);

    cb_destroy();

    return 0;
}