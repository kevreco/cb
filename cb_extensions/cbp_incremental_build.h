/*

This plugin depends on:
  cb_arena.h
  cb_hash.h
  cb_file_info.h
  cb_file_it.h

*/

#include <stdio.h>
#include <string.h>

#include "cb_arena.h"
#include "cb_hash.h"
#include "cb_file_info.h"
#include "cb_file_it.h"

#ifndef _WIN32
#include "cb_gcc_dep_parser.h"
#endif

#ifndef CB_SSCANF
#ifdef _WIN32
#define CB_SSCANF sscanf_s
#else
#define CB_SSCANF sscanf
#endif
#endif

/* Linked list node */
typedef struct cbp_ib_node cbp_ib_node;
struct cbp_ib_node
{
    cb_strv value;
    cbp_ib_node* next;
};

typedef struct cbp_incremental_build cbp_incremental_build;
struct cbp_incremental_build
{
    /* Plugin base */
    cb_plugin plugin;
    
    /* Current toolchain */
    cb_toolchain_t toolchain;
    /* Current project */
    cb_project_t* project;
    
    /* Arena to allocate all the cbp_ib_node and their string views. */
    cb_arena arena;

    cbp_ib_node* first; /* First node of the registered file list */
    cbp_ib_node* last;  /* Last node of the registered file list */
    
    /* When compiler flags or preprocessor defines are changed we need to do a full rebuild */
    cb_bool needs_full_rebuild;
    
    /* Some statistics. Reset each run. */
    int stat_ignored;
    int stat_compilable;
};

typedef struct cb_tmp_strv_handle cb_tmp_strv_handle;
struct cb_tmp_strv_handle {
    cb_strv strv;
    cb_size anchor;
};

CB_API void cbp_incremental_build_init(cbp_incremental_build* plugin);
CB_API void cbp_incremental_build_delete_cache(cbp_incremental_build* plugin);

CB_INTERNAL void cbp_ib_bake_starting(cb_plugin* plugin);
CB_INTERNAL void cbp_ib_register_file(cb_plugin* plugin, cb_strv absolute_filepath);
CB_INTERNAL const char* cbp_ib_extra_argument(cb_plugin* plugin);
CB_INTERNAL cb_bool cbp_ib_can_process_file(cb_plugin* plugin, const char* file);
CB_INTERNAL void cbp_ib_file_processed(cb_plugin* plugin, const char* file, const char* std_out, const char* std_err);
CB_INTERNAL void cbp_ib_destroy(cb_plugin* plugin);

/* Write a formated file info line into the dep file */
CB_INTERNAL void cbp_ib_record_dep_info(FILE* dep_file, const char* file_to_record);
/* Read a formated file info line from the dep file */
/* @TODO remove out_size if not used */
CB_INTERNAL cb_bool cbp_ib_read_dep_info(FILE* file, char* buffer, int buffer_size, size_t* out_size, cb_file_info* file_info);

CB_INTERNAL cb_bool cbp_ib_check_full_rebuild_needed(cbp_incremental_build* ipb);

CB_INTERNAL cb_tmp_strv_handle cbp_ib_format_dep_folder(const cb_toolchain_t* toolchain, const cb_project_t* project);
CB_INTERNAL cb_tmp_strv_handle cbp_ib_format_dep_store_filepath(cbp_incremental_build* ipb, const char* filepath);

/*-----------------------------------------------------------------------*/
/* API implementation */
/*-----------------------------------------------------------------------*/

CB_API void cbp_incremental_build_init(cbp_incremental_build* plugin)
{
    memset(plugin, 0, sizeof(cbp_incremental_build));
    plugin->plugin.name = "cbp_incremental_build";

    plugin->plugin.bake_starting = cbp_ib_bake_starting;
    plugin->plugin.extra_argument = cbp_ib_extra_argument;
    plugin->plugin.register_file = cbp_ib_register_file;
    plugin->plugin.can_process_file = cbp_ib_can_process_file;
    plugin->plugin.file_processed = cbp_ib_file_processed;
    plugin->plugin.destroy = cbp_ib_destroy;

    cb_arena_init(&plugin->arena);
}

