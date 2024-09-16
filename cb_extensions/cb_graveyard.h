
#define cb_copy_many_v(dest, src, ...) \
	cb_copy_many(dest, src \
    , (const char* []) { __VA_ARGS__ } \
	, (sizeof((const char* []) { __VA_ARGS__ }) / sizeof(const char*)))

CB_INTERNAL bool
cb_copy_many(const char* dest_directory, const char* source_directory, const char* source_suffixes[], int suffixes_count)
{
	cb_bool result = cb_false;
	CB_ASSERT(dest_directory && source_directory);

	cb_dstr source;
	cb_dstr_init(&source);
	cb_dstr_assign(&source, source_directory, strlen(source_directory));
	int source_prefix_len = source.size;

	cb_dstr dest;
	cb_dstr_init(&dest);
	cb_dstr_assign(&dest, dest_directory, strlen(dest_directory));
	int dest_prefix_len = dest.size;

	int i;

	for (i = 0; i < suffixes_count; ++i)
	{
		size_t suffix_len = strlen(source_suffixes[i]);
		cb_dstr_append_from(&source, source_prefix_len, source_suffixes[i], suffix_len);
		cb_dstr_append_from(&dest, dest_prefix_len, source_suffixes[i], suffix_len);

		if (cb_path_exists_str(source.data))
		{
			if (!cb_copy_file(source.data, dest.data))
			{
				result = cb_false;
				break;
			}
		}
	}
	
	cb_dstr_destroy(&dest);
	cb_dstr_destroy(&source);

	result = cb_true;
	return result;
}