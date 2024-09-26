#define CB_IMPLEMENTATION
#include <cb/cb.h>
#include <cb_extensions/cb_assert.h>

int main(void)
{
    const char* path = NULL;

    cb_init();

    cb_project("gui");
    cb_set(cb_BINARY_TYPE, cb_EXE);

#ifdef _WIN32
    cb_add(cb_FILES, "src/win32_main.c");
    
    cb_add_many_vnull(cb_LIBRARIES,
        "User32",
        NULL);
#else
    cb_add(cb_FILES, "src/x11_main.c");

    cb_add_many_vnull(cb_LIBRARIES,
        "X11",
        NULL);
#endif

    path = cb_bake();

    cb_assert_file_exists(path);

    cb_destroy();

    return 0;
}