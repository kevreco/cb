#define CB_IMPLEMENTATION
#include <cb/cb.h>
#include <cb_extensions/cb_assert.h>

int main(void)
{
    const char* path = NULL;
    const char* output = NULL;

    cb_process_handle* handle;
    cb_init();

    cb_project("exe");
    cb_set(cb_BINARY_TYPE, cb_EXE);

    cb_add(cb_FILES, "src/main.c");
    cb_add(cb_FILES, "src/value.c");

    path = cb_bake();

    cb_assert_file_exists(path);
    
    handle = cb_process_to_string(path, NULL, 0);

    output = cb_process_stdout_string(handle);

#ifdef _WIN32
    /* Windows default mode for stdout is "text" which translate \n to \r\n. */
    cb_assert_true(cb_str_equals(output, "Hello Exe - 44\r\n"));
#else
    cb_assert_true(cb_str_equals(output, "Hello Exe - 44\n"));
#endif

    cb_assert_true(cb_process_end(handle) == 0);

    cb_destroy();

    return 0;
}