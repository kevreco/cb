#include <stdio.h>

#define CB_IMPLEMENTATION
#include "D:/kevin/project42/cb/cb/cb.h"
#include "D:/kevin\project42\cb/cb_extensions/cb_incremental_build.h"

#include <cb_extensions/cb_assert.h>

cb_incremental_build_plugin incremental_build_plugin;

#ifdef _WIN32
#define change_time_cmd "copy nul %s"
#else
#define change_time_cmd "touch %s"
#endif


#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <sys/utime.h>
    #include <io.h>
    #define utime _utime
    #define utimbuf _utimbuf
#else
    #include <utime.h>
    #include <unistd.h>
#endif


int fake_time = 0;


int change_time(const char *filename, time_t time)
{
    //// Try to create the file if it doesn't exist
    //FILE *fp = fopen(filename, "ab");
    //if (!fp) {
    //    perror("Failed to open file");
    //    return -1;
    //}
    //fclose(fp);


    // Set current time
    struct utimbuf new_times;
    new_times.actime = time;//time(NULL);    // access time
    new_times.modtime = time;//new_times.actime; // modification time

    if (utime(filename, &new_times) != 0)
    {
        perror("Failed to update timestamp");
        return -1;
    }

    return 0;
}

void set_to_zero_time(const char *filename)
{
    time_t zero = (time_t)(0);
    change_time(filename, zero);
}

time_t get_next_fake_time()
{
    fake_time += 1;
    return (time_t)(fake_time);
}


void set_to_next_fake_time(const char *filename)
{
    /* Increment by 2 in case the platform resolution is a single second */
    fake_time += 1;
    change_time(filename, (time_t)(fake_time));
}

void change_time2(const char* file)
{
    (void)file;
    
    cb_size anchor = cb_tmp_save();

    //const char* cmd = cb_tmp_sprintf(change_time_cmd, file);
    cb_process("copy nul D:/kevin/project42/cb/tests/extensions/cb_incremental_build/src/bar.c");
    cb_tmp_restore(anchor);
}

/* The incremental database is saved in the output directory of a specific project.
   Sharing the same output directory is likely something you will not want in this situation. */ 
int main(void)
{
    //const char* path = NULL;
    //const char* output = NULL;
    //cb_process_handle* handle;

    cb_plugin* plugins[] = {
        &incremental_build_plugin.plugin
    };
    
    cb_incremental_build_plugin_init(&incremental_build_plugin);
    
    cb_init_with_plugins(plugins, 1);
    

    cb_project("lib");
    cb_set(cb_BINARY_TYPE, cb_STATIC_LIBRARY);
    
    const char* bar_c = "D:/kevin/project42/cb/tests/extensions/cb_incremental_build/src/bar.c";
    const char* bar_h = "D:/kevin/project42/cb/tests/extensions/cb_incremental_build/src/bar.h";
    const char* foo_c = "D:/kevin/project42/cb/tests/extensions/cb_incremental_build/src/foo.c";
    const char* foo_h = "D:/kevin/project42/cb/tests/extensions/cb_incremental_build/src/foo.h";
    const char* common_h = "D:/kevin/project42/cb/tests/extensions/cb_incremental_build/src/common.h";
   
    cb_add(cb_FILES, bar_c);
    cb_add(cb_FILES, foo_c);
    
    
    cb_incremental_build_plugin_delete_cache(&incremental_build_plugin);
    
    set_to_zero_time(bar_c);
    set_to_zero_time(bar_h);
    set_to_zero_time(foo_c);
    set_to_zero_time(foo_h);
    set_to_zero_time(common_h);
  
    
    cb_bake();
    
    //cb_log_error("incremental_build_plugin.stat_ignored: %d", incremental_build_plugin.stat_ignored);
    //cb_log_error("incremental_build_plugin.stat_considered: %d", incremental_build_plugin.stat_considered);
    CB_ASSERT(incremental_build_plugin.stat_ignored == 0);
    CB_ASSERT(incremental_build_plugin.stat_considered == 2);
 
    set_to_next_fake_time(bar_c);
    
    cb_bake();
    
    cb_log_error("incremental_build_plugin.stat_ignored: %d", incremental_build_plugin.stat_ignored);
    //cb_log_error("incremental_build_plugin.stat_considered: %d", incremental_build_plugin.stat_considered);
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 1);
    CB_ASSERT(incremental_build_plugin.stat_considered == 1);
    
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 2);
    CB_ASSERT(incremental_build_plugin.stat_considered == 0);
    
    set_to_next_fake_time(foo_c);
    
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 1);
    CB_ASSERT(incremental_build_plugin.stat_considered == 1);
    
    set_to_next_fake_time(foo_h);
    
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 1);
    CB_ASSERT(incremental_build_plugin.stat_considered == 1);
    
    set_to_next_fake_time(bar_h);
    
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 1);
    CB_ASSERT(incremental_build_plugin.stat_considered == 1);
    
    set_to_next_fake_time(common_h);
    
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 0);
    CB_ASSERT(incremental_build_plugin.stat_considered == 2);
    
    set_to_next_fake_time(foo_h);
    set_to_next_fake_time(bar_h);
    
    cb_bake();  
    
    printf("%d", incremental_build_plugin.stat_ignored);
    CB_ASSERT(incremental_build_plugin.stat_ignored == 0);
    CB_ASSERT(incremental_build_plugin.stat_considered == 2);
    
    
    /* All file should be rebuilt if compiler flags are changed */
    
    
    cb_bake();  
    //handle = cb_process_to_string(path, NULL, 0);
    //
    //output = cb_process_stdout_string(handle);
    //
    //fprintf(stdout, "<Z>\n%s</Z>", output);
    //
    //cb_process_end(handle);
    
    return 0;

    //cb_extension_table ext[] = {
    //    cb_incremental_build_table()
    //};
    //
    //cb_init_with_extensions(ext, 1);
    //
    ///* First time */
    //{
    //    cb_project("foo");
    //
    //    cb_add_files(".", "*.txt");
    //    
    //    ib_set_dependency("a.txt", "a1.txt");
    //    ib_set_dependency("a1.txt", "a11.txt");
    //    
    //    ib_set_dependency("a.txt", "a2.txt");
    //    ib_set_dependency("a2.txt", "a22.txt");
    //    
    //    ib_build();
    //    
    //    change("a.txt");
    //    change("a1.txt");
    //    change("a11.txt");
    //  
    //    change("a2.txt");
    //    change("a22.txt");
    //    /* Everything must be rebuilt. */
    //    ib_build();
    //    
    //    change("b.txt");
    //    
    //    ib_build();
    //     
    //    change("c.txt");
    //    
    //     /* Only c.txt must be built. */
    //    ib_build();
    //    
    //    /* Nothing must be built. */
    //    ib_build();
    //
    //}
    //
    ///* @TODO cyclic graph. */
    //{
    //    cb_project("cyclic");
    //}
    //
    ///* @TODO with c files. */
    //{
    //    cb_project("with_c_files");
    //}
    //
    // /* @TODO changing flags must dirty everything?. */
    //{
    //    cb_project("change flags");
    //}
    //cb_destroy();

    //return 0;
}


