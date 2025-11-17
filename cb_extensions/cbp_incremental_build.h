/*

This plugin depends on:

  cb_dep_parser.h (Unix)
  cb_hash.h
  cb_file_io.h
  cb_file_info.h
  cb_file_it.h

*/

#ifndef CB_PLUGIN_INCREMENTAL_BUILD_H
#define CB_PLUGIN_INCREMENTAL_BUILD_H

#include "cb_dep_parser.h"
#include "cb_hash.h"
#include "cb_file_io.h"
#include "cb_file_info.h"
#include "cb_file_it.h"

#ifndef CB_SSCANF
#ifdef _WIN32
#define CB_SSCANF sscanf_s
#else
#define CB_SSCANF sscanf
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cbp_incremental_build cbp_incremental_build;
struct cbp_incremental_build
{
    /* Plugin base, must stay at the top */
    cb_plugin plugin;
    
    /* Current toolchain */
    cb_toolchain_t toolchain;
    /* Current project */
    cb_project_t* project;
    
    /* When compiler flags or preprocessor defines are changed we need to do a full rebuild */
    cb_bool needs_full_rebuild;
    
    /* Some statistics. Reset each run. */
    int stat_ignored;
    int stat_compilable;
};

CB_API void cbp_incremental_build_init(cbp_incremental_build* plugin);

/* Was created for the testing purpose to ensure that the tests run without cache. */
CB_API void cbp_incremental_build_delete_cache(cbp_incremental_build* plugin);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CB_PLUGIN_INCREMENTAL_BUILD_H */

#ifdef CB_IMPLEMENTATION

#ifndef CB_PLUGIN_INCREMENTAL_BUILD_IMPL
#define CB_PLUGIN_INCREMENTAL_BUILD_IMPL

typedef struct cb_tmp_strv_handle cb_tmp_strv_handle;
struct cb_tmp_strv_handle {
    cb_strv strv;
    cb_size anchor;
};

CB_INTERNAL void cbp_ib_bake_starting(cb_plugin* plugin);
CB_INTERNAL const char* cbp_ib_extra_argument(cb_plugin* plugin);
CB_INTERNAL cb_bool cbp_ib_can_process_file(cb_plugin* plugin, const char* file);
CB_INTERNAL void cbp_ib_file_processed(cb_plugin* plugin, const char* file, const char* std_out, const char* std_err);

/* Write a formated file info line into the dep store file. */
CB_INTERNAL void cbp_ib_dep_store_write_info(FILE* dep_store_file, const char* file_to_record);
/* Read a formated file info line from the dep store file. 
   Example of line:
     my/path/file.c;0123;0123;0123\r\n
*/
CB_INTERNAL cb_bool cbp_ib_dep_store_read_info(FILE* dep_store_file, char* buffer, int buffer_size, cb_file_info* file_info);

CB_INTERNAL cb_bool cbp_ib_check_full_rebuild_needed(cbp_incremental_build* ib);

CB_INTERNAL cb_tmp_strv_handle cbp_ib_format_dep_folder(const cb_toolchain_t* toolchain, const cb_project_t* project);
CB_INTERNAL cb_tmp_strv_handle cbp_ib_format_dep_store_filepath(cbp_incremental_build* ib, const char* filepath);

/*-----------------------------------------------------------------------*/
/* API implementation */
/*-----------------------------------------------------------------------*/

CB_API void cbp_incremental_build_init(cbp_incremental_build* ib)
{
    memset(ib, 0, sizeof(cbp_incremental_build));
    ib->plugin.name = "cbp_incremental_build";

    ib->plugin.bake_starting = cbp_ib_bake_starting;
    ib->plugin.can_process_file = cbp_ib_can_process_file;
    ib->plugin.extra_argument = cbp_ib_extra_argument;
    ib->plugin.file_processed = cbp_ib_file_processed;
}

CB_API void cbp_incremental_build_delete_cache(cbp_incremental_build* ib)
{
    cb_toolchain_t toolchain = { 0 };
    cb_project_t* project = NULL;
    cb_tmp_strv_handle handle = { 0 };
    cb_file_it it = { 0 };
    CB_UNUSED(ib);
     
    toolchain = cb_toolchain_get();
    project = cb_current_project();
    
    handle = cbp_ib_format_dep_folder(&toolchain, project);

    /* Remove all files from the dep directory */
    cb_file_it_init_recursive(&it, handle.strv.data);

    while(cb_file_it_get_next(&it))
    {
        const char* file = cb_file_it_current_file(&it);
        cb_bool deleted = cb_delete_file(file);
        CB_ASSERT(deleted);
    }
    
    cb_file_it_destroy(&it);

    cb_tmp_restore(handle.anchor);
}

