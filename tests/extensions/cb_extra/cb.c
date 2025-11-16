/* Deactive warning for getenv on Windows. */
#if _WIN32
  #define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CB_IMPLEMENTATION
#include <cb/cb.h>
#include <cb_extensions/cb_extra.h>
#include <cb_extensions/cb_assert.h>

int main(void)
{
    const char* env_var = NULL;
    cb_init();

    cb_project("lib");
    cb_set(cb_BINARY_TYPE, cb_STATIC_LIBRARY);

    cb_add(cb_FILES, "src/foo.c");
    
    /* Only check this on the CI because the file will already exists on local computer when run multiple times. 
       @TODO fix this.
    */
    env_var = getenv("GITHUB_ACTIONS"); 
    if (env_var)
    {
        /* Binary is not supposed to exist. */
        if (cb_baked_binary_already_exists())
        {
            return -1;
        }
    }

    cb_bake();

    /* Binary was baked so it must exist. */
    if (cb_baked_binary_already_exists())
    {
        return 0;
    }
    
    return -1;
}