CB_API void cbp_incremental_build_delete_cache(cbp_incremental_build* plugin)
{
    cb_toolchain_t toolchain = {0};
    cb_project_t* project = NULL;
    cb_tmp_strv_handle handle = {0};
    cb_file_it it = {0};
    CB_UNUSED(plugin);
     
    toolchain = cb_toolchain_get();
    project = cb_current_project();
    
    handle = cbp_ib_format_dep_folder(&toolchain, project);
     cb_log_important("DELETE CACHE 2");
    /* Remove all files from the dep directory */
    cb_file_it_init_recursive(&it, handle.strv.data);

    while(cb_file_it_get_next(&it))
    {
        const char* file = cb_file_it_current_file(&it);
        cb_bool deleted = cb_delete_file(file);
        CB_ASSERT(deleted);
    }

    cb_tmp_restore(handle.anchor);
       cb_log_important("DELETE CACHE 3");
}

CB_INTERNAL void cbp_ib_bake_starting(cb_plugin* plugin)
{
    cbp_incremental_build* ipb = (cbp_incremental_build*)plugin;
    ipb->stat_ignored = 0;
    ipb->stat_compilable = 0;
    
    ipb->first = 0;
    ipb->last  = 0;
    
    /* Reference current toolchain and project. */
    ipb->toolchain = cb_toolchain_get();
    ipb->project = cb_current_project();
   
   cb_log_important("cbp_ib_bake_starting 2");
    /* Ensure that the cache folder exists. */
    {
        /* Format directory in tmp allocator. */
        cb_tmp_strv_handle dir_handle = cbp_ib_format_dep_folder(&ipb->toolchain, ipb->project);

        printf("CREATED DIRECTORY %s", dir_handle.strv.data);
        /* Created directories */
        cb_create_directories(dir_handle.strv.data, dir_handle.strv.size);

        /* Release tmp memory */
        cb_tmp_restore(dir_handle.anchor);
    } 
    
    cb_log_important("cbp_ib_bake_starting 3");
    ipb->needs_full_rebuild = cbp_ib_check_full_rebuild_needed(ipb);
    cb_log_important("cbp_ib_bake_starting 4");
}

CB_INTERNAL void cbp_ib_register_file(cb_plugin* plugin, cb_strv absolute_filepath)
{
    cbp_incremental_build* ipb = (cbp_incremental_build*)plugin;

    char* ptr = (char*)cb_arena_alloc(&ipb->arena, sizeof(cbp_ib_node) + absolute_filepath.size + 1); /* +1 for null-terminating char */
    
    cbp_ib_node* new_node = (cbp_ib_node*)ptr;
    memset(new_node, 0, sizeof(cbp_ib_node));
    new_node->value.data = ptr + sizeof(cbp_ib_node);
    new_node->value.size = absolute_filepath.size;
    memcpy((char*)new_node->value.data, (char*)absolute_filepath.data, absolute_filepath.size + 1);  /* +1 for null-terminating char */
    
    new_node->value.data[new_node->value.size] = '\0';
    
    if(!ipb->first)
    {
        ipb->first = new_node;
        ipb->last = new_node;
    }
    else
    {
        cbp_ib_node* n = ipb->last;
        n->next = new_node;
        ipb->last = new_node;
    }
}

