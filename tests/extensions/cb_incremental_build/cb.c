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

#define CB_IMPLEMENTATION
#include <cb/cb.h>
#include <cb_extensions/cbp_incremental_build.h>
#include <cb_extensions/cb_assert.h>

static cbp_incremental_build incremental_build_plugin;

static int fake_time = 0;

static int change_time(const char *filename, time_t time)
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

static void set_to_zero_time(const char *filename)
{
    time_t zero = (time_t)(0);
    change_time(filename, zero);
}

static void set_to_next_fake_time(const char *filename)
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

    cb_add(cb_FILES, bar_c);
    cb_add(cb_FILES, foo_c);

    cbp_incremental_build_delete_cache(&incremental_build_plugin);
    
    set_to_zero_time(bar_c);
    set_to_zero_time(bar_h);
    set_to_zero_time(foo_c);
    set_to_zero_time(foo_h);
    set_to_zero_time(common_h);

    cb_bake();
    
    /* First build after all files were changed. All compilation unit must be rebuilt. */
    CB_ASSERT(incremental_build_plugin.stat_ignored == 0);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 2);
 
    set_to_next_fake_time(bar_c);
    
    /* Only one file must be rebuild after change bar.c */
    cb_bake();

    CB_ASSERT(incremental_build_plugin.stat_ignored == 1);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 1);
    
    /* No file must be rebuild if we don't change any file. */
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 2);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 0);

    set_to_next_fake_time(foo_c);
    
    /* Only one file must be rebuild after change foo.c */
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 1);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 1);
    
    set_to_next_fake_time(foo_h);
    
    /* Only one file must be rebuild after change foo.h */
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 1);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 1);
    
    /* Only one file must be rebuild after change bar.h */
    set_to_next_fake_time(bar_h);
    
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 1);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 1);
    
    set_to_next_fake_time(common_h);
    
    /* bar.c and foo.c must be rebuild after change common.h */
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 0);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 2);
    
    set_to_next_fake_time(foo_h);
    set_to_next_fake_time(bar_h);
    
    /* bar.c and foo.c must be rebuild after change bar.h and foo.h */
    cb_bake();  

    CB_ASSERT(incremental_build_plugin.stat_ignored == 0);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 2);
    
    /* No file must be rebuild if we don't change any file. */
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 2);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 0);
    
    /* Changing any flag must invalide all compilation unit. */
    #ifdef _WIN32
    cb_add(cb_CXFLAGS, "/O2");
    #else
       cb_add(cb_CXFLAGS, "-O2");
    #endif
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 0);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 2);
    
    /* No file must be rebuild if we don't change any file. */
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 2);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 0);
    
    /* Changing the defines must invalide all compilation unit. */
    cb_add(cb_DEFINES, "UNICODE");
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 0);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 2);
    
    /* No file must be rebuild if we don't change any file. */
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 2);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 0);
    
    /* Changing the include search directories must invalide all compilation unit. */
    cb_add(cb_INCLUDE_DIRECTORIES, "./");
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 0);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 2);
    
    /* Make sure nothing get recompiled after 0 change. */
    cb_bake();
    
    CB_ASSERT(incremental_build_plugin.stat_ignored == 2);
    CB_ASSERT(incremental_build_plugin.stat_compilable == 0);
    
    return 0;
}

