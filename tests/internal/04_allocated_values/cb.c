#define CB_API
#define CB_IMPLEMENTATION
#include <cb/cb.h>
#include <cb_extensions/cb_assert.h>

void assert_strings_are_within_tmp_buffer(void)
{
    cb_kv_range projects_range;
    cb_kv current_project;
    cb_kv_range properties_range;
    cb_kv current_property;

    cb_project_t* p = NULL;
    cb_context* ctx = cb_current_context();

    projects_range = cb_mmap_get_range_all(&ctx->projects);
    while (cb_mmap_range_get_next(&projects_range, &current_project))
    {
        p = (cb_project_t*)current_project.u.ptr;

        /* Name of project is within the tmp buffer */
        cb_assert_true(cb_tmp_contains(p->name.data));

        properties_range = cb_mmap_get_range_all(&p->mmap);
        while (cb_mmap_range_get_next(&properties_range, &current_property))
        {
            /* Key is within the tmp buffer */
            cb_assert_true(cb_tmp_contains(current_property.key.data));
            /* Value is within the tmp buffer */
            cb_assert_true(cb_tmp_contains(current_property.u.strv.data));
        }
    }
}

int main(void)
{
    cb_init();

    cb_project("foo");
    cb_set(cb_BINARY_TYPE, cb_STATIC_LIBRARY);

    cb_add(cb_FILES, "src/int.c");
    cb_add(cb_FILES, "src/string.c");

    assert_strings_are_within_tmp_buffer();

    cb_destroy();

    return 0;
}

