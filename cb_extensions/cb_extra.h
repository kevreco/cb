#ifndef CB_EXTRA_H
#define CB_EXTRA_H

CB_API const char*
cb_baked_binary_exists(const char* project_name, cb_toolchain_t toolchain);

#endif /* CB_EXTRA_H */

#ifdef CB_IMPLEMENTATION

#ifndef CB_EXTRA_IMPL
#define CB_EXTRA_IMPL

/*-----------------------------------------------------------------------*/
/* API implementation */
/*-----------------------------------------------------------------------*/

CB_API const char*
cb_baked_binary_exists(const char* project_name, cb_toolchain_t toolchain)
{
    const char* ext = NULL;

    if (cb_str_equals(toolchain.family, "msvc"))
    {
        if (!ext && cb_property_equals(project, cb_BINARY_TYPE, cb_EXE))
        {
             ext = ".exe";
        }
        
        if (!ext && cb_property_equals(project, cb_BINARY_TYPE, cb_SHARED_LIBRARY))
        {
             ext = ".dll";
        }
        
        if (!ext && cb_property_equals(project, cb_BINARY_TYPE, cb_STATIC_LIBRARY))
        {
             ext = ".lib";
        }
    }
    else if (cb_str_equals(toolchain.family, "gcc"))
    {
        if (!ext && cb_property_equals(project, cb_BINARY_TYPE, cb_EXE))
        {
             ext = "";
        }
        
        if (!ext && cb_property_equals(project, cb_BINARY_TYPE, cb_SHARED_LIBRARY))
        {
             ext = ".so";
        }
        
        if (!ext && cb_property_equals(project, cb_BINARY_TYPE, cb_STATIC_LIBRARY))
        {
             ext = ".a";
        }
    }

    const char* output_dir = cb_get_output_directory(project, &toolchain);
    
    cb_size anchor = cb_tmp_save();
	
    const char* artefact = cb_tmp_sprintf("%s%s%s", output_dir, project_name, ext);
    
    cb_bool file_exists = cb_path_exists();
    
    cb_tmp_restore(anchor);
    
    return file_exists;
}

#endif /* CB_EXTRA_IMPL */

#endif /* CB_IMPLEMENTATION */

