#ifndef CB_FILE_INFO_H
#define CB_FILE_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cb_file_info cb_file_info;
struct cb_file_info {
    cb_u64 volume_id; /* Volume (windows) or device ID (linux) */
    cb_u64 file_id;   /* file index (windows) or inode (linux) */
    cb_u64 size;             
    /*
        Last modification timestamps
        Resolution is platform dependent, could be 2 seconds resolution or a nanosecond resolution.
    */
    cb_u64 last_modification; 
    cb_u64 hash;      /* Hash of the file content */
};

enum {
  cb_file_info_NONE = 0,
  cb_file_info_VOLUME_ID = 1 << 0,
  cb_file_info_FILE_ID = 1 << 1,
  cb_file_info_SIZE = 1 << 2,
  cb_file_info_MODIFICATION_TIME = 1 << 3,
  cb_file_info_HASH = 1 << 4,
  cb_file_info_ALL = ~0
};

/*
    path: path of the file.
    buffer: buffer to use in case cb_file_info_HASH is used.
    buffer_size: size of the buffer to use in case cb_file_info_HASH is used.
    file_info: structure populated by the query.
    flags: to know which info to query
*/
CB_API cb_bool cb_file_info_query(const char *path, int flags, cb_file_info *file_info);
CB_API cb_bool cb_file_info_matches(const char *path, int flags, cb_file_info *file_info);

#ifdef __cplusplus
}
#endif

#endif /* CB_FILE_INFO_H */

#ifdef CB_IMPLEMENTATION

#ifndef CB_FILE_INFO_IMPL
#define CB_FILE_INFO_IMPL

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

/*-----------------------------------------------------------------------*/
/* API implementation */
/*-----------------------------------------------------------------------*/

CB_API cb_bool cb_file_info_query(const char *path, int flags, cb_file_info *file_info)
{
    HANDLE hFile = CreateFileA(
        path,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );


    if (hFile == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        cb_log_error("CreateFileA failed with error: %lu\n", err);
        cb_log_error("invalid handle: %s", path);
        return cb_false;
    }

    /* Get volume and file ids */
    if (flags & cb_file_info_FILE_ID)
    {
        BY_HANDLE_FILE_INFORMATION handle_info;
        if (!GetFileInformationByHandle(hFile, &handle_info))
        {
            cb_log_error("could not get file information:  %s", path);
            CloseHandle(hFile);
            return cb_false;
        }
        
        file_info->volume_id = handle_info.dwVolumeSerialNumber;
        file_info->file_id = ((cb_u64)handle_info.nFileIndexHigh << 32) | handle_info.nFileIndexLow;
    }

   
    /* Get File size */
    if (flags & cb_file_info_SIZE)
    {
        LARGE_INTEGER size;
        if (!GetFileSizeEx(hFile, &size))
        {
            cb_log_error("could not get file size");
            CloseHandle(hFile);
            return cb_false;
        } 
        
        file_info->size = (cb_u64)size.QuadPart;
    }
    
    /* Get file timestamps
       NOTE:
         FILETIME contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
    */
    if (flags & cb_file_info_MODIFICATION_TIME)
    {
        FILETIME creationTime, lastAccessTime, lastWriteTime;
        if (!GetFileTime(hFile, &creationTime, &lastAccessTime, &lastWriteTime))
        {
            cb_log_error("could not get file time");
            CloseHandle(hFile);
            return cb_false;
        }
        
        (void)creationTime; /* @TODO use this? */
        (void)lastAccessTime; /* @TODO use this? */
       
        file_info->last_modification = ( (cb_u64)lastWriteTime.dwLowDateTime | ( (cb_u64)lastWriteTime.dwHighDateTime << 32 ) );
    }

    if (flags & cb_file_info_HASH)
    {
        cb_u64 hash = 0;
        if (!cb_hash_64_file(hFile, &hash))
        {
            cb_log_error("could not get file hash");
            CloseHandle(hFile);
            return cb_false;
        }

        file_info->hash = hash;
    }
    
    CloseHandle(hFile);
 
    return cb_true;
}

