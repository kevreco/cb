#define CB_IMPLEMENTATION
#include <cb/cb.h>

int main()
{
    cb_init();

    cb_project("foo");
    cb_set(cbk_BINARY_TYPE, cbk_static_lib);

    cb_add_file("src/int.c");
    cb_add_file("src/string.c");	

    cb_bool result = cb_bake(cb_toolchain_default(), "foo");

    cb_destroy();

	return result ? 0 : -1;
}