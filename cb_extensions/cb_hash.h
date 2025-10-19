#ifndef CB_HASH_H
#define CB_HASH_H

#include "cb_file_io.h"

#ifdef __cplusplus
extern "C" {
#endif

CB_API cb_u64 cb_hash_64(char* str, int size);
CB_API cb_u64 cb_hash_64_init();
CB_API cb_u64 cb_hash_64_combine(cb_u64 value, char* str, int size);
CB_API cb_u64 cb_hash_64_str(char* str);

#ifdef WIN32
CB_API cb_bool cb_hash_64_file(HANDLE hFile, cb_u64* hash);
#endif

CB_API cb_bool cb_hash_64_from_filename(const char* filename, cb_u64* hash);

#ifdef __cplusplus
}
#endif

#endif /* CB_HASH_H */

#ifdef CB_IMPLEMENTATION

#ifndef CB_HASH_IMPL
#define CB_HASH_IMPL

#define CB_FNV1A_64_SEED 14695981039346656037ULL
#define CB_FNV1A_64_PRIME 1099511628211ULL

#define CB_FNV1A_32_SEED 2166136261U
#define CB_FNV1A_32_PRIME 16777619U

typedef cb_u32 cb_fnv1a_32;
typedef cb_u64 cb_fnv1a_64;

CB_INTERNAL cb_fnv1a_32 fnv1a_32_make()
{
    return CB_FNV1A_32_SEED;
}

CB_INTERNAL cb_fnv1a_32 fnv1a_32_update(cb_fnv1a_32 state, char* data, int size)
{
    int i = 0;
    for (i = 0; i < size; i += 1)
    {
        state = (state ^ data[i]) * CB_FNV1A_32_PRIME;
    }
    return state;
}

/* Take a null-terminated string */
CB_INTERNAL cb_fnv1a_32 cb_fnv1a_32_update_str(cb_fnv1a_32 state, char* data)
{
    while (*data)
    {
        state = (state ^ data[0]) * CB_FNV1A_32_PRIME;
        data += 1;
    }
    return state;
}
     
CB_INTERNAL cb_fnv1a_64 cb_fnv1a_64_make()
{
    return CB_FNV1A_64_SEED;
}
            
CB_INTERNAL cb_fnv1a_64 cb_fnv1a_64_update(cb_fnv1a_64 state, char* data, int size)
{
    int i = 0;
    for (i = 0; i < size; i += 1)
    {
        state = (state ^ data[i]) * CB_FNV1A_64_PRIME;
    }
    return state;
}

CB_INTERNAL cb_fnv1a_64 cb_fnv1a_64_update_str(cb_fnv1a_64 state, char* data)
{
    while (*data)
    {
        state = (state ^ data[0]) * CB_FNV1A_64_PRIME;
        data += 1;
    }
    return state;
}

/*-----------------------------------------------------------------------*/
/* API implementation */
/*-----------------------------------------------------------------------*/

CB_API cb_u64 cb_hash_64(char* str, int size)
{
    return cb_fnv1a_64_update(cb_fnv1a_64_make(), str, size);
}

CB_API cb_u64 cb_hash_64_init()
{
    return cb_fnv1a_64_make();
}

CB_API cb_u64 cb_hash_64_combine(cb_u64 value, char* str, int size)
{
    return cb_fnv1a_64_update(value, str, size);
}

CB_API cb_u64 cb_hash_64_str(char* str)
{
    return cb_fnv1a_64_update_str(cb_fnv1a_64_make(), str);
}

#ifdef WIN32

CB_API cb_bool cb_hash_64_file(HANDLE hFile, cb_u64* hash)
{
    DWORD bytesRead;
    
    cb_fnv1a_64 state = cb_fnv1a_64_make();
    
    cb_u32 buffer_size = 16 * 4096;
    char* buffer = cb_tmp_alloc(buffer_size);
    
    do
    {
        if (!ReadFile(hFile, buffer, buffer_size, &bytesRead, NULL))
        {
            return cb_false;
        }

        if (bytesRead > 0)
        {
            state = cb_fnv1a_64_update(state, buffer, (cb_u64)bytesRead);
        }

    } while (bytesRead > 0);
    
    *hash = state;
    
    return cb_true;
}

#endif

CB_API cb_bool cb_hash_64_from_filename(const char* filename, cb_u64* hash)
{
    cb_fnv1a_64 state = {0};
    size_t count = 0;
    cb_u32 buffer_size = 16 * 4096;
    char* buffer = NULL;
      
	FILE* file = cb_file_open_readonly(filename);
	if (!file)
	{
		cb_log_error("cb_hash_64_from_filename: could not open file '%s'\n", filename);
		return 0;
	}
    
    state = cb_fnv1a_64_make();
    
    buffer = cb_tmp_alloc(buffer_size);
 
    while ((count = fread(buffer, 1, buffer_size, file)) != 0)
    {
        state = cb_fnv1a_64_update(state, buffer, (int)count);
    }

    *hash = state;
    
    if (!feof(file))
    {
        fclose(file);
        return  cb_false;
    }
    
    fclose(file);
    
    return cb_true;
}

#endif /* CB_HASH_IMPL */

#endif /* CB_IMPLEMENTATION */