CB_API cb_bool cb_file_info_matches(const char *path, int flags, cb_file_info *file_info)
{
    HANDLE hFile = CreateFileA(
        path,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );


    if (hFile == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        cb_log_error("CreateFileA failed with error: %lu\n", err);
        cb_log_error("invalid handle: %s", path);
        return cb_false;
    }

    /* Get file ids */
    if (flags & cb_file_info_FILE_ID)
    {
        BY_HANDLE_FILE_INFORMATION handle_info;
        if (!GetFileInformationByHandle(hFile, &handle_info))
        {
            cb_log_error("could not get file information:  %s", path);
            CloseHandle(hFile);
            return cb_false;
        }
        
        cb_u64 volume_id = handle_info.dwVolumeSerialNumber;
        
        if (file_info->volume_id != volume_id)
        {
            CloseHandle(hFile);
            return cb_false;
        }
        
        cb_u64 file_id = ((cb_u64)handle_info.nFileIndexHigh << 32) | handle_info.nFileIndexLow;
        
        if (file_info->file_id != file_id)
        {
            CloseHandle(hFile);
            return cb_false;
        }
    }
   
    /* Get File size */
    if (flags & cb_file_info_SIZE)
    {
        LARGE_INTEGER size;
        if (!GetFileSizeEx(hFile, &size))
        {
            cb_log_error("could not get file size");
            CloseHandle(hFile);
            return cb_false;
        } 
        
        cb_u64 size_ = (cb_u64)size.QuadPart;

        if (file_info->size != size_)
        {
            CloseHandle(hFile);
            return cb_false;
        }
    }

    /* Get file timestamps
       NOTE:
         FILETIME contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
    */
    if (flags & cb_file_info_MODIFICATION_TIME)
    {
        FILETIME creationTime, lastAccessTime, lastWriteTime;
        if (!GetFileTime(hFile, &creationTime, &lastAccessTime, &lastWriteTime))
        {
            cb_log_error("could not get file time");
            CloseHandle(hFile);
            return cb_false;
        }
        
        (void)creationTime; /* @TODO use this? */
        (void)lastAccessTime; /* @TODO use this? */
       
        cb_u64 last_modification = ( (cb_u64)lastWriteTime.dwLowDateTime | ( (cb_u64)lastWriteTime.dwHighDateTime << 32 ) );

        if (file_info->last_modification != last_modification)
        {
            CloseHandle(hFile);
            return cb_false;
        }
    }

    if (flags & cb_file_info_MODIFICATION_TIME)
    {
        cb_u64 hash = 0;
        if (!cb_hash_64_file(hFile, &hash))
        {
            cb_log_error("could not get file hash");
            CloseHandle(hFile);
            return cb_false;
        }

        if ( file_info->hash != hash)
        {
            CloseHandle(hFile);
            return cb_false;
        }
    }
    
    CloseHandle(hFile);
 
    return cb_true;
}

#else /* POSIX */

#include <sys/stat.h>

CB_API cb_bool cb_file_info_query(const char *path, int flags, cb_file_info *file_info)
{
    cb_u64 hash = 0;
    struct stat st;
    if (stat(path, &st) != 0)
    {
        return cb_false;
    }

    if (flags & cb_file_info_FILE_ID)
    {
        file_info->volume_id = (cb_u64)st.st_dev;
        file_info->file_id = (cb_u64)st.st_ino;
    }
    
    if (flags & cb_file_info_SIZE)
    {
        file_info->size = (cb_u64)st.st_size;
    }
    
    if (flags & cb_file_info_MODIFICATION_TIME)
    {
        file_info->last_modification = (cb_u64)st.st_mtime;
    }
    
    if (flags & cb_file_info_HASH)
    {
        if (!cb_hash_64_from_filename(path, &hash))
        {
            return cb_false;
        }
    }
    
    file_info->hash = hash;
  
    return cb_true;
}

CB_API cb_bool cb_file_info_matches(const char *path, int flags, cb_file_info *file_info)
{
    cb_u64 value = 0;
    
    struct stat st;
    if (stat(path, &st) != 0)
    {
        return cb_false;
    }

    if (flags & cb_file_info_FILE_ID)
    {
        cb_u64 volume_id = (cb_u64)st.st_dev;
        
        if (file_info->volume_id != volume_id)
        {
            return cb_false;
        }
        
        /* file_id */
        value = (cb_u64)st.st_ino;
        
        if (file_info->file_id != value)
        {
            return cb_false;
        }
    }
    
    if (flags & cb_file_info_SIZE)
    {
        cb_u64 size = (cb_u64)st.st_size;
        
        if (file_info->size != size)
        {
            return cb_false;
        }
    }
    
    if (flags & cb_file_info_MODIFICATION_TIME)
    {
        /* last_modification */
        value = (cb_u64)st.st_mtime;
        
        if (file_info->last_modification != value)
        {
            return cb_false;
        }
    }
    
    if (flags & cb_file_info_HASH)
    {
        /* get hash */
        
        if (!cb_hash_64_from_filename(path, &value))
        {
            return cb_false;
        }
        
        if (file_info->hash != value)
        {
            return cb_false;
        }
    }
  
    return cb_true;
}

#endif

#endif /* CB_FILE_INFO_IMPL */

#endif /* CB_IMPLEMENTATION */