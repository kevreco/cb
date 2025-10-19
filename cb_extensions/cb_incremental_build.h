/*

    This 
Depends on:
  cb_arena
  cb_hash
  cb_file_info
  cb_file_it

*/

#include <stdio.h>
#include <string.h>

#include "cb_arena.h"
#include "cb_hash.h"
/* @TODO rename to cb_file_info */
#include "cb_fileid.h"
#include "cb_file_it.h"

typedef struct cb_ibp_node cb_ibp_node;
struct cb_ibp_node
{
    cb_strv value;
    cb_ibp_node* next;
};

typedef struct cb_incremental_build_plugin cb_incremental_build_plugin;
struct cb_incremental_build_plugin
{
    cb_plugin plugin;
    int previous_line_start; /* @TODO if not used */
    int previous_line_end;   /* @TODO if not used */
    cb_dstr referenced_paths;
    cb_darrT(const char*) registered_paths; // Array of byte
    cb_arena arena;
    cb_ibp_node* first;
    cb_ibp_node* last;
    
    /* statistics reset each run. */
    int stat_ignored;
    int stat_considered;
};

/* @TODO use tmp_alloc instead */
char hash_buffer[16 * 1024];

typedef struct cb_tmp_str_handle cb_tmp_str_handle;
struct cb_tmp_str_handle {
    cb_strv strv;
    cb_size anchor;
};


CB_API void cb_incremental_build_plugin_init(cb_incremental_build_plugin* plugin);
CB_API void cb_incremental_build_plugin_delete_cache(cb_incremental_build_plugin* plugin);

CB_INTERNAL void cb_ibp_bake_starting(cb_plugin* plugin);
CB_INTERNAL void cb_ibp_register_file(cb_plugin* plugin, cb_strv absolute_filepath);
CB_INTERNAL const char* cb_ibp_extra_argument(cb_plugin* plugin);
CB_INTERNAL cb_bool cb_ibp_can_process_file(cb_plugin* plugin, const char* file);
CB_INTERNAL void cb_ibp_project_processed(cb_plugin* plugin, const char* std_out, const char* std_err);
//static void cb_incremental_build_plugin_clear(cb_plugin* plugin);
CB_INTERNAL void cb_ibp_destroy(cb_plugin* plugin);

// Return positive integer if it's a include line displayed for \showIncludes
CB_INTERNAL int cb_ibp_is_show_include_line(const char* line);

CB_INTERNAL void cb_ibp_write_file_info_to_dep_file(FILE* dep_file, cb_strv dep_path);

CB_INTERNAL cb_bool cb_ibp_deserialize_file_info(FILE* file, char* buffer, int buffer_size, size_t* out_size, cb_file_info* file_info);

CB_INTERNAL cb_tmp_str_handle cb_ibp_format_dep_folder(cb_toolchain_t* toolchain, cb_project_t* project);
CB_INTERNAL cb_tmp_str_handle cb_ibp_format_dep_file_name(const char* filepath, cb_toolchain_t* toolchain, cb_project_t* project, cb_bool create_directory);


CB_API void cb_incremental_build_plugin_init(cb_incremental_build_plugin* plugin)
{
    memset(plugin, 0, sizeof(cb_incremental_build_plugin));
    plugin->plugin.name = "cb_incremental_build_plugin";

    plugin->plugin.bake_starting = cb_ibp_bake_starting;
    plugin->plugin.extra_argument = cb_ibp_extra_argument;
    plugin->plugin.register_file = cb_ibp_register_file;
    plugin->plugin.can_process_file = cb_ibp_can_process_file;
    plugin->plugin.project_processed = cb_ibp_project_processed;
    //plugin->plugin.clear = cb_incremental_build_plugin_clear;
    plugin->plugin.destroy = cb_ibp_destroy;
  
    cb_arena_init(&plugin->arena);
}

CB_API void cb_incremental_build_plugin_delete_cache(cb_incremental_build_plugin* plugin)
{
    (void)plugin;
    
    cb_toolchain_t toolchain = cb_toolchain_get();
    cb_project_t* project = cb_current_project();
    
    cb_tmp_str_handle handle = cb_ibp_format_dep_folder(&toolchain, project);
    
    
    /* */
    cb_file_it it;
    cb_file_it_init_recursive(&it, handle.strv.data);

    while(cb_file_it_get_next(&it))
    {
        const char* file = cb_file_it_current_file(&it);
        cb_bool deleted = cb_delete_file(file);
        CB_ASSERT(deleted);
    }
//CB_API void cb_file_it_destroy(cb_file_it* it);
//
//CB_API const char* cb_file_it_current_file(cb_file_it* it);
//
//CB_API cb_bool cb_file_it_get_next(cb_file_it* it);
//    cb_bool deleted = cb_delete_folder(handle.strv.data);

    
    cb_tmp_restore(handle.anchor);
}

