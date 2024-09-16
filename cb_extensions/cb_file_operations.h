#ifndef CB_FILE_OP_H
#define CB_FILE_OP_H

/*
* This extension rely on the cb_file_it.h extension
*
* - cb_copy_directory
* - cb_delete_file
* - cb_move_file
* - cb_move_files
*/

/* we don't really need to*/
/* recursively copy the content of the directory in another one, empty directory will be omitted */
CB_INTERNAL cb_bool
cb_copy_directory(const char* source_dir, const char* target_dir)
{
	char dest_buffer[CB_MAX_PATH];
	memset(dest_buffer, 0, sizeof(dest_buffer));

	cb_file_it it;
	cb_file_it_init_recursive(&it, source_dir);

	while (cb_file_it_get_next(&it))
	{
		/* copy current directory*/
		const char* source_relative_path = it.current_file + it.dir_len_stack[0];

		int n = snprintf(dest_buffer, CB_MAX_PATH, "%s", target_dir);
		n += cb_add_trailing_dir_separator(dest_buffer, n) ? 1 : 0;
		n += snprintf(dest_buffer + n, CB_MAX_PATH, "%s", source_relative_path);

		cb_copy_file(it.current_file, dest_buffer);
	}
	return cb_true;
}

/* recursively copy the content of the directory in another one, empty directory will be omitted */
CB_INTERNAL cb_bool
cb_move_files(const char* source_dir, const char* target_dir, cb_bool(*can_move)(cb_strv path))
{
	char dest_buffer[CB_MAX_PATH];
	memset(dest_buffer, 0, sizeof(dest_buffer));

	cb_file_it it;
	cb_file_it_init(&it, source_dir);

	while (cb_file_it_get_next(&it))
	{
		/* copy current directory */
		const char* source_relative_path = it.current_file + it.dir_len_stack[0];

		int n = snprintf(dest_buffer, CB_MAX_PATH, "%s", target_dir);
		n += cb_add_trailing_dir_separator(dest_buffer, n) ? 1 : 0;
		n += snprintf(dest_buffer + n, CB_MAX_PATH, "%s", source_relative_path);

		cb_strv p = cb_strv_make(dest_buffer, n);

		cb_bool should_move = !can_move || (can_move && can_move(p));
		if (should_move)
		{
			cb_move_file(it.current_file, dest_buffer);
		}
	}
	return cb_true;
}

#endif /* CB_FILE_OP_H */
