#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CB_IMPLEMENTATION
#include <cb/cb.h>
#include <cb_extensions/cb_assert.h>
#include <cb_extensions/cb_file_io.h>
#include <cb_extensions/cb_gcc_dep_parser.h>

/* The incremental database is saved in the output directory of a specific project.
   Sharing the same output directory is likely something you will not want in this situation. */ 
int main(void)
{
    FILE* f = NULL;
    cb_strv value = { 0 };
    cb_gcc_dep_parser p = { 0 };

    char read_buffer[4096] = { 0 };
    char dep_buffer[4096] = { 0 };
    
    cb_gcc_dep_parser_init(&p, read_buffer, sizeof(read_buffer), dep_buffer, sizeof(dep_buffer));
    
    {
        f = cb_file_open_readonly("deps/empty.d");
        cb_gcc_dep_parser_reset(&p, f);

        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_false);
        CB_ASSERT(cb_strv_equals_str(value, ""));

        fclose(f);
    }
    
    {
        f = cb_file_open_readonly("deps/whitespaces.d");
        cb_gcc_dep_parser_reset(&p, f);

        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_false);
        CB_ASSERT(cb_strv_equals_str(value, ""));
        
        fclose(f);
    }
    
    
    {
        f = cb_file_open_readonly("deps/target_without_colon.d");
        cb_gcc_dep_parser_reset(&p, f);
        
        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_false);
        CB_ASSERT(cb_strv_equals_str(value, ""));
        
        fclose(f);
    }

    {
        f = cb_file_open_readonly("deps/target_without_deps.d");
        cb_gcc_dep_parser_reset(&p, f);

        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_false);
        CB_ASSERT(cb_strv_equals_str(value, ""));
        
        fclose(f);
    }

    {
        f = cb_file_open_readonly("deps/target_with_single_dep.d");

        cb_gcc_dep_parser_reset(&p, f);

        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "foo.h"));
        
        fclose(f);
    }
    
    {
        f = cb_file_open_readonly("deps/target_with_single_dep_ws.d");
        cb_gcc_dep_parser_reset(&p, f);
       
        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "foo.h"));
        
        fclose(f);
    }
    
    {
        f = cb_file_open_readonly("deps/target_with_multiple_deps_01.d");
        cb_gcc_dep_parser_reset(&p, f);

        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "foo.h"));
        
        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "bar.h"));
        
        fclose(f);
    }
    
    {
        f = cb_file_open_readonly("deps/target_with_multiple_deps_02.d");
        cb_gcc_dep_parser_reset(&p, f);

        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "b"));
        
        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "c"));
        
        fclose(f);
    }
    
    {
        f = cb_file_open_readonly("deps/target_with_multiple_deps_03.d");
        cb_gcc_dep_parser_reset(&p, f);

        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, " b"));
        
        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, " c"));
        
        fclose(f);
    }
    
    {
        f = cb_file_open_readonly("deps/target_with_multiple_deps_04.d");
        cb_gcc_dep_parser_reset(&p, f);

        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "b"));
        
        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "c"));
        
        fclose(f);
    }
      
    {
        f = cb_file_open_readonly("deps/target_with_multiple_deps_05.d");
        cb_gcc_dep_parser_reset(&p, f);

        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "b"));
        
        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "c"));
        
        fclose(f);
    }      
    {
        f = cb_file_open_readonly("deps/unusual_case_01.d");
        cb_gcc_dep_parser_reset(&p, f);

        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "bc"));
        
        fclose(f);
    }
    
    {
        f = cb_file_open_readonly("deps/real_example.d");
        cb_gcc_dep_parser_reset(&p, f);

        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "/path/src/foo.c"));
        
        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "/path/src/foo.h"));
        
        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "/path/src/common.h"));
        
        fclose(f);
    }
{
        f = cb_file_open_readonly("deps/real_example_lf.d");
        cb_gcc_dep_parser_reset(&p, f);

        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "/path/src/foo.c"));
        
        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "/path/src/foo.h"));
        
        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "/path/src/common.h"));
        
        fclose(f);
    }    
    return 0;
}