CB_INTERNAL void cb_ibp_bake_starting(cb_plugin* plugin)
{
    cb_incremental_build_plugin* ipb = (cb_incremental_build_plugin*)plugin;
    ipb->stat_ignored = 0;
    ipb->stat_considered = 0;
    
    ipb->first = 0;
    ipb->last  = 0;
}

CB_INTERNAL void cb_ibp_register_file(cb_plugin* plugin, cb_strv absolute_filepath)
{
    cb_incremental_build_plugin* ipb = (cb_incremental_build_plugin*)plugin;

    char* ptr = (char*)cb_arena_alloc(&ipb->arena, sizeof(cb_ibp_node) + absolute_filepath.size + 1); /* +1 for null-terminating char */
    
    cb_ibp_node* new_node = (cb_ibp_node*)ptr;
    memset(new_node, 0, sizeof(cb_ibp_node));
    new_node->value.data = ptr + sizeof(cb_ibp_node);
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
        cb_ibp_node* n = ipb->last;
        n->next = new_node;
        ipb->last = new_node;
    }
}

CB_INTERNAL const char* cb_ibp_extra_argument(cb_plugin* plugin)
{
    (void)plugin;
    
    cb_toolchain_t toolchain = cb_toolchain_get();

    if (cb_str_equals(toolchain.family, "msvc"))
    {
        return 
            "/showIncludes /FC";
    }
    else if (cb_str_equals(toolchain.family, "gcc"))
    {
        CB_ASSERT(0 && "Unhandled toolchain for cb_incremental_build_plugin");
        return "@TODO";
    }
    else
    {
        CB_ASSERT(0 && "Unhandled toolchain for cb_incremental_build_plugin");
    }
    return "";
}

CB_INTERNAL cb_bool cb_ibp_can_process_file(cb_plugin* plugin, const char* file)
{
    (void)file;
    
    cb_incremental_build_plugin* ipb = (cb_incremental_build_plugin*)plugin;

    cb_toolchain_t toolchain = cb_toolchain_get();
    cb_project_t* project = cb_current_project();

    // @TODO use cb_tmp for this.
    char buffer[2048] = {0};
    size_t out_size;
    cb_file_info file_info;
    
    cb_bool file_need_to_be_processed = cb_false;
    //cb_size anchor = cb_tmp_save();
    cb_file_id file_id = {0};
    cb_bool ok = cb_get_file_id(file, &file_id);
    CB_ASSERT(ok);

    cb_strv basename = cb_path_filename_str(file);
    //const char* out_dir = cb_get_output_directory(project, &toolchain);
    char buf[1024];
    // @TODO create function like cb_ibp_format_dep_filepath()
    snprintf(buf, sizeof(buf), "%s/ic/" CB_STRV_FMT "/%zu-%zu-" CB_STRV_FMT, toolchain.default_directory_base, CB_STRV_ARG(project->name), file_id.volume_id, file_id.file_id, CB_STRV_ARG(basename));
    
    /* If the dep_file does not exists, the original file needs to be processed. */
    if (!cb_path_exists(buf))
    {
        file_need_to_be_processed = cb_true;
    }
    else
    {

        /* Open dep_file */
        FILE* dep_file = cb_file_open_readonly(buf);
       
        /* @TODO handle multiple files */
        while (1)
        {
            if (cb_ibp_deserialize_file_info(dep_file, buffer, (int)sizeof(buffer), &out_size, &file_info))
            {
                 char buffer2[2048] = {0};
                   cb_log_important("TRY MATCH: %s", file);
                if (!cb_file_info_matches(buffer, buffer2, (int)sizeof(buffer2), &file_info))
                {
                    cb_log_important("[[NOT SKIP]]: %s", file);
                    cb_log_important("[[DUE TO]]: %s", buffer);

                    fclose(dep_file);
                    file_need_to_be_processed = cb_true;
                    break;
                }
                //cb_log_important("A:" CB_U64_FMT, out_size);
                //
                //cb_log_important("B:" CB_STRV_FMT "\t", out_size, buffer);
                //cb_log_important("C:" CB_U64_FMT  "\t", file_info.size);
                //cb_log_important("D:" CB_U64_FMT  "\t", file_info.last_modification);
                //cb_log_important("E:" CB_U64_FMT  "\t>>>>>\n", file_info.hash);
            }
            else
            {
                if (!feof(dep_file))
                {
                     // Something went wrong via the deserializing, assume the file to be changed.
                    cb_log_important("[[NOT SKIP]]: %s", file);
                    cb_log_important("[[BAD SERIALIZING]]: %s", buffer);
                    file_need_to_be_processed = cb_true;
                    break;
                }
                   
                break;
            }
            /* @TODO do approriate loop */
            
            
       
        }

        /* Close dep_file */
        fclose(dep_file);
    }
    
    //cb_tmp_restore(anchor);
    if (!file_need_to_be_processed)
    {    
        cb_log_important("[[SKIP]]: %s", file);
    }
    
    
    if (file_need_to_be_processed)
    {
        ipb->stat_considered += 1;
    }
    else
    {
        ipb->stat_ignored += 1;
    }
    
    return file_need_to_be_processed;
}