CB_INTERNAL const char* cbp_ib_extra_argument(cb_plugin* plugin)
{
    cbp_incremental_build* ipb = (cbp_incremental_build*)plugin;
    /*const char* dep_file_path = NULL;
    const char* dep_file_abs_path = NULL;*/

    if (cb_str_equals(ipb->toolchain.family, "msvc"))
    {
        return "/showIncludes /FC";
    }
    else if (cb_str_equals(ipb->toolchain.family, "gcc"))
    {
        /* NOTE: If necessary "-fdep-paths=full" can be used to display the absolute paths. */
        /* 
        dep_file_path = cb_tmp_sprintf("%s/cbp_ib_cache/" CB_STRV_FMT "/tmp.dependencies", ipb->toolchain.default_directory_base, CB_STRV_ARG(ipb->project->name));
        dep_file_abs_path = cb_path_get_absolute_file(dep_file_path);
        
        */
        
        /*return cb_tmp_sprintf("-MMD -MF \"%s\"", dep_file_abs_path);*/
         /*return cb_tmp_sprintf("-H \"%s\"", dep_file_abs_path);*/
        /*return cb_tmp_sprintf("-MMD \"%s\"", dep_file_abs_path);*/
        
        
        
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
    cbp_incremental_build* ipb = (cbp_incremental_build*)plugin;

    cb_bool file_need_to_be_compiled = cb_true;
 
    cb_tmp_strv_handle handle = {0};
    
    int file_info_flags = 0;
 
    const char* dep_file_path = NULL;
    cb_log_important("cbp_ib_can_process_file: %s", file);
    fprintf(stderr, "FILE: %s\n", file);
    
    if (!ipb->needs_full_rebuild)
    {
        file_need_to_be_compiled = cb_false;
        
        handle = cbp_ib_format_dep_store_filepath(ipb, file);
        dep_file_path = handle.strv.data;
        
        /* If the dep_file does not exists, the original file needs to be processed. */
        if (!cb_path_exists(dep_file_path))
        {
            file_need_to_be_compiled = cb_true;
        }
        else
        {
            /* Open dep_file */
            FILE* dep_file = cb_file_open_readonly(dep_file_path);
            if (dep_file)
            {
                cb_size anchor = cb_tmp_save();
                
                int buffer_size = 4096;
                char* buffer = cb_tmp_alloc(buffer_size);
                while (1)
                {
                    /* @TODO remove this argument if not used. */
                    size_t out_size;
                    cb_file_info file_info = {0};
                    if (cbp_ib_read_dep_info(dep_file, buffer, buffer_size, &out_size, &file_info))
                    {
                        cb_log_important("TRY MATCH INPUT: %s", file);
                        cb_log_important("WITH dep line: %s", buffer);
                        /* Check size, modification time and hash.
                           Don't check volume id and file id because we don't retrieve them in
                           cbp_ib_read_dep_info (because we don't need them since we are using the full path)
                        */
                        file_info_flags = cb_file_info_SIZE | cb_file_info_MODIFICATION_TIME | cb_file_info_HASH;
                        if (!cb_file_info_matches(buffer, file_info_flags, &file_info))
                        {
                            cb_log_important("[[NOT SKIP]]: %s", file);
                            cb_log_important("[[DUE TO]]: %s", buffer);

                            file_need_to_be_compiled = cb_true;
                            break;
                        }
                    }
                    else
                    {
                        if (!feof(dep_file))
                        {
                            /*/ Something went wrong via the deserializing, assume the file to be changed. */
                            cb_log_important("[[NOT SKIP]]: %s", file);
                            cb_log_important("[[BAD SERIALIZING]]: %s", buffer);
                            file_need_to_be_compiled = cb_true;
                            break;
                        }
                       
                        break;
                    }
                }
                
                cb_tmp_restore(anchor);
                
                /* Close dep_file */
                fclose(dep_file);
            }
        }
        
        cb_tmp_restore(handle.anchor);
    }
    
    if (!file_need_to_be_compiled)
    {    
        cb_log_important("[[SKIP]]: %s", file);
    }
    else
    {
        cb_log_important("[[NOT SKIP ???]]: %s", file);
    }
    
    if (file_need_to_be_compiled)
    {
        ipb->stat_compilable += 1;
    }
    else
    {
        ipb->stat_ignored += 1;
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

/* @TODO write prototype in declaration */
CB_INTERNAL cb_tmp_strv_handle cbp_ib_format_dep_store_filepath(cbp_incremental_build* ipb, const char* filepath)
{
    cb_tmp_strv_handle str_handle;

    cb_file_info file_info = {0};
    int flags = cb_file_info_FILE_ID;
    cb_bool ok = cb_file_info_query(filepath, flags, &file_info);
    
    CB_ASSERT(ok);
    
    str_handle.anchor = cb_tmp_save();
    
    str_handle.strv = cb_tmp_strv_printf("%s/cbp_ib_cache/" CB_STRV_FMT "/%zu-%zu-" CB_STRV_FMT ".cache", ipb->toolchain.default_directory_base, CB_STRV_ARG(ipb->project->name), file_info.volume_id, file_info.file_id, CB_STRV_ARG(cb_path_filename_str(filepath)));
   
    return str_handle;
}


CB_INTERNAL cb_tmp_strv_handle cbp_ib_format_file_in_dep_directory(cbp_incremental_build* ipb, const char* filename)
{
    cb_tmp_strv_handle flag_dep_file_handle = {0};
    cb_tmp_strv_handle dir_handle = {0};
    
    flag_dep_file_handle.anchor = cb_tmp_save();
 
    dir_handle = cbp_ib_format_dep_folder(&ipb->toolchain, ipb->project);

    flag_dep_file_handle.strv = cb_tmp_strv_printf(CB_STRV_FMT  "%s", CB_STRV_ARG(dir_handle.strv), filename);
        
    return flag_dep_file_handle;
}


#ifdef _WIN32

/* WIN32-only
   If it's an 'include line entry' displayed by \showIncludes,
   returns the length of the line, otherwise returns 0 */
CB_INTERNAL int cbp_ib_is_show_include_line(const char* line)
{
    static cb_strv prefixes[] = {
        
        CB_STRV("Note: including file: "),              /* English */
        CB_STRV("Remarque : inclusion du fichier :  "), /* French */
        CB_STRV("Hinweis: Einlesen der Datei: "),       /* German */
        CB_STRV("Nota: file incluso  "),                /* Italian */
        CB_STRV("注意: 包含文件:  "),                    /* Chinese */
        CB_STRV("メモ: インクルード ファイル:  ")               /* Japanese */
    };
    
    int sizeof_array = sizeof(prefixes) /  sizeof(prefixes[0]);
    
    for(int i = 0; i < sizeof_array; i += 1)
    {
        if (strncmp(line, prefixes[i].data, prefixes[i].size) == 0)
        {
            // @TODO fix this, either use size_t only for std lib and int everywhere else, or use size_t everywhere
            return (int)prefixes[i].size;
        }
    }
    
    return 0;
}

#define CB_SKIP_WHITESPACES(src) do { } while(0)

CB_INTERNAL void cbp_ib_file_processed(cb_plugin* plugin, const char* file, const char* std_out, const char* std_err)
{
    (void)std_err;
    
    cbp_incremental_build* ipb = (cbp_incremental_build*)plugin;

    const char* dep_path_start = NULL;
    const char* dep_path_end = NULL;
    
    const char* c = std_out;

    cb_tmp_strv_handle handle = cbp_ib_format_dep_store_filepath(ipb, file);
    FILE* dep_file = cb_file_open_readwrite(handle.strv.data);
      
    if (dep_file)
    {
        /* Record current file, it is part of the dependency */
        cbp_ib_record_dep_info(dep_file, file);

        while(*c != '\0')
        {
            /* Check if line starts with the marker of the /showIncludes */
            int prefix_len = cbp_ib_is_show_include_line(c);
            
            if(prefix_len > 0)
            {
                c += prefix_len;
                
                /* Skip whitespaces */
                while(*(c) == ' ') c += 1;
                
                dep_path_start = c;
                c = strchr(c, '\n');
                
                /* End of string reached */
                if (c == NULL)
                {
                    break;
                }
                
                /* Get end path of the current include, removing \n or \r\n */
                dep_path_end = (c > dep_path_start  && c[-1] == '\r' ) ? c - 1 : c;

                cb_strv dep_path = cb_strv_make(dep_path_start, dep_path_end - dep_path_start);
                
                const char* dep_to_record = cb_tmp_sprintf(CB_STRV_FMT, CB_STRV_ARG(dep_path));
                
                /* Write dep to dep_file if it's not a system include */
                cb_bool is_system_file = cb_strv_contains_str(dep_path, "Microsoft Visual Studio");
                
                if (!is_system_file)
                {
                    cbp_ib_record_dep_info(dep_file, dep_to_record);
                }
            }
            
            c += 1;
        }
        
        cb_file_close(dep_file);
    }
    
    cb_tmp_restore(handle.anchor);
}
#else

CB_INTERNAL void cbp_ib_file_processed(cb_plugin* plugin, const char* filepath, const char* gcc_dep_filepath, const char* unused)
{
    cbp_incremental_build* ipb = (cbp_incremental_build*)plugin;
    cb_tmp_strv_handle handle = cbp_ib_format_dep_store_filepath(ipb, filepath);
    
    cb_size anchor = 0;
    cb_strv value = { 0 };
    cb_gcc_dep_parser parser = { 0 };
    
    const char* filepath_str = NULL;
    int buffer_size = 4096;
    /*char* buffer_read = cb_tmp_calloc(buffer_size);*/
    char buffer_read[4096];
    /*char* dep_read = cb_tmp_calloc(buffer_size);*/
     char dep_read[4096];
   FILE* gcc_dep_file_to_read = NULL;

    
    // Create new file, overwrite if already exists
    FILE* dep_store_to_write = cb_file_open_readwrite(handle.strv.data);
   
    
    (void)unused;
     /*
    (void)plugin;
    (void)filepath;
    (void)gcc_dep_filepath;
    
    (void)ipb;
    (void)handle;
    */
    if (dep_store_to_write)
    {
        gcc_dep_file_to_read = cb_file_open_readonly(gcc_dep_filepath);
       
        if (gcc_dep_file_to_read)
        {
            cb_gcc_dep_parser_init(&parser, buffer_read, buffer_size, dep_read, buffer_size);
            cb_gcc_dep_parser_reset(&parser, gcc_dep_file_to_read);
            
            while(cb_gcc_dep_parser_get_next(&parser, gcc_dep_file_to_read, &value))
            {
                anchor = cb_tmp_save();
                
                filepath_str = cb_tmp_sprintf(CB_STRV_FMT, CB_STRV_ARG(value));
                cbp_ib_record_dep_info(dep_store_to_write, filepath_str);
                
                cb_tmp_restore(anchor);
            }
            
            fclose(gcc_dep_file_to_read);
        }
        
        fclose(dep_store_to_write);
    }
    
    cb_tmp_restore(handle.anchor);
}

#endif

CB_INTERNAL void cbp_ib_destroy(cb_plugin* plugin)
{
    cbp_incremental_build* ipb = (cbp_incremental_build*)plugin;
     
    cb_arena_destroy(&ipb->arena);
     
    cb_log_important("cbp_ib_destroy\n");
}

CB_INTERNAL void cbp_ib_record_dep_info(FILE* dep_file, const char* file_to_record)
{
    cb_size anchor = cb_tmp_save();
                   
    cb_file_info file_info = {0};
    int flags = cb_file_info_ALL;
    if(!cb_file_info_query(file_to_record, flags, &file_info))
    {
        cb_log_error("could not get file info");
    }
    else
    {
        fprintf(dep_file, "%s" ";", file_to_record);
        fprintf(dep_file, CB_U64_FMT  ";", file_info.size);
        fprintf(dep_file, CB_U64_FMT  ";", file_info.last_modification);
        fprintf(dep_file, CB_U64_FMT  "\r\n", file_info.hash);
    }
    
    cb_tmp_restore(anchor);
}

CB_INTERNAL cb_bool cbp_ib_read_dep_info(FILE* file, char* buffer, int buffer_size, size_t* out_size, cb_file_info* file_info)
{
    size_t scanned_count = 0;
    CB_ASSERT(buffer_size >= 4096);
    
    if (fgets(buffer, (size_t)buffer_size, file))
    {
        /* @TODO try to use memchr instead of strcspn? */
        /* Get span until next separator */
        const char* chars = ";";
        size_t len = strcspn(buffer, chars);

        if (len == (size_t)buffer_size)
        {
            cb_log_important("could not deserialize file.");
            return cb_false;
        }
        
        /* First string was found, set null-terminating char, and save the length of the string. */
        buffer[len] = '\0';
        *out_size = len;
        
        /* Skip separator. */
        buffer += len + 1;
        
        /* Read .. @TODO */
        scanned_count = CB_SSCANF(buffer,
                CB_U64_FMT ";"  CB_U64_FMT ";" CB_U64_FMT "\r\n",
                &file_info->size, &file_info->last_modification, &file_info->hash);

        if (scanned_count == 3)
        {
            return cb_true;
        }
    }

    return cb_false;
}

CB_INTERNAL cb_bool cbp_ib_check_full_rebuild_needed(cbp_incremental_build* ipb)
{
    size_t i = 0;
    size_t prop_count = 0;
    cb_bool needs_full_rebuild = cb_true;
    
    /* Read file containing the flags. */
    cb_tmp_strv_handle handle = cbp_ib_format_file_in_dep_directory(ipb, "flags.cache");

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
        "cflags", // @TODO use cb_CFLAGS
        "cxxflags", // @TODO use cb_CXXLAGS
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

cb_log_important("cbp_ib_check_full_rebuild_needed 5");
 
    flag_hash = cb_hash_64_init();

    prop_count = sizeof(properties) / sizeof(properties[0]);

    for (i = 0; i < prop_count; i += 1)
    {
        const char* prop = properties[i];

        cb_kv_range range = cb_mmap_get_range_str(&ipb->project->mmap, prop);
        cb_kv current = { 0 };
        while (cb_mmap_range_get_next(&range, &current))
        {
            cb_strv strv = current.u.strv;
            flag_hash = cb_hash_64_combine(flag_hash, strv.data, (int)strv.size);
            flag_len += (cb_u64)strv.size;
        }
    }

    cb_log_important("flag_hash:" CB_U64_FMT, flag_hash);
    cb_log_important("flag_len:" CB_U64_FMT, flag_len);
    cb_log_important("cached_flag_hash:" CB_U64_FMT, cached_flag_hash);
    cb_log_important("cached_flag_len:" CB_U64_FMT, cached_flag_len);

    if (cached_value_retrieved)
    {
        if (flag_hash == cached_flag_hash
           && flag_len == cached_flag_len)
        {
            // TODO move this where appropriate.
            needs_full_rebuild = cb_false;
        }
    }

    /* Write current file len and file hash.*/
    {
        /* @TODO this fails supposedly because the folder does not exists yet.
         @TODO ensure that the cache directory exists as soon as possible */
        FILE* flag_dep_file_write = cb_file_open_readwrite(handle.strv.data);
        
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