CB_INTERNAL void cbp_ib_bake_starting(cb_plugin* plugin)
{
    cbp_incremental_build* ib = (cbp_incremental_build*)plugin;
    ib->stat_ignored = 0;
    ib->stat_compilable = 0;
    
    /* Reference current toolchain and project. */
    ib->toolchain = cb_toolchain_get();
    ib->project = cb_current_project();

    /* Ensure that the cache folder exists. */
    {
        /* Format directory in tmp allocator. */
        cb_tmp_strv_handle dir_handle = cbp_ib_format_dep_folder(&ib->toolchain, ib->project);

        /* Created directories */
        cb_create_directories(dir_handle.strv.data, dir_handle.strv.size);

        /* Release tmp memory */
        cb_tmp_restore(dir_handle.anchor);
    } 
    
    ib->needs_full_rebuild = cbp_ib_check_full_rebuild_needed(ib);
}

CB_INTERNAL const char* cbp_ib_extra_argument(cb_plugin* plugin)
{
    cbp_incremental_build* ib = (cbp_incremental_build*)plugin;

    if (cb_str_equals(ib->toolchain.family, "msvc"))
    {
        return "/showIncludes ";
    }
    else if (cb_str_equals(ib->toolchain.family, "gcc"))
    {
        return " -MMD ";
    }
    else
    {
        CB_ASSERT(0 && "Unhandled toolchain for cbp_incremental_build");
    }
    return "";
}

CB_INTERNAL cb_bool cbp_ib_can_process_file(cb_plugin* plugin, const char* file)
{
    cbp_incremental_build* ib = (cbp_incremental_build*)plugin;

    cb_bool file_need_to_be_compiled = cb_true;
 
    cb_tmp_strv_handle handle = {0};
    
    int file_info_flags = 0;
 
    const char* dep_store_filepath = NULL;
    
    if (!ib->needs_full_rebuild)
    {
        file_need_to_be_compiled = cb_false;
        
        handle = cbp_ib_format_dep_store_filepath(ib, file);
        dep_store_filepath = handle.strv.data;
        
        /* If the dep_file does not exists, the original file needs to be processed. */
        if (!cb_path_exists(dep_store_filepath))
        {
            file_need_to_be_compiled = cb_true;
        }
        else
        {
            /* Open dep_store_file */
            FILE* dep_store_file = cb_file_open_readonly(dep_store_filepath);
            if (dep_store_file)
            {
                cb_size anchor = cb_tmp_save();
                
                int buffer_size = 4096;
                char* buffer = cb_tmp_alloc(buffer_size);
                while (1)
                {
                    cb_file_info file_info = { 0 };
                    if (cbp_ib_dep_store_read_info(dep_store_file, buffer, buffer_size, &file_info))
                    {
                        /* Check size, modification time and hash.
                           Don't check volume id and file id because we don't retrieve them in
                           cbp_ib_dep_store_read_info (because we don't need them since we are using the full path)
                        */
                        file_info_flags = cb_file_info_SIZE | cb_file_info_MODIFICATION_TIME | cb_file_info_HASH;
                        if (!cb_file_info_matches(buffer, file_info_flags, &file_info))
                        {
                            file_need_to_be_compiled = cb_true;
                            break;
                        }
                    }
                    else
                    {
                        if (!feof(dep_store_file))
                        {
                            /*/ Something went wrong via the deserializing, assume the file to be changed. */
                            file_need_to_be_compiled = cb_true;
                            break;
                        }
                       
                        break;
                    }
                }
                
                cb_tmp_restore(anchor);
                
                /* Close dep_file */
                fclose(dep_store_file);
            }
        }
        
        cb_tmp_restore(handle.anchor);
    }
    
    if (!file_need_to_be_compiled)
    {    
        cb_log_debug("incremental build: skip: %s", file);
    }
    else
    {
        cb_log_debug("incremental build: not skip: %s", file);
    }
    
    if (file_need_to_be_compiled)
    {
        ib->stat_compilable += 1;
    }
    else
    {
        ib->stat_ignored += 1;
    }
    
    return file_need_to_be_compiled;
}


CB_INTERNAL cb_tmp_strv_handle cbp_ib_format_dep_folder(const cb_toolchain_t* toolchain, const cb_project_t* project)
{
    cb_tmp_strv_handle str_handle;
    
    str_handle.anchor = cb_tmp_save();
    str_handle.strv = cb_tmp_strv_printf("%s/cbp_ib_cache/" CB_STRV_FMT "/", toolchain->default_directory_base, CB_STRV_ARG(project->name));

    return str_handle;
}