#define CB_SKIP_WHITESPACES(src) do { while(*src == ' ') src += 1; } while(0)

CB_INTERNAL cb_tmp_str_handle cb_ibp_format_dep_folder(cb_toolchain_t* toolchain, cb_project_t* project)
{
    cb_tmp_str_handle str_handle;
    
    str_handle.anchor = cb_tmp_save();   
    str_handle.strv = cb_tmp_strv_printf("%s/ic/" CB_STRV_FMT "/", toolchain->default_directory_base, CB_STRV_ARG(project->name));

    return str_handle;
}

/* @TODO write prototype in declaration */
CB_INTERNAL cb_tmp_str_handle cb_ibp_format_dep_file_name(const char* filepath, cb_toolchain_t* toolchain, cb_project_t* project, cb_bool create_directory)
{
    cb_tmp_str_handle str_handle;
    cb_file_id file_id = {0};
    
    cb_bool ok = cb_get_file_id(filepath, &file_id);
    
    CB_ASSERT(ok);
    
    str_handle.anchor = cb_tmp_save();
    
    /* Create directory if requested */
    if (create_directory)
    {
        cb_tmp_str_handle dir_handle = cb_ibp_format_dep_folder(toolchain, project);
        
        cb_log_debug("try to create dir: %s\n", dir_handle.strv.data);
        cb_create_directories(dir_handle.strv.data, dir_handle.strv.size);
        
        cb_tmp_restore(dir_handle.anchor);
    }
    
    str_handle.strv = cb_tmp_strv_printf("%s/ic/" CB_STRV_FMT "/%zu-%zu-" CB_STRV_FMT, toolchain->default_directory_base, CB_STRV_ARG(project->name), file_id.volume_id, file_id.file_id, CB_STRV_ARG(cb_path_filename_str(filepath)));
   
    return str_handle;
}

