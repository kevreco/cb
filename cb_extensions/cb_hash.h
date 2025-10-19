#ifndef CB_HASH_H
#define CB_HASH_H

/* @TODO use CB_INTERNAL instead of static inline */

#define CB_FNV1A_32_SEED 2166136261U
#define CB_FNV1A_32_PRIME 16777619U

typedef cb_u32 cb_fnv1a_32;

static inline cb_fnv1a_32 fnv1a_32_make()
{
    return CB_FNV1A_32_SEED;
}

static inline cb_fnv1a_32 fnv1a_32_update(cb_fnv1a_32 state, char* data, int size)
{
    for (int i = 0; i < size; ++i)
    {
        state = (state ^ data[i]) * CB_FNV1A_32_PRIME;
    }
    return state;
}

/* Take a null-terminated string */
static inline cb_fnv1a_32 cb_fnv1a_32_update_str(cb_fnv1a_32 state, char* data)
{
    while (*data)
    {
        state = (state ^ data[0]) * CB_FNV1A_32_PRIME;
        data += 1;
    }
    return state;
}
            
#define CB_FNV1A_64_SEED 14695981039346656037ULL
#define CB_FNV1A_64_PRIME 1099511628211ULL

typedef cb_u64 cb_fnv1a_64;
     
static inline cb_fnv1a_64 cb_fnv1a_64_make()
{
    return CB_FNV1A_64_SEED;
}
            
static inline cb_fnv1a_64 cb_fnv1a_64_update(cb_fnv1a_64 state, char* data, int size)
{
    for (int i = 0; i < size; ++i)
    {
        state = (state ^ data[i]) * CB_FNV1A_64_PRIME;
    }
    return state;
}

static inline cb_fnv1a_64 cb_fnv1a_64_update_str(cb_fnv1a_64 state, char* data)
{
    while (*data)
    {
        state = (state ^ data[0]) * CB_FNV1A_64_PRIME;
        data += 1;
    }
    return state;
}

static inline cb_u64 cb_hash_64(char* str, int size)
{
    return cb_fnv1a_64_update(cb_fnv1a_64_make(), str, size);
}

static inline cb_u64 cb_hash_64_str(char* str)
{
    return cb_fnv1a_64_update_str(cb_fnv1a_64_make(), str);
}

#ifdef WIN32
static inline cb_bool cb_hash_64_file(HANDLE hFile, char* buffer, cb_u32 buffer_size, cb_u64* hash)
{
    DWORD bytesRead;
    
    cb_fnv1a_64 state = cb_fnv1a_64_make();
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
#else
static inline  cb_hash_64_from_filename(char* filename, cb_u64* hash)
{
	const char* read_only_mode = "r";

	FILE* file = fopen(filename, read_only_mode);
	if (!file)
	{
		fprintf(stderr, "cb_hash_64_from_filename: could not open file '%s'\n", filename);
		return 0;
	}
    
    cb_fnv1a_64 state = cb_fnv1a_64_make();

    size_t count;

    while ((count = fread(buffer, 1, buffer_size, f)) != 0)
    {
        state = cb_fnv1a_64_update(state, buffer, (int)count);
    }

    *hash = state;
    
    if (!feof(fp))
    {
        fclose(file);
        return  cb_false;
    }
    
    fclose(file);
    
    return cb_true;
}
#endif



#endif /* CB_HASH_H */