#ifndef CB_FILE_INFO_H
#define CB_FILE_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cb_file_id cb_file_id;
struct cb_file_id {
    cb_u64 volume_id; /* Volume (windows) or device ID (linux) */
    cb_u64 file_id;   /* file index (windows) or inode (linux) */
};

typedef struct cb_file_info cb_file_info;
struct cb_file_info {
    cb_u64 volume_id;        /* Volume (windows) or device ID (linux) */
    cb_u64 file_id;          /* file index (windows) or inode (linux) */
    cb_u64 size;             
    cb_u64 hash;             /* Hash of the content */
    /*
        Last modification timestamps
        Resolution is platform dependent, could be 2 seconds resolution or a nanosecond resolution.
    */
    cb_u64 last_modification; 
};

/* Get a unique identifier for the file at the given path. */
CB_INTERNAL cb_bool cb_get_file_id(const char* path, cb_file_id* file_id);
CB_INTERNAL cb_bool cb_get_file_info(const char *path, char* buffer, int buffer_size, cb_file_info *file_info);

#ifdef __cplusplus
}
#endif

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

CB_INTERNAL cb_bool cb_get_file_id(const char *path, cb_file_id *file_id)
{
    HANDLE hFile = CreateFileA(
        path,
        0,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return cb_false;
    }

    BY_HANDLE_FILE_INFORMATION info;
    if (!GetFileInformationByHandle(hFile, &info))
    {
        CloseHandle(hFile);
        return cb_false;
    }

    CloseHandle(hFile);

    file_id->volume_id = info.dwVolumeSerialNumber;
    file_id->file_id = ((cb_u64)info.nFileIndexHigh << 32) | info.nFileIndexLow;

    return cb_true;
}

CB_INTERNAL time_t cb_get_file_info_to_unix_time(const FILETIME *ft) {
    // Windows FILETIME is in 100-nanosecond intervals since Jan 1, 1601 (UTC)
    // Unix epoch is Jan 1, 1970
    const int64_t WINDOWS_TICK = 10000000LL;  // 1 second = 10^7 ticks
    const int64_t SEC_TO_UNIX_EPOCH = 11644473600LL; // Seconds from 1601 to 1970

    ULARGE_INTEGER ull;
    ull.LowPart = ft->dwLowDateTime;
    ull.HighPart = ft->dwHighDateTime;

    return (time_t)((ull.QuadPart / WINDOWS_TICK) - SEC_TO_UNIX_EPOCH);
}


CB_INTERNAL cb_bool cb_get_file_info(const char *path, char* buffer, int buffer_size, cb_file_info *file_info)
{
    //path = "D:\\kevin\\project42\\cb\\tests\\extensions\\cb_incremental_build\\src\\common.h";
    
    HANDLE hFile = CreateFileA(
        path,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    cb_log_important("PATH: [%s]", path);
    
    if (hFile == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        cb_log_error("CreateFileA failed with error: %lu\n", err);
        cb_log_error("invalid handle: %s", path);
        return cb_false;
    }

    /* Get file ids */
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
       
        //file_info->last_modification = cb_get_file_info_to_unix_time(&lastWriteTime);
        file_info->last_modification = ( (uint64_t)lastWriteTime.dwLowDateTime | ( (uint64_t)lastWriteTime.dwHighDateTime << 32 ) );
    }

    cb_u64 hash = 0;
    if (!cb_hash_64_file(hFile, buffer, buffer_size, &hash))
    {
        cb_log_error("could not get file hash");
        CloseHandle(hFile);
        return cb_false;
    }

    file_info->hash = hash;
    
    CloseHandle(hFile);
 
    return cb_true;
}

/* @TODO add prototype at top. */
CB_INTERNAL cb_bool cb_file_info_matches(const char *path, char* buffer, int buffer_size, cb_file_info *file_info)
{
    //path = "D:\\kevin\\project42\\cb\\tests\\extensions\\cb_incremental_build\\src\\common.h";
    
    HANDLE hFile = CreateFileA(
        path,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    cb_log_important("PATH: [%s]", path);
    
    if (hFile == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        cb_log_error("CreateFileA failed with error: %lu\n", err);
        cb_log_error("invalid handle: %s", path);
        return cb_false;
    }

    /* Get file ids */
    //{
    //    BY_HANDLE_FILE_INFORMATION handle_info;
    //    if (!GetFileInformationByHandle(hFile, &handle_info))
    //    {
    //        cb_log_error("could not get file information:  %s", path);
    //        CloseHandle(hFile);
    //        return cb_false;
    //    }
    //    
    //    file_info->volume_id = handle_info.dwVolumeSerialNumber;
    //    file_info->file_id = ((cb_u64)handle_info.nFileIndexHigh << 32) | handle_info.nFileIndexLow;
    //}
    

   
    /* Get File size */
    {
        LARGE_INTEGER size;
        if (!GetFileSizeEx(hFile, &size))
        {
            cb_log_error("could not get file size");
            CloseHandle(hFile);
            return cb_false;
        } 
        
        cb_u64 size_ = (cb_u64)size.QuadPart;
        
        cb_log_important("MATCH?: " CB_U64_FMT "==" CB_U64_FMT, size_, file_info->size);
        
        if (file_info->size != size_)
        {
            return cb_false;
        }
    }

    /* Get file timestamps
       NOTE:
         FILETIME contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
    */
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
       
        cb_u64 last_modification = ( (uint64_t)lastWriteTime.dwLowDateTime | ( (uint64_t)lastWriteTime.dwHighDateTime << 32 ) );
        //cb_u64 last_modification = cb_get_file_info_to_unix_time(&lastWriteTime);
        
        cb_log_important("MATCH?: " CB_U64_FMT "==" CB_U64_FMT, file_info->last_modification, last_modification);
        
        
        if (file_info->last_modification != last_modification)
        {
            return cb_false;
        }
    }

    cb_u64 hash = 0;
    if (!cb_hash_64_file(hFile, buffer, buffer_size, &hash))
    {
        cb_log_error("could not get file hash");
        CloseHandle(hFile);
        return cb_false;
    }

    cb_log_important("MATCH?: " CB_U64_FMT "==" CB_U64_FMT, file_info->hash, hash);

    if ( file_info->hash != hash)
    {
        return cb_false;
    }
    
    CloseHandle(hFile);
 
    return cb_true;
}

#else /* POSIX */

#include <sys/stat.h>
/* #include <unistd.h>  @TODO remove if not used */ 

CB_INTERNAL cb_bool cb_get_file_id(const char *path, cb_file_id *file_id)
{
    struct stat st;
    if (stat(path, &st) != 0)
    {
        return cb_false;
    }

    file_id->volume_id = (cb_u64)st.st_dev;
    file_id->file_id = (cb_u64)st.st_ino;

    return cb_true;
}

CB_INTERNAL cb_bool cb_get_file_info(const char *path, char* buffer, size_t buffer_size, cb_file_info *file_info)
{
    struct stat st;
    if (stat(path, &st) != 0)
    {
        return cb_false;
    }

    file_info->volume_id = (cb_u64)st.st_dev;
    file_info->file_id = (cb_u64)st.st_ino;
    file_info->size = (cb_u64)st.st_size;
    file_info->last_modification = (cb_u64)sb.st_mtime;
    
    cb_u64 hash = 0;
    if (!cb_hash_64_from_filename(path, buffer, buffer_size, &hash))
    {
        return cb_false;
    }
    
    file_info->hash = hash;
  
    return cb_true;
}

#endif

#endif // CB_FILE_INFO_H