#ifndef CB_FILE_IO_H
#define CB_FILE_IO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Open file in read-only mode */
CB_API FILE* cb_file_open_readonly(const char* path);

/* Open file in write mode - overwrite */
CB_API FILE* cb_file_open_write(const char* path);

/* Write the content of a string view to a new file */
CB_API cb_bool cb_file_write_strv(const char* filepath, cb_strv sv);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CB_FILE_IO_H */

#ifdef CB_IMPLEMENTATION

#ifndef CB_FILE_IO_IMPL
#define CB_FILE_IO_IMPL

CB_INTERNAL FILE*
cb_file_open(const char* path, const char* mode)
{
#ifdef WIN32
	FILE* file = NULL;
	if (fopen_s(&file, path, mode) != 0)
#else
	FILE* file = fopen(path, mode);
	if (!file)
#endif
	{
		cb_log_error("cb_file_open: could not open file '%s' (%s)\n", path, mode);
		return NULL;
	}

	return file;
}

CB_API FILE*
cb_file_open_readonly(const char* path)
{
#ifdef WIN32
	const char* mode = "rb";
#else
	const char* mode = "r";
#endif
	return cb_file_open(path, mode);
}


CB_API FILE*
cb_file_open_write(const char* path)
{
#ifdef WIN32
	const char* mode = "wb";
#else
	const char* mode = "w";
#endif
	return cb_file_open(path, mode);
}

CB_API cb_bool
cb_file_write_strv(const char* filepath, cb_strv sv)
{
    FILE* f = cb_file_open_write(filepath);
    if (f)
    {
        size_t written = fwrite(sv.data, 1, sv.size, f);
        if (written != sv.size)
        {
            if (ferror(f))
            {
                cb_log_error("cb_file_write_strv: write error %s", filepath);
                return cb_false;
            }
            fclose(f);
            cb_log_error("cb_file_write_strv: error %s", filepath);
            return cb_false;
        }
    }

    if (fclose(f) == EOF) 
    {
        cb_log_error("cb_file_write_strv: close error %s", filepath);
        return cb_false;
    }
    
    return cb_true;
}

#endif /* CB_FILE_IO_IMPL */

#endif /* CB_IMPLEMENTATION */