CB_INTERNAL void cb_ibp_project_processed(cb_plugin* plugin, const char* std_out, const char* std_err)
{

    cb_incremental_build_plugin* ipb = (cb_incremental_build_plugin*)plugin;
    //const char* registered_file_buffer = ipb->referenced_paths.data;
    cb_toolchain_t toolchain = cb_toolchain_get();
    cb_project_t* project = cb_current_project();
     
    const char* c = std_out;
    
    const char* src_path_start;
    const char* src_path_end;
    const char* dep_path_start;
    const char* dep_path_end;
    //cb_strv next_file = {0};
    cb_strv strv_empty = {0};
    cb_strv src = strv_empty;
    cb_ibp_node* reg_path_node = ipb->first;
    
    FILE* dep_file = 0;;
    while(*c != '\0')
    {
        /* Check if line starts with the marker of the /showIncludes */
        int prefix_len = cb_ibp_is_show_include_line(c);
        
        if(prefix_len > 0)
        {
            c += prefix_len;
            
            CB_SKIP_WHITESPACES(c);
            
            dep_path_start = c;
            c = strchr(c, '\n');
            
            // End of string reached
            if (c == NULL)
            {
                break;
            }
            
            dep_path_end = (c > dep_path_start  && c[-1] == '\r' ) ? c - 1 : c;
           
            cb_bool source_is_valid =
                src.size > 0 /* source file was found. */
                && dep_file != NULL /* dep file was created. */
                ;
                
            if (source_is_valid)
            {
                cb_strv dep_path = cb_strv_make(dep_path_start, dep_path_end - dep_path_start);
                
               
                 /* @TODO: Handle include dep here */
                 /* DEBUYG
                fprintf(stdout, "<DD>INCLUDE FOUND:%.*s</DD>\n", (int)(dep_path_end - dep_path_start), dep_path_start);
                
                /* Write dep to dep_file if it's not a system include */
                
                if (!cb_strv_contains_str(dep_path, "Microsoft Visual Studio"))
                {
                    cb_ibp_write_file_info_to_dep_file(dep_file, dep_path);
                    
                    //cb_size anchor = cb_tmp_save();
                    //
                    //const char* dep_path_str = cb_tmp_strv_to_str(dep_path);
                    //
                    ///* @TODO: rename cb_fileid.h to cb_file_info.h */
                    ///* @TODO still write info if it fails with all zero ? */
                    //
                    //cb_file_info info = {0};
                    //if(!cb_get_file_info(dep_path_str, hash_buffer, (int)sizeof(hash_buffer), &info))
                    //{
                    //    cb_log_error("could not get file info");
                    //}
                    //else
                    //{
                    //    cb_ibp_write_file_info_dep_file(dep_file, dep_path, &info);
                    //}
                    //
                    //cb_tmp_restore(anchor);
                }

            }
        }
        else
        {
            src_path_start = c;
            c = strchr(c, '\n');
            src_path_end = (c > src_path_start  && c[-1] == '\r' ) ? c - 1 : c;
           
            cb_strv maybe_src = cb_strv_make(src_path_start, src_path_end - src_path_start);
            
            //fprintf(stdout, "<SRC?:>" CB_STRV_FMT "<:SRC?>\n", CB_STRV_ARG(maybe_src));
          
            if (reg_path_node)
            {
                cb_strv reg = reg_path_node->value;
                
                //fprintf(stdout, "<REG:>" CB_STRV_FMT "<:REG>\n", CB_STRV_ARG(reg));

                if (cb_strv_ends_with(reg, maybe_src))
                {
                    src = reg;
                    reg_path_node = reg_path_node->next;
                    
                    /* Open dep file */
                    if (dep_file)
                    {
                        cb_file_close(dep_file);
                    }
                    
                    /* Ensure src was created as a null-terminated string */
                    
                    CB_ASSERT(src.data[src.size] == '\0');
                    
                    cb_bool create_directory = cb_true;
                    cb_tmp_str_handle handle = cb_ibp_format_dep_file_name(src.data, &toolchain, project, create_directory);
                    
                    //cb_file_id file_id;
                    //cb_bool ok = cb_get_file_id(src.data, &file_id);
                    //CB_ASSERT(ok);
                    //
                    //
                    //
                    ////const char* out_dir = cb_get_output_directory(project, &toolchain);
                    //char buf[1024];
                    //
                    //
                    //int dir_n = snprintf(buf, sizeof(buf), "%s/ic/" CB_STRV_FMT "/", toolchain.default_directory_base, CB_STRV_ARG(project->name));
                    //
                    //fprintf(stdout, "try to create dir: %s\n", handle.strv.data);
                    //
                    //cb_create_directories(buf, dir_n);
                    //
                    ////CB_ASSERT(dir_created);
                    //
                    //// output: toolchain_output_dir/ic/project_name/filename
                    //snprintf(buf, sizeof(buf), "%s/ic/" CB_STRV_FMT "/%zu-%zu-" CB_STRV_FMT, toolchain.default_directory_base, CB_STRV_ARG(project->name), file_id.volume_id, file_id.file_id, CB_STRV_ARG(maybe_src));

             
                    dep_file = cb_file_open_readwrite(handle.strv.data);
                    if (dep_file)
                    {
                        /* Current file ias actually part of the dependency */
                        cb_ibp_write_file_info_to_dep_file(dep_file, src);
                        /* @TODO we need to write the information of the current file as first entry of the dep file. */
                        
                       
                        fprintf(stdout, "<DEP_FILE:>%s<:DEP_FILE>\n", handle.strv.data);
                        }
                    cb_tmp_restore(handle.anchor);
                    
                }
                else
                {
                    src = strv_empty;
                }
            }
            else
            {
                src = strv_empty;
            }
            
            fprintf(stdout, "<SRC:>" CB_STRV_FMT "<:SRC>\n", CB_STRV_ARG(src));

        }
        
        c += 1;
    }
    
    (void)plugin;
    fprintf(stdout, "<A>\n%s</A>\n", std_out);
    fprintf(stdout, "<B>\n%s</B>\n", std_err);
    
    int nd = 0;
    cb_ibp_node* n = ipb->first;
    while (n)
    {
        printf("FILE DEBUG (%d): " CB_STRV_FMT  "\n", nd , CB_STRV_ARG( n->value));
        n = n->next;
        nd += 1;
    }
    cb_arena_reset(&ipb->arena);
    
    if (dep_file)
    {
        cb_file_close(dep_file);
    }
}

