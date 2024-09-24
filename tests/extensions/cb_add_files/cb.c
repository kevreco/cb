#define CB_IMPLEMENTATION
#include <cb/cb.h>
#include <cb_extensions/cb_add_files.h>
#include <cb_extensions/cb_assert.h>

#define assert_wildmatch(left, right) \
    do { \
        cb_assert_true(cb_wildmatch(left, right)); \
    } while(0)

#define assert_no_match(left, right) \
    do { \
        cb_assert_false(cb_wildmatch(left, right)); \
    } while(0)

#define assert_contains(key, value) \
    do { \
        cb_assert_true(cb_contains(key, value)); \
    } while(0)

void assert_wildmatches(void)
{
    assert_wildmatch("", "");
    assert_wildmatch("a", "a");
    assert_wildmatch("ぁ", "ぁ");
    assert_wildmatch("*", "ぁ");
    assert_wildmatch("*", "a");
    assert_wildmatch("*.c", "a.c");
    assert_wildmatch("\\*.c", "\\a.c");
    assert_wildmatch("\\**.c", "\\a.c");
    assert_wildmatch("/**.c", "\\a.c");
    assert_wildmatch("a/**.c", "a/a.c");
    assert_wildmatch("a\\**.c", "a/a.c");
    assert_wildmatch("*.c", ".\\src\\tester\\a.c");
    assert_wildmatch("*src*.c", ".\\src\\tester\\a.c");
    assert_wildmatch("*\\src\\*.c", ".\\src\\tester\\a.c");

    assert_no_match("", "a");
    assert_no_match("a", "");
    assert_no_match("aa", "a/a");
}

int main(void)
{
    assert_wildmatches();

    cb_init();
    
    /* Non-recursive. */
    {
        cb_project("foo");

        cb_add_files(".", "*.txt");
        cb_add_files("./folder/", "*.txt");

        assert_contains(cb_FILES, "./a.txt");
        assert_contains(cb_FILES, "./b.txt");
        assert_contains(cb_FILES, "./c.txt");
        assert_contains(cb_FILES, "./folder/e.txt");
        assert_contains(cb_FILES, "./folder/f.txt");
    }
    
    /* Recursive. */
    {
        cb_project("bar");

        cb_add_files_recursive(".", "*.txt");

        assert_contains(cb_FILES, "./a.txt");
        assert_contains(cb_FILES, "./b.txt");
        assert_contains(cb_FILES, "./c.txt");
        assert_contains(cb_FILES, "./folder/e.txt");
        assert_contains(cb_FILES, "./folder/f.txt");
    }

    cb_destroy();

    return 0;
}