CB_INTERNAL cb_tmp_strv_handle cbp_ib_format_dep_store_filepath(cbp_incremental_build* ib, const char* filepath)
{
    cb_tmp_strv_handle str_handle;

    cb_file_info file_info = {0};
    int flags = cb_file_info_FILE_ID;
    cb_bool ok = cb_file_info_query(filepath, flags, &file_info);
    
    CB_ASSERT(ok);
    
    str_handle.anchor = cb_tmp_save();
    
    str_handle.strv = cb_tmp_strv_printf("%s/cbp_ib_cache/" CB_STRV_FMT "/%zu-%zu-" CB_STRV_FMT ".cache", ib->toolchain.default_directory_base, CB_STRV_ARG(ib->project->name), file_info.volume_id, file_info.file_id, CB_STRV_ARG(cb_path_filename_str(filepath)));
   
    return str_handle;
}


CB_INTERNAL cb_tmp_strv_handle cbp_ib_format_file_in_dep_directory(cbp_incremental_build* ib, const char* filename)
{
    cb_tmp_strv_handle flag_dep_file_handle = {0};
    cb_tmp_strv_handle dir_handle = {0};
    
    flag_dep_file_handle.anchor = cb_tmp_save();
 
    dir_handle = cbp_ib_format_dep_folder(&ib->toolchain, ib->project);

    flag_dep_file_handle.strv = cb_tmp_strv_printf(CB_STRV_FMT  "%s", CB_STRV_ARG(dir_handle.strv), filename);
        
    return flag_dep_file_handle;
}


#ifdef _WIN32


CB_INTERNAL void cbp_ib_file_processed(cb_plugin* plugin, const char* file, const char* std_out, const char* std_err)
{
    cbp_incremental_build* ib = (cbp_incremental_build*)plugin;

    cb_size anchor = 0;
    cb_strv value = { 0 };
    cb_dep_parser parser = { 0 };
    const char* filepath_str = NULL;
  
    cb_tmp_strv_handle handle = cbp_ib_format_dep_store_filepath(ib, file);

    FILE* dep_store_to_write = cb_file_open_write(handle.strv.data);
      
    (void)std_err;
    
    if (dep_store_to_write)
    {
         /* Record current file, it is part of the dependency */
        cbp_ib_dep_store_write_info(dep_store_to_write, file);
        
        cb_msvc_dep_parser_init(&parser);
            
        cb_msvc_dep_parser_reset(&parser, std_out);
        
        while(cb_msvc_dep_parser_get_next(&parser, std_out, &value))
        {
            anchor = cb_tmp_save();
            
            filepath_str = cb_tmp_sprintf(CB_STRV_FMT, CB_STRV_ARG(value));
            cbp_ib_dep_store_write_info(dep_store_to_write, filepath_str);
            
            cb_tmp_restore(anchor);
        }
        
        fclose(dep_store_to_write);
    }
    
    cb_tmp_restore(handle.anchor);
}

#else

CB_INTERNAL void cbp_ib_file_processed(cb_plugin* plugin, const char* filepath, const char* gcc_dep_filepath, const char* unused)
{
    cbp_incremental_build* ib = (cbp_incremental_build*)plugin;
    cb_tmp_strv_handle handle = cbp_ib_format_dep_store_filepath(ib, filepath);
    
    cb_size anchor = 0;
    cb_strv value = { 0 };
    cb_dep_parser parser = { 0 };
    
    const char* filepath_str = NULL;
    int buffer_size = 4096;
    char* buffer_read = cb_tmp_calloc(buffer_size);
    char* dep_read = cb_tmp_calloc(buffer_size);
    FILE* gcc_dep_file_to_read = NULL;

    /* Create new file, overwrite if already exists. */
    FILE* dep_store_to_write = cb_file_open_write(handle.strv.data);

    (void)unused;

    if (dep_store_to_write)
    {
        gcc_dep_file_to_read = cb_file_open_readonly(gcc_dep_filepath);
       
        /* Read all dependencies from the dependency .d file */
        if (gcc_dep_file_to_read)
        {
            cb_gcc_dep_parser_init(&parser, buffer_read, buffer_size, dep_read, buffer_size);
            
            cb_gcc_dep_parser_reset(&parser, gcc_dep_file_to_read);
            
            while(cb_gcc_dep_parser_get_next(&parser, gcc_dep_file_to_read, &value))
            {
                anchor = cb_tmp_save();
                
                filepath_str = cb_tmp_sprintf(CB_STRV_FMT, CB_STRV_ARG(value));
                cbp_ib_dep_store_write_info(dep_store_to_write, filepath_str);
                
                cb_tmp_restore(anchor);
            }
            
            fclose(gcc_dep_file_to_read);
        }
        
        fclose(dep_store_to_write);
    }
    
    cb_tmp_restore(handle.anchor);
}

#endif

CB_INTERNAL void cbp_ib_dep_store_write_info(FILE* dep_store_file, const char* file_to_record)
{
    cb_size anchor = cb_tmp_save();

    cb_file_info file_info = {0};
    int flags = cb_file_info_ALL;
    if(!cb_file_info_query(file_to_record, flags, &file_info))
    {
        cb_log_error("could not get file info");
        return;
    }
    
    fprintf(dep_store_file, "%s" ";", file_to_record);
    fprintf(dep_store_file, CB_U64_FMT  ";", file_info.size);
    fprintf(dep_store_file, CB_U64_FMT  ";", file_info.last_modification);
    fprintf(dep_store_file, CB_U64_FMT  "\r\n", file_info.hash);

    cb_tmp_restore(anchor);
}

CB_INTERNAL cb_bool cbp_ib_dep_store_read_info(FILE* file, char* buffer, int buffer_size, cb_file_info* file_info)
{
    size_t scanned_count = 0;
    char* cursor = buffer;
    CB_ASSERT(buffer_size >= 4096);
    
    if (fgets(buffer, (size_t)buffer_size, file))
    {
        cursor = memchr(cursor, ';', (size_t)buffer_size);

        if (!cursor)
        {
            cb_log_info("could not deserialize file.");
            return cb_false;
        }

        /* First token was found, set null-terminating char */
        *cursor = '\0';

        /* Skip '\0'. */
        cursor += 1;

        scanned_count = CB_SSCANF(cursor,
                CB_U64_FMT ";"  CB_U64_FMT ";" CB_U64_FMT "\r\n",
                &file_info->size, &file_info->last_modification, &file_info->hash);

        if (scanned_count == 3)
        {
            return cb_true;
        }
    }

    return cb_false;
}

CB_INTERNAL cb_bool cbp_ib_check_full_rebuild_needed(cbp_incremental_build* ib)
{
    size_t i = 0;
    size_t prop_count = 0;
    cb_bool needs_full_rebuild = cb_true;
    
    /* If it exists to read file containing the flags. */
    cb_tmp_strv_handle handle = cbp_ib_format_file_in_dep_directory(ib, "flags.cache");

    FILE* flag_dep_file_read = cb_file_open_readonly(handle.strv.data);

    /* Character in flag strings. */
    cb_u64 flag_len = 0;
    /* hash sum of all the flag strings. */
    cb_u64 flag_hash = cb_hash_64_init();
        
    cb_bool cached_value_retrieved = cb_false;
    cb_u64 cached_flag_len = 0;
    cb_u64 cached_flag_hash = 0;

    static const char* properties[] = {
        cb_DEFINES,
        cb_CXFLAGS,
        "cflags", /* @TODO use cb_CFLAGS */
        "cxxflags", /* @TODO use cb_CXXLAGS? */
        cb_INCLUDE_DIRECTORIES
    };
    
    if (flag_dep_file_read)
    {
        /* Create tmp buffer to read the content of the file. 
           Must be a singel line with the length of the file and the hash of the file.
        */
        int buffer_size = 512;
        char* buffer = (char*)cb_tmp_calloc(buffer_size);

        if (fgets(buffer, buffer_size, flag_dep_file_read))
        {
            /* Read the two integers in the file. */
            size_t n = CB_SSCANF(buffer, CB_U64_FMT ";" CB_U64_FMT, &cached_flag_len, &cached_flag_hash);

            /* Ensure we scanned two values. */
            if (n == 2)
            {
                cached_value_retrieved = cb_true;
            }
        }
        
        /* Close the file */
        fclose(flag_dep_file_read);
    }

    flag_hash = cb_hash_64_init();

    prop_count = sizeof(properties) / sizeof(properties[0]);

    /* Calculate hash and size o the file. */
    for (i = 0; i < prop_count; i += 1)
    {
        const char* prop = properties[i];

        cb_kv_range range = cb_mmap_get_range_str(&ib->project->mmap, prop);
        cb_kv current = { 0 };
        while (cb_mmap_range_get_next(&range, &current))
        {
            cb_strv strv = current.u.strv;
            flag_hash = cb_hash_64_combine(flag_hash, strv.data, (int)strv.size);
            flag_len += (cb_u64)strv.size;
        }
    }

    if (cached_value_retrieved)
    {
        if (flag_hash == cached_flag_hash
           && flag_len == cached_flag_len)
        {
            needs_full_rebuild = cb_false;
        }
    }

    /* Write current file len and file hash.*/
    {
        FILE* flag_dep_file_write = cb_file_open_write(handle.strv.data);
        
        if (flag_dep_file_write)
        {
            fprintf(flag_dep_file_write, CB_U64_FMT ";" CB_U64_FMT, flag_len, flag_hash);
            fclose(flag_dep_file_write);
        }
        else
        {
            /* Something was wrong, try to delete the cache file anyway. */
            cb_delete_file(handle.strv.data);
        }
    }
    
    cb_tmp_restore(handle.anchor);
    
    return needs_full_rebuild;
}

#endif /* CB_PLUGIN_INCREMENTAL_BUILD_IMPL */

#endif /* CB_IMPLEMENTATION */