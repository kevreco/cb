#ifndef CB_DUMP_H
#define CB_DUMP_H

/*
   Display all keys and values of all projects.
*/

#ifdef __cplusplus
extern "C" {
#endif

/* Display properties of all project into a string. */
CB_API const char* cb_dump_to_str(void);

/* Display properties of all project into a file. */
CB_API void cb_dump_to_file(FILE* file);

/* Same as cb_dump_to_file using stdout as file. */
CB_API void cb_dump(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CB_DUMP_H */

#ifdef CB_IMPLEMENTATION

CB_API const char*
cb_dump_to_str(void)
{
    cb_dstr str;
    const char* result;
    cb_kv_range projects_range;
    cb_kv current_project;
    cb_kv_range properties_range;
    cb_kv current_property;

    cb_project_t* p = NULL;
    cb_context* ctx = cb_current_context();

    cb_dstr_init(&str);

    projects_range = cb_mmap_get_range_all(&ctx->projects);
    while (cb_mmap_range_get_next(&projects_range, &current_project))
    {
        p = (cb_project_t*)current_project.u.ptr;

        cb_dstr_append_f(&str, "Project '%s'\n", p->name.data);
        properties_range = cb_mmap_get_range_all(&p->mmap);
        while (cb_mmap_range_get_next(&properties_range, &current_property))
        {
            cb_dstr_append_f(&str, "%s : %s \n", current_property.key.data, current_property.u.strv.data);
        }
    }
    result = cb_tmp_str(str.data);

    cb_dstr_destroy(&str);

    return result;
}

CB_API void
cb_dump_to_file(FILE* file)
{
    fprintf(file, "%s", cb_dump_to_str());
}

CB_API void
cb_dump(void)
{
    fprintf(stdout, "%s", cb_dump_to_str());
}

#endif /* CB_IMPLEMENTATION */