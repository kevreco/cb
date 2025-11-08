#include <stdio.h>

#define CB_IMPLEMENTATION
/* @TODO change this to <cb/cb.h> etc */
#include "cb/cb.h"
#include "cb_extensions/cbp_incremental_build.h"

#include <cb_extensions/cb_assert.h>

cbp_incremental_build incremental_build_plugin;

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
    /* Set current time */
    struct utimbuf new_times;
    new_times.actime = time;  /* access time  */
    new_times.modtime = time; /* modification time */

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

/* The incremental database is saved in the output directory of a specific project.
   Sharing the same output directory is likely something you will not want in this situation. */ 
int main(void)
{
    const char* bar_c = "src/bar.c";
    const char* bar_h = "src/bar.h";
    const char* foo_c = "src/foo.c";
    const char* foo_h = "src/foo.h";
    const char* common_h = "src/common.h";
    
    cb_plugin* plugins[] = {
        &incremental_build_plugin.plugin
    };
    
    cbp_incremental_build_init(&incremental_build_plugin);
    
    cb_init_with_plugins(plugins, 1);
    

    cb_project("lib");
    cb_set(cb_BINARY_TYPE, cb_STATIC_LIBRARY);
    
    //const char* bar_c = "D:/kevin/project42/cb/tests/extensions/cb_incremental_build/src/bar.c";
    //const char* bar_h = "D:/kevin/project42/cb/tests/extensions/cb_incremental_build/src/bar.h";
    //const char* foo_c = "D:/kevin/project42/cb/tests/extensions/cb_incremental_build/src/foo.c";
    //const char* foo_h = "D:/kevin/project42/cb/tests/extensions/cb_incremental_build/src/foo.h";
    //const char* common_h = "D:/kevin/project42/cb/tests/extensions/cb_incremental_build/src/common.h";
    
    /* To test file with space */
    cb_add(cb_FILES, "src/bar/b ar.c");
    /* To test file with same name but different paths */
    cb_add(cb_FILES, "src/bar/bar.c");
    cb_add(cb_FILES, bar_c);
    cb_add(cb_FILES, foo_c);
    
    
    cbp_incremental_build_delete_cache(&incremental_build_plugin);
    
    set_to_zero_time(bar_c);
    set_to_zero_time(bar_h);
    set_to_zero_time(foo_c);
    set_to_zero_time(foo_h);
    set_to_zero_time(common_h);
  
    
    cb_bake();
    
    cb_log_error("incremental_build_plugin.stat_ignored: %d", incremental_build_plugin.stat_ignored);
    cb_log_error("incremental_build_plugin.stat_compilable: %d", incremental_build_plugin.stat_compilable);
    CB_ASSERT(incremental_build_plugin.stat_ignored == 0);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 2);
 
    set_to_next_fake_time(bar_c);
    
    cb_bake();
    
    cb_log_error("incremental_build_plugin.stat_ignored: %d", incremental_build_plugin.stat_ignored);
    cb_log_error("incremental_build_plugin.stat_compilable: %d", incremental_build_plugin.stat_compilable);
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 1);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 1);
    
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 2);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 0);
    
    set_to_next_fake_time(foo_c);
    
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 1);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 1);
    
    set_to_next_fake_time(foo_h);
    
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 1);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 1);
    
    set_to_next_fake_time(bar_h);
    
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 1);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 1);
    
    set_to_next_fake_time(common_h);
    
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 0);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 2);
    
    set_to_next_fake_time(foo_h);
    set_to_next_fake_time(bar_h);
    
    cb_bake();  
    
    printf("%d", incremental_build_plugin.stat_ignored);
    CB_ASSERT(incremental_build_plugin.stat_ignored == 0);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 2);
    
    
    /* All file should be rebuilt if compiler flags are changed */
    
    cb_bake();
    CB_ASSERT(incremental_build_plugin.stat_ignored == 2);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 0);
    
    cb_add(cb_CXFLAGS, "O2");
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 0);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 2);
    
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 2);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 0);
    
    cb_add(cb_DEFINES, "UNICODE");
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 0);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 2);
    
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 2);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 0);
    
    return 0;
}


