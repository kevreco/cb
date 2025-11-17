#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CB_IMPLEMENTATION
#include <cb/cb.h>
#include <cb_extensions/cb_assert.h>
#include <cb_extensions/cb_file_io.h>
#include <cb_extensions/cb_dep_parser.h>

static void msvc_parser_tests();
static void gcc_parser_tests();

static const char* read_file_content(const char* filepath);
static void free_file_content(const char* content);

/* The incremental database is saved in the output directory of a specific project.
   Sharing the same output directory is likely something you will not want in this situation. */ 
int main(void)
{
    msvc_parser_tests();
    gcc_parser_tests();
    return 0;
}

static void msvc_parser_tests()
{
    const char* str = NULL;
    cb_strv value = { 0 };
    cb_dep_parser p = { 0 };

    cb_msvc_dep_parser_init(&p);
    
    {
        str = read_file_content("msvc/empty.txt");
        cb_msvc_dep_parser_reset(&p, str);

        CB_ASSERT(cb_msvc_dep_parser_get_next(&p, str, &value) == cb_false);

        free_file_content(str);
    }
    
    {
        str = read_file_content("msvc/target_without_deps.txt");
        cb_msvc_dep_parser_reset(&p, str);

        CB_ASSERT(cb_msvc_dep_parser_get_next(&p, str, &value) == cb_false);

        free_file_content(str);
    }
    
    {
        str = read_file_content("msvc/target_with_single_dep.txt");
        cb_msvc_dep_parser_reset(&p, str);

        CB_ASSERT(cb_msvc_dep_parser_get_next(&p, str, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "foo.h"));

        free_file_content(str);
    }
    
    {
        str = read_file_content("msvc/target_with_multiple_deps_01.txt");
        cb_msvc_dep_parser_reset(&p, str);

        CB_ASSERT(cb_msvc_dep_parser_get_next(&p, str, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "foo.h"));
        
         CB_ASSERT(cb_msvc_dep_parser_get_next(&p, str, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "bar.h"));

        free_file_content(str);
    }
    
    {
        str = read_file_content("msvc/target_with_multiple_deps_02.txt");
        cb_msvc_dep_parser_reset(&p, str);

        CB_ASSERT(cb_msvc_dep_parser_get_next(&p, str, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "my/p a t h/foo.h"));
        
         CB_ASSERT(cb_msvc_dep_parser_get_next(&p, str, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "my/p a t h/bar.h"));

        free_file_content(str);
    }

    {
        str = read_file_content("msvc/real_life_example.txt");
        cb_msvc_dep_parser_reset(&p, str);

        while(cb_msvc_dep_parser_get_next(&p, str, &value))
        {
            /* Do nothing */
            continue;
        }

        free_file_content(str);
    }
}

static void gcc_parser_tests()
{
    FILE* f = NULL;
    cb_strv value = { 0 };
    cb_dep_parser p = { 0 };

    char read_buffer[4096] = { 0 };
    char dep_buffer[4096] = { 0 };
    
    cb_gcc_dep_parser_init(&p, read_buffer, sizeof(read_buffer), dep_buffer, sizeof(dep_buffer));
    
    {
        f = cb_file_open_readonly("gcc/empty.d");
        cb_gcc_dep_parser_reset(&p, f);

        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_false);
        CB_ASSERT(cb_strv_equals_str(value, ""));

        fclose(f);
    }
    
    {
        f = cb_file_open_readonly("gcc/whitespaces.d");
        cb_gcc_dep_parser_reset(&p, f);

        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_false);
        CB_ASSERT(cb_strv_equals_str(value, ""));
        
        fclose(f);
    }
    
    
    {
        f = cb_file_open_readonly("gcc/target_without_colon.d");
        cb_gcc_dep_parser_reset(&p, f);
        
        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_false);
        CB_ASSERT(cb_strv_equals_str(value, ""));
        
        fclose(f);
    }

    {
        f = cb_file_open_readonly("gcc/target_without_deps.d");
        cb_gcc_dep_parser_reset(&p, f);

        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_false);
        CB_ASSERT(cb_strv_equals_str(value, ""));
        
        fclose(f);
    }

    {
        f = cb_file_open_readonly("gcc/target_with_single_dep.d");

        cb_gcc_dep_parser_reset(&p, f);

        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "foo.h"));
        
        fclose(f);
    }
    
    {
        f = cb_file_open_readonly("gcc/target_with_single_dep_ws.d");
        cb_gcc_dep_parser_reset(&p, f);
       
        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "foo.h"));
        
        fclose(f);
    }
    
    {
        f = cb_file_open_readonly("gcc/target_with_multiple_deps_01.d");
        cb_gcc_dep_parser_reset(&p, f);

        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "foo.h"));
        
        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "bar.h"));
        
        fclose(f);
    }
    
    {
        f = cb_file_open_readonly("gcc/target_with_multiple_deps_02.d");
        cb_gcc_dep_parser_reset(&p, f);

        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "b"));
        
        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "c"));
        
        fclose(f);
    }
    
    {
        f = cb_file_open_readonly("gcc/target_with_multiple_deps_03.d");
        cb_gcc_dep_parser_reset(&p, f);

        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, " b"));
        
        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, " c"));
        
        fclose(f);
    }
    
    {
        f = cb_file_open_readonly("gcc/target_with_multiple_deps_04.d");
        cb_gcc_dep_parser_reset(&p, f);

        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "b"));
        
        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "c"));
        
        fclose(f);
    }
      
    {
        f = cb_file_open_readonly("gcc/target_with_multiple_deps_05.d");
        cb_gcc_dep_parser_reset(&p, f);

        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "b"));
        
        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "c"));
        
        fclose(f);
    }      
    {
        f = cb_file_open_readonly("gcc/unusual_case_01.d");
        cb_gcc_dep_parser_reset(&p, f);

        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "bc"));
        
        fclose(f);
    }
    
    {
        f = cb_file_open_readonly("gcc/real_example.d");
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
        f = cb_file_open_readonly("gcc/real_example_lf.d");
        cb_gcc_dep_parser_reset(&p, f);

        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "/path/src/foo.c"));
        
        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "/path/src/foo.h"));
        
        CB_ASSERT(cb_gcc_dep_parser_get_next(&p, f, &value) == cb_true);
        CB_ASSERT(cb_strv_equals_str(value, "/path/src/common.h"));
        
        fclose(f);
    }
}

static const char* read_file_content(const char* filepath)
{
    FILE* fp = cb_file_open_readonly(filepath);
    char* data;
    long size;

    if (!fp)
    {
        return NULL;
    }

    /* get file size */
    if (fseek(fp, 0, SEEK_END) != 0)
    {
        fclose(fp);
        return NULL;
    }

    size = ftell(fp);
    if (size < 0)
    {
        fclose(fp);
        return NULL;
    }

    /* allocate buffer */
    data = (char*)calloc(1, (size_t)size + 1);
    if (!data)
    {
        fclose(fp);
        return NULL;
    }

    /* read the whole file */
    rewind(fp);
    if (fread((char*)data, 1, (size_t)size, fp) != (size_t)size)
    {
        free((void*)data);
        fclose(fp);
        return NULL;
    }

    fclose(fp);

    data[size] = '\0';
    return data;
}

static void free_file_content(const char* str)
{
    free((void*)str);
}