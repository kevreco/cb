#define CB_IMPLEMENTATION
#include <cb/cb.h>
#include <cb_extensions/cb_copy_directory.h>
#include <cb_extensions/cb_assert.h>

int main(void)
{
    cb_copy_directory("./source_directory", "./dest_directory");

    cb_assert_file_exists("./dest_directory/a.txt");
    cb_assert_file_exists("./dest_directory/b.txt");
    cb_assert_file_exists("./dest_directory/c.txt");

    return 0;
}


