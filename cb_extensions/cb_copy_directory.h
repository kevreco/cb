#ifndef CB_COPY_DIRECTORY_H
#define CB_COPY_DIRECTORY_H

#include "cb_file_it.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Recursively copy the content of the directory in another one, empty directories will be omitted. */
CB_API cb_bool cb_copy_directory(const char* source_dir, const char* target_dir);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CB_COPY_DIRECTORY_H */

#ifdef CB_IMPLEMENTATION

CB_API cb_bool
cb_copy_directory(const char* source_dir, const char* target_dir)
{
	cb_file_it it;
	cb_bool result = cb_true;
	cb_size tmp_save = 0; 
	char* dest_buffer = NULL;
	cb_size n = 0;
	const char* source_relative_path = NULL;

	tmp_save = cb_tmp_save();
	dest_buffer = cb_tmp_alloc(CB_MAX_PATH);
	cb_file_it_init_recursive(&it, source_dir);

	while (cb_file_it_get_next(&it))
	{
		/* copy current directory */
		source_relative_path = it.current_file + it.dir_len_stack[1];

		n = 0;
		n += cb_str_append_from(dest_buffer, target_dir, n, CB_MAX_PATH);
		n += cb_ensure_trailing_dir_separator(dest_buffer, n);
		n += cb_str_append_from(dest_buffer, source_relative_path, n, CB_MAX_PATH);

		if(!cb_copy_file(it.current_file, dest_buffer))
		{
			cb_log_error("Could not copy directory '%s' to '%s'", source_dir, target_dir);
			cb_set_and_goto(result, cb_false, exit);
		}
	}

exit:

	cb_tmp_restore(tmp_save);
	return result;
}

#endif /* CB_IMPLEMENTATION */