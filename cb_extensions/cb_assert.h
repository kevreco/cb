#ifndef CB_ASSERT_H
#define CB_ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

CB_API void cb_assert_file_exists(const char* filepath);
CB_API void cb_assert_file_exists_f(const char* format, ...);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CB_ASSERT_H */

#ifdef CB_IMPLEMENTATION

CB_API void
cb_assert_file_exists(const char* filepath)
{
	if (!cb_path_exists(filepath))
	{
		cb_log_error("File does not exist: %s", filepath);
		exit(1);
	}
}

CB_API void
cb_assert_file_exists_f(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	
	const char* str = cb_tmp_vsprintf(format, args);
	cb_assert_file_exists(str);

	va_end(args);
}

#endif /* CB_IMPLEMENTATION */