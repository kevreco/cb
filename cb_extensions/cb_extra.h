#ifndef CB_EXTRA_H
#define CB_EXTRA_H

/* To know if a project has already been built.
   Must be used right before cb_bake-like functions.
   NOTE:
     This rely on internal behavior of cb.
     Use with caution!
*/

CB_API cb_bool
cb_baked_binary_already_exists();

CB_API cb_bool
cb_baked_binary_already_exists_with(const char* project_name, cb_toolchain_t toolchain);

#endif /* CB_EXTRA_H */

#ifdef CB_IMPLEMENTATION

#ifndef CB_EXTRA_IMPL
#define CB_EXTRA_IMPL

/*-----------------------------------------------------------------------*/
/* API implementation */
/*-----------------------------------------------------------------------*/
CB_API cb_bool
cb_baked_binary_already_exists()
{
    cb_toolchain_t toolchain = cb_toolchain_get();
    const char* project_name = cb_current_project_name();
    return cb_baked_binary_already_exists_with(project_name, toolchain);
}

CB_API cb_bool
cb_baked_binary_already_exists_with(const char* project_name, cb_toolchain_t toolchain)
{
    const char* ext = "<unknown>";
    cb_size anchor = 0;
    const char* output_dir = NULL;
    const char* artefact = "";
    cb_bool file_exists = cb_false;

    cb_project_t* project = cb_project(project_name);

    anchor = cb_tmp_save();
 
    output_dir = cb_get_output_directory(project, &toolchain);
        
    if (cb_str_equals(toolchain.family, "msvc"))
    {
        if (cb_property_equals(project, cb_BINARY_TYPE, cb_EXE))
             ext = ".exe";
        else if (cb_property_equals(project, cb_BINARY_TYPE, cb_SHARED_LIBRARY))
             ext = ".dll";
        else if (cb_property_equals(project, cb_BINARY_TYPE, cb_STATIC_LIBRARY))
             ext = ".lib";
         
        artefact = cb_tmp_sprintf("%s%s%s", output_dir, project_name, ext);
    }
    else if (cb_str_equals(toolchain.family, "gcc"))
    {
        if (cb_property_equals(project, cb_BINARY_TYPE, cb_EXE))
        {
            ext = "";
            artefact = cb_tmp_sprintf("%s%s%s", output_dir, project_name, ext);
        }
        else if (cb_property_equals(project, cb_BINARY_TYPE, cb_SHARED_LIBRARY))
        {
            ext = ".so";
            artefact = cb_tmp_sprintf("%slib%s%s", output_dir, project_name, ext);
        }
        else if (cb_property_equals(project, cb_BINARY_TYPE, cb_STATIC_LIBRARY))
        {
            ext = ".a";
            artefact = cb_tmp_sprintf("%slib%s%s", output_dir, project_name, ext);
        }
    }

    file_exists = cb_path_exists(artefact);
    
    cb_tmp_restore(anchor);
    
    return file_exists;
}

#endif /* CB_EXTRA_IMPL */

#endif /* CB_IMPLEMENTATION */