//static void cb_incremental_build_plugin_clear(cb_plugin* plugin)
//{
//     cb_incremental_build_plugin* ipb = (cb_incremental_build_plugin*)plugin;
//}
CB_INTERNAL void cb_ibp_destroy(cb_plugin* plugin)
{
    cb_incremental_build_plugin* ipb = (cb_incremental_build_plugin*)plugin;
     
    cb_arena_destroy(&ipb->arena);
     
    cb_log_important("cb_ibp_destroy\n");
}

CB_INTERNAL int cb_ibp_is_show_include_line(const char* line)
{
    static cb_strv prefixes[] = {
        // English
        CB_STRV("Note: including file: ")
        // @TODO other languages
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


CB_INTERNAL void cb_ibp_write_file_info_to_dep_file(FILE* dep_file, cb_strv dep_path)
{
    cb_size anchor = cb_tmp_save();
                    
    const char* dep_path_str = cb_tmp_strv_to_str(dep_path);

    /* @TODO: rename cb_fileid.h to cb_file_info.h */
    /* @TODO still write info if it fails with all zero ? */
    
    cb_file_info file_info = {0};
    if(!cb_get_file_info(dep_path_str, hash_buffer, (int)sizeof(hash_buffer), &file_info))
    {
        cb_log_error("could not get file info");
    }
    else
    {
        fprintf(dep_file, CB_STRV_FMT ";", CB_STRV_ARG(dep_path));
        fprintf(dep_file, CB_U64_FMT  ";", file_info.size);
        fprintf(dep_file, CB_U64_FMT  ";", file_info.last_modification);
        fprintf(dep_file, CB_U64_FMT  "\r\n", file_info.hash);
    }
    
    cb_tmp_restore(anchor);
}

CB_INTERNAL cb_bool cb_ibp_deserialize_file_info(FILE* file, char* buffer, int buffer_size, size_t* out_size, cb_file_info* file_info)
{
    CB_ASSERT(buffer_size >= 2048);
    
    if (fgets(buffer, (size_t)buffer_size, file))
    {
        /* Get span until char contains in */
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
        /* Skip separator */
        buffer += len + 1;
        
        //// Now you have a view: buffer[0..len]
        size_t n = sscanf_s(buffer,
                CB_U64_FMT ";"  CB_U64_FMT ";" CB_U64_FMT "\r\n",
                &file_info->size, &file_info->last_modification, &file_info->hash);

        if (n == 3)
        {
            return cb_true;
        }
    }

    return cb_false;
}

//static int cb_ibp_write_strv(cb_darrT(const char*) arr, cb_strv value)
//{
//    cb_darr_push_back_many(arr->darr, &value.size, sizeof(value.size),  sizeof(char));
//    cb_darr_push_back_many(arr->darr, value.data, value.size,  sizeof(char));
//
//}
//
//static int cb_ibp_read_strv(cb_darrT(const char*) arr, cb_strv* value)
//{
//    // @TODO read size before
//    memcpy(&value.size, arr->darr.data, sizeof(value.size));
//    cb_darr_remove_many(arr->darr, 0, sizeof(value.size), sizeof(char))
//    // @TODO read string content
//    memcpy(value.data, arr->darr.data, value->size);
//   
//    cb_darr_push_back_many(arr->darr, &value.size, sizeof(value.size), sizeof(char));
//    cb_darr_push_back_many(arr->darr, value.data, value.size,  sizeof(char));
//}