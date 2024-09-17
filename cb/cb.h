#ifndef RE_CB_H
#define RE_CB_H

#define CB_IMPLEMENTATION

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h> /* va_start, va_end */

#if _WIN32
	#if !defined _CRT_SECURE_NO_WARNINGS
		#define _CRT_SECURE_NO_WARNINGS
	#endif
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>

	#define CB_THREAD  __declspec( thread )
#else
	#include <unistd.h>   /* open, close, access */
    #include <sys/stat.h> /* mkdir */
	#include <fcntl.h>    /* O_RDONLY etc. */
	#include <errno.h>
	#include <sys/sendfile.h> /* sendfile */
	#include <sys/wait.h>     /* waitpid */
	#include <dirent.h>       /* opendir */

	#define CB_THREAD __thread
#endif

/* in c89 va_copy does not exist */
#if defined(__GNUC__) || defined(__clang__)
#ifndef va_copy
#define va_copy(dest, src) (__builtin_va_copy(dest, src))
#endif
#endif

#ifndef CB_API
#define CB_API
#endif
#ifndef CB_INTERNAL
#define CB_INTERNAL static
#endif

#ifndef CB_ASSERT
#include <assert.h> /* assert */
#define CB_ASSERT assert
#endif

#ifndef CB_MALLOC
#include <stdlib.h> /* malloc */
#define CB_MALLOC malloc
#endif

#ifndef CB_FREE
#include <stdlib.h> /* free */
#define CB_FREE free
#endif

#define cb_true ((cb_bool)1)
#define cb_false ((cb_bool)0)

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int cb_id; /* hashed key */
typedef unsigned int cb_bool;

CB_API void cb_init();
CB_API void cb_destroy();

typedef struct cb_project_t cb_project_t;
/* Set or create current project.  */
CB_API cb_project_t* cb_project(const char* name); 

/* Add value for the specific key. */
CB_API void cb_add(const char* key, const char* value);
/* Wrapper of cb_set with string formatting */
CB_API void cb_add_f(const char* key, const char* fmt, ...);

/* Remove all previous values according to the key and set the new one. */
CB_API void cb_set(const char* key, const char* value);
/* Wrapper around cb_set with string formatting */
CB_API void cb_set_f(const char* key, const char* fmt, ...);

/* Remove all values associated with the key. Returns number of removed values */
CB_API int cb_remove_all(const char* key);
/* Wrapper around cb_remove_all with string formatting */
CB_API int cb_remove_all_f(const char* key, const char* fmt, ...);

/* Remove item with the exact key and value. */
CB_API cb_bool cb_remove_one(const char* key, const char* value);
/* Wrapper around cb_remove_one with string formatting */
CB_API cb_bool cb_remove_one_f(const char* key, const char* fmt, ...);

CB_API void cb_add_file(const char* filepath);

typedef struct cb_toolchain cb_toolchain;
/* Returns the name of the result, which could be the path of a library or any other value depending on the toolchain */
typedef const char* (*cb_toolchain_bake_t)(cb_toolchain* tc, const char*);

typedef struct cb_toolchain cb_toolchain;
struct cb_toolchain {
	cb_toolchain_bake_t bake;
	/* Name of the toolchain, mostly for debugging purpose */
	const char* name;
	/* Name of the default directory, @FIXME we might be able to get rid of this. */
	const char* default_directory_base;
};

/* Process the current project using the default toolchain.
   Returns the name of the result, which could be the path of a library or any other value depending on the toolchain.
   Use the default toolchain (gcc or msvc).
*/
CB_API const char* cb_bake(const char* project_name);

/* Same as cb_bake. Take an explicit toolchain instead of using the default one. */
CB_API const char* cb_bake_with(cb_toolchain toolchain, const char* project_name);

/* Same as cb_bake, additionaly run the resulting executable (if applicable).
   Returns NULL if project could not be baked or if the executable could not be run.
*/
CB_API const char* cb_bake_and_run(const char* project_name);

/* Same as cb_bake_with, additionaly run the resulting executable (if applicable).
   Returns NULL if project could not be baked or if the executable could not be run.
*/
CB_API const char* cb_bake_and_run_with(cb_toolchain toolchain, const char* project_name);

CB_API cb_toolchain cb_toolchain_default();

CB_API cb_bool cb_subprocess(const char* cmd);
CB_API cb_bool cb_subprocess_with_starting_directory(const char* cmd, const char* starting_directory);

/* commonly used properties (basically to make it discoverable with auto completion and avoid misspelling) */

/* keys */
extern const char* cbk_BINARY_TYPE;       /* Exe, shared_lib or static_lib. */
extern const char* cbk_CXFLAGS;           /* Extra flags to give to the C/C++ compiler. */
extern const char* cbk_DEFINES;           /* Define preprocessing symbol. */
extern const char* cbk_FILES;             /* Files to consume (could be .c, .cpp, etc.). */
extern const char* cbk_INCLUDE_DIR;       /* Include directories. */
extern const char* cbk_LINK_PROJECT;      /* Other project to link. */
extern const char* cbk_LFLAGS;            /* Extra flags to give to the linker. */
extern const char* cbk_OUTPUT_DIR;        /* Ouput directory for the generated files. */
extern const char* cbk_TARGET_NAME;       /* Name (basename) of the main generated file (.exe, .a, .lib, .dll, etc.). */
extern const char* cbk_WORKING_DIRECTORY; /* Working directory for a specific project. */
/* values */
extern const char* cbk_exe;
extern const char* cbk_shared_lib;
extern const char* cbk_static_lib;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* RE_CB_H */

#ifdef CB_IMPLEMENTATION
#ifndef CB_IMPLEMENTATION_CPP
#define CB_IMPLEMENTATION_CPP

#ifdef _WIN32
#define CB_DEFAULT_DIR_SEPARATOR_CHAR '\\'
#define CB_DEFAULT_DIR_SEPARATOR "\\"
#else
#define CB_DEFAULT_DIR_SEPARATOR_CHAR '/'
#define CB_DEFAULT_DIR_SEPARATOR "/"
#endif

#define CB_PREFERRED_DIR_SEPARATOR_CHAR '/'
#define CB_PREFERRED_DIR_SEPARATOR "/"
/* Table of content
*  ----------------------------------------------------------------
* 
* # Structures
*   - @TODO
* # Functions of internal structures
*   - cb_log     - write some logs
*   - cb_darr    - dynamic array
*   - cb_dstr    - dynamic string
*   - cb_kv      - key value for the multimap
*   - cb_mmap    - multimap containing key/value strings
* # Functions of the cb library
*   - cb_project(...) - @TODO explanation
*   - cb_set(...)     - @TODO explanation
*   - cb_add(...)     - @TODO explanation
*   - cb_bake(...)    - @TODO explanation
* # Toolchain
*   - msvc
*   - gcc
*/

/* keys */
const char* cbk_BINARY_TYPE = "binary_type";
const char* cbk_CXFLAGS = "cxflags";
const char* cbk_DEFINES = "defines";
const char* cbk_FILES = "files";
const char* cbk_INCLUDE_DIR = "include_dir";
const char* cbk_LINK_PROJECT = "link_project";
const char* cbk_LFLAGS = "lflags";
const char* cbk_OUTPUT_DIR = "output_dir";
const char* cbk_TARGET_NAME = "target_name";
const char* cbk_WORKING_DIRECTORY = "working_directory";
/* values */
const char* cbk_exe = "exe";
const char* cbk_shared_lib = "shared_library";
const char* cbk_static_lib = "static_library";

/* string view */
typedef struct cb_strv cb_strv;
struct cb_strv {
	int size;
	const char* data;
};

/* dynamic array
 * NOTE: cb_darr needs to start with the same component as cb_strv
 * because we need to compare them in the same way in cb_kv 
 */
typedef struct cb_darr cb_darr;
struct cb_darr {
	int size;
    char* data;
	int capacity;
};

/* type safe dynamic array */
#define cb_darrT(type)    \
    union {               \
        cb_darr base;     \
        struct  {         \
            int size;     \
			type* data;   \
            int capacity; \
        } darr;           \
    }

#define cb_darr_push_back(a, value) \
    do { \
        cb_darr* array_ = a; \
        void* value_ref_ = &(value); \
        cb_darr_insert_one(array_, array_->size, value_ref_, sizeof(value)); \
    } while (0)

#define cb_darrT_init(a) \
    cb_darr_init(&(a)->base)

#define cb_darrT_destroy(a) \
    cb_darr_destroy(&(a)->base)

#define cb_darrT_insert(a, index, value) \
    do {  \
        cb_darr_insert_one_space(&(a)->base, (index), sizeof(*(a)->darr.data)); \
		(a)->darr.data[(index)] = value; \
	} while (0)

#define cb_darrT_remove(a, index) \
    do {  \
        cb_darr_remove_one(&(a)->base, (index), sizeof(*(a)->darr.data)); \
	} while (0)

#define cb_darrT_push_back(a, value) \
    do {  \
        int last__ = (a)->darr.size; \
        cb_darr_insert_one_space(&(a)->base, last__, sizeof(*(a)->darr.data)); \
		(a)->darr.data[last__] = value; \
	} while (0)

#define cb_darrT_at(a, index) \
    ((a)->darr.data[index])

#define cb_darrT_set(a, index, value) \
    (a)->darr.data[index] = (value)

#define cb_darrT_ptr(a, index) \
    (&(a)->darr.data[index])

#define cb_darrT_size(a) \
    ((a)->darr.size)

/* dynamic string */
typedef cb_darr cb_dstr;

typedef char* cb_darr_it;

/* key/value data used in the map and mmap struct */
typedef struct cb_kv cb_kv;
struct cb_kv
{
	cb_id hash; /* hash of the key */
	cb_strv key; /* key */
	cb_bool is_dynamic_string;
	union {
		int _int;
		float _float;
		const void* ptr;
		cb_strv strv;
		cb_dstr dstr;
	} u; /* value */
};

/* multimap */
typedef cb_darrT(cb_kv) cb_mmap;

#define cb_rangeT(type) \
struct {                \
	type* begin;        \
	type* end;          \
	int count;          \
}

typedef cb_rangeT(cb_kv) cb_kv_range;

typedef struct cb_context cb_context; /* forward declaration */

typedef struct cb_project_t cb_project_t;
struct cb_project_t {
	cb_context* context;
	cb_id id;
	cb_strv name;

	cb_mmap mmap; /* multi map of strings - when you want to have multiple values per key */
};

typedef struct cb_context cb_context;
/* context, the root which hold everything */
struct cb_context {
	cb_mmap projects;
	cb_project_t* current_project;
	cb_darr string_pool; /* to allocate user strings, allow easy allocation of concatenated strings with cb_str */
};

static cb_context default_ctx;
static cb_context* current_ctx;

/*-----------------------------------------------------------------------*/
/* cb_log */
/*-----------------------------------------------------------------------*/

CB_INTERNAL void
cb_log(FILE* file, const char* prefix, const char* fmt, ...)
{
	fprintf(file, "%s", prefix);

	va_list args;
	va_start(args, fmt);
	vfprintf(file, fmt, args);
	va_end(args);
	fprintf(file, "\n");
}

/* NOTE: Those following macros are compatible with c99 only */
#define cb_log_error(fmt, ...)     cb_log(stderr, "[CB-ERROR] ", fmt, ##__VA_ARGS__)
#define cb_log_warning(fmt, ...)   cb_log(stderr, "[CB-WARNING] ", fmt, ##__VA_ARGS__)
#define cb_log_debug(fmt, ...)     cb_log(stdout, "[CB-DEBUG] ", fmt,  ##__VA_ARGS__)
#define cb_log_important(fmt, ...) cb_log(stdout, "", fmt,  ##__VA_ARGS__)

/*-----------------------------------------------------------------------*/
/* temporary allocation */
/*-----------------------------------------------------------------------*/

#ifndef CB_TMP_CAPACITY
#define CB_TMP_CAPACITY (8 * 1024 * 1024) /* Arbitrary size. Must be big enough. Can be increased if needed. */
#endif

static CB_THREAD int cb_tmp_size = 0;
static CB_THREAD char cb_tmp_buffer[CB_TMP_CAPACITY] = { 0 };

CB_INTERNAL void*
cb_tmp_alloc(int size)
{
	CB_ASSERT((cb_tmp_size + size <= CB_TMP_CAPACITY) && "Size of the temporary allocator is too small. Increase it if needed.");

	if (cb_tmp_size + size > CB_TMP_CAPACITY)
	{
		return NULL;
	}

	void* data = &cb_tmp_buffer[cb_tmp_size];
	cb_tmp_size += size;
	return data;
}

CB_INTERNAL void
cb_tmp_reset(void)
{
	cb_tmp_size = 0;
}

CB_INTERNAL char*
cb_tmp_vsprintf(const char* format, va_list args)
{
	va_list args_copy;
	va_copy(args_copy, args);

	int n = vsnprintf(NULL, 0, format, args);
	CB_ASSERT(n >= 0);

	char* data = cb_tmp_alloc(n + 1);

	vsnprintf(data, n + 1, format, args_copy);

	return data;
}

CB_INTERNAL char*
cb_tmp_sprintf(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char* data = cb_tmp_vsprintf(format, args);
	va_end(args);
	return data;
}

CB_INTERNAL const char*
cb_tmp_strv_to_str(cb_strv sv)
{
	char* data = cb_tmp_alloc(sv.size + 1);
	memcpy(data, sv.data, sv.size);
	data[sv.size] = '\0';
	return data;
}

CB_INTERNAL int
cb_tmp_save()
{
	return cb_tmp_size;
}

CB_INTERNAL void
cb_tmp_restore(int index)
{
	cb_tmp_size = index;
}

/*-----------------------------------------------------------------------*/
/* cb_darr - dynamic array */
/*-----------------------------------------------------------------------*/

CB_INTERNAL void
cb_darr_init(cb_darr* arr)
{
	arr->size = 0;
	arr->capacity = 0;
	arr->data = NULL;
}

CB_INTERNAL void
cb_darr_destroy(cb_darr* arr)
{
	if (arr->data != NULL)
	{
		arr->size = 0;
		arr->capacity = 0;
		CB_FREE(arr->data);
		arr->data = NULL;
	} 
}

CB_INTERNAL void*
cb_darr_ptr(const cb_darr* arr, int index, int sizeof_vlaue)
{
	CB_ASSERT(index >= 0);
	CB_ASSERT(
		index < arr->size /* Within accessible item range */
		|| index == arr->size /* Also allow getting the item at the end */
	);

	return arr->data + (index * sizeof_vlaue);
}

CB_INTERNAL char* cb_darr_end(const cb_darr* arr, int sizeof_value) { return arr->data + (arr->size * sizeof_value); }

CB_INTERNAL int
cb_darr__get_new_capacity(const cb_darr* arr, int sz)
{
	int new_capacity = arr->capacity ? (arr->capacity + arr->capacity / 2) : 8;
	return new_capacity > sz ? new_capacity : sz;
}

CB_INTERNAL void
cb_darr_reserve(cb_darr* arr, int new_capacity, int sizeof_value)
{
	if (new_capacity <= arr->capacity)
	{
		return;
	}

	char* new_data = (char*)CB_MALLOC((size_t)new_capacity * sizeof_value);
	CB_ASSERT(new_data);
	if (arr->data != NULL) {
		memcpy(new_data, arr->data, (size_t)arr->size * sizeof_value);
		CB_FREE(arr->data);
	}
	arr->data = new_data;
	arr->capacity = new_capacity;
}

CB_INTERNAL void
cb_darr__grow_if_needed(cb_darr* arr, int needed, int sizeof_value)
{
	if (needed > arr->capacity)
		cb_darr_reserve(arr, cb_darr__get_new_capacity(arr, needed), sizeof_value);
}

CB_INTERNAL void
cb_darr_insert_many_space(cb_darr* arr, int index, int count, int sizeof_value)
{
	int count_to_move = arr->size - index;

	CB_ASSERT(arr != NULL);
	CB_ASSERT(count > 0);
	CB_ASSERT(index >= 0);
	CB_ASSERT(index <= arr->size);

	cb_darr__grow_if_needed(arr, arr->size + count, sizeof_value);

	if (count_to_move > 0)
	{
		memmove(
			cb_darr_ptr(arr, index + count, sizeof_value),
			cb_darr_ptr(arr, index, sizeof_value),
			count_to_move * sizeof_value);
	}

	arr->size += count;
}

CB_INTERNAL void
cb_darr_insert_one_space(cb_darr* arr, int index, int sizeof_value)
{
	cb_darr_insert_many_space(arr, index, 1, sizeof_value);
}

CB_INTERNAL void
cb_darr_insert_many(cb_darr* arr, int index, const void* value, int count, int sizeof_value)
{
	cb_darr_insert_many_space(arr, index, count, sizeof_value);

	memcpy(cb_darr_ptr(arr, index, sizeof_value), value, count * sizeof_value);
}

CB_INTERNAL void
cb_darr_insert_one(cb_darr* arr, int index, const void* value, int sizeof_value)
{
	cb_darr_insert_many(arr, index, value, 1, sizeof_value);
}

CB_INTERNAL void
cb_darr_push_back_many(cb_darr* arr, const void* values_ptr, int count, int sizeof_value)
{
	cb_darr_insert_many(arr, arr->size, values_ptr, count, sizeof_value);
}

CB_INTERNAL void
cb_darr_remove_many(cb_darr* arr, int index, int count, int sizeof_value)
{
	CB_ASSERT(arr);
	CB_ASSERT(index >= 0);
	CB_ASSERT(count >= 0);
	CB_ASSERT(index < arr->size);
	CB_ASSERT(count <= arr->size);
	CB_ASSERT(index + count <= arr->size);

	if (count <= 0)
		return;

	memmove(
		cb_darr_ptr(arr, index, sizeof_value),
		cb_darr_ptr(arr, index + count, sizeof_value),
		(arr->size - (index + count)) * sizeof_value
	);

	arr->size -= count;
}

CB_INTERNAL void
cb_darr_remove_one(cb_darr* arr, int index, int sizeof_value)
{
	cb_darr_remove_many(arr, index, 1, sizeof_value);
}

typedef cb_bool(*cb_predicate_t)(const void* left, const void* right);

CB_INTERNAL int
cb_lower_bound_predicate(const void* void_ptr, int left, int right, const void* value, int sizeof_value, cb_predicate_t pred)
{
	const char* ptr = (const char*)void_ptr;
	int count = right - left;
	int step;
	int mid; /* index of the found value */

	while (count > 0) {
		step = count >> 1; /* count divide by two using bit shift */

		mid = left + step;

		if (pred(ptr + (mid * sizeof_value), value)) {
			left = mid + 1;
			count -= step + 1;
		}
		else {
			count = step;
		}
	}
	return left;
}

/* @FIXME maybe we can directly create an overload for cb_mmap, not sure we want to use raw array with lower_bound */
CB_INTERNAL int
cb_darr_lower_bound_predicate(const cb_darr* arr, const void* value, int sizeof_value, cb_predicate_t less)
{
	return cb_lower_bound_predicate(arr->data, 0, arr->size, value, sizeof_value, less);
}

/*-----------------------------------------------------------------------*/
/* cb_strv - string view */
/*-----------------------------------------------------------------------*/

CB_INTERNAL cb_strv
cb_strv_make(const char* data, int size)
{
	cb_strv s;
	s.data = data;
	s.size = size; 
	return s;
}

CB_INTERNAL cb_strv
cb_strv_make_str(const char* str)
{
	return cb_strv_make(str, strlen(str));
}

CB_INTERNAL int
cb_lexicagraphical_cmp(const char* left, size_t left_count, const char* right, size_t right_count)
{
	char c1, c2;
	size_t min_size = left_count < right_count ? left_count : right_count;
	while (min_size-- > 0)
	{
		c1 = (unsigned char)*left++;
		c2 = (unsigned char)*right++;
		if (c1 != c2)
			return c1 < c2 ? 1 : -1;
	};

	return left_count - right_count;
}

CB_INTERNAL int cb_strv_compare(cb_strv sv, const char* data, int size) { return cb_lexicagraphical_cmp(sv.data, sv.size, data, size); }
CB_INTERNAL int cb_strv_compare_strv(cb_strv sv, cb_strv other) { return cb_strv_compare(sv, other.data, other.size); }
CB_INTERNAL int cb_strv_compare_str(cb_strv sv, const char* str) { return cb_strv_compare(sv, str, strlen(str)); }
CB_INTERNAL cb_bool cb_strv_equals(cb_strv sv, const char* data, int size) { return cb_strv_compare(sv, data, size) == 0; }
CB_INTERNAL cb_bool cb_strv_equals_strv(cb_strv sv, cb_strv other) { return cb_strv_compare_strv(sv, other) == 0; }
CB_INTERNAL cb_bool cb_strv_equals_str(cb_strv sv, const char* other) { return cb_strv_compare_strv(sv, cb_strv_make_str(other)) == 0; }

/*-----------------------------------------------------------------------*/
/* cb_str - c string utilities */
/*-----------------------------------------------------------------------*/

CB_INTERNAL cb_bool cb_str_equals(const char* left, const char* right) { return cb_strv_equals_strv(cb_strv_make_str(left), cb_strv_make_str(right)); }

/*-----------------------------------------------------------------------*/
/* cb_dstr - dynamic string */
/*-----------------------------------------------------------------------*/

#define cb_dstr_append_v(dstr, ...) \
	cb_dstr_append_many(dstr \
    , (const char* []) { __VA_ARGS__ } \
	, (sizeof((const char* []) { __VA_ARGS__ }) / sizeof(const char*)))

CB_INTERNAL const char* cb_empty_string() { return "\0EMPTY_STRING"; }
CB_INTERNAL void cb_dstr_init(cb_dstr* dstr) { cb_darr_init(dstr); dstr->data = (char*)cb_empty_string(); }
CB_INTERNAL void cb_dstr_destroy(cb_dstr* dstr) { if (dstr->data == cb_empty_string()) { dstr->data = NULL; } cb_darr_destroy(dstr); }
/* does not free anything, just reset the size to 0 */
CB_INTERNAL void cb_dstr_clear(cb_dstr* dstr) { if (dstr->data != cb_empty_string()) { dstr->size = 0; dstr->data[dstr->size] = '\0'; } }

CB_INTERNAL void
cb_dstr_reserve(cb_dstr* s, int new_string_capacity)
{
	assert(new_string_capacity > s->capacity && "You should request more capacity, not less."); /* ideally we should ensure this before this call. */
	
	if (new_string_capacity <= s->capacity)
		return; 

	int new_mem_capacity = new_string_capacity + 1;
	char* new_data = (char*)CB_MALLOC((size_t)new_mem_capacity * sizeof(char));
	if (s->size)
	{
		int size_plus_null_term = s->size + 1;
		memcpy(new_data, s->data, (size_t)size_plus_null_term * sizeof(char));
		CB_FREE(s->data);
	}
	s->data = new_data;
	s->capacity = new_string_capacity;
}

CB_INTERNAL void
cb_dstr__grow_if_needed(cb_dstr* s, int needed)
{
	if (needed > s->capacity) { 
		cb_dstr_reserve(s, cb_darr__get_new_capacity(s, needed));
	}
}

CB_INTERNAL void
cb_dstr_append_from(cb_dstr* s, int index, const char* data, int size)
{
	cb_dstr__grow_if_needed(s, index + size);

	memcpy(s->data + index, (const void*)data, ((size) * sizeof(char)));
	s->size = index + size;
	s->data[s->size] = '\0';
}

CB_INTERNAL int
cb_dstr_append_from_fv(cb_dstr* s, int index, const char* fmt, va_list args)
{
	va_list args_copy;
	va_copy(args_copy, args);

	/* Caluclate necessary len */
	int add_len = vsnprintf(NULL, 0, fmt, args_copy);
	CB_ASSERT(add_len >= 0);

	cb_dstr__grow_if_needed(s, s->size + add_len);

	add_len = vsnprintf(s->data + index, add_len + 1, fmt, args);

	s->size = index + add_len;
	return add_len;
}

CB_INTERNAL void
cb_dstr_assign(cb_dstr* s, const char* data, int size)
{
	cb_dstr_append_from(s, 0, data, size);
}

CB_INTERNAL void
cb_dstr_assign_str(cb_dstr* s, const char* str)
{
	cb_dstr_assign(s, str, strlen(str));
}

CB_INTERNAL void
cb_dstr_assign_f(cb_dstr* s, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	cb_dstr_append_from_fv(s, 0, fmt, args);
	va_end(args);
}

CB_INTERNAL void
cb_dstr_append_many(cb_dstr* s, const char* strings[], int count)
{
	int i;

	for (i = 0; i < count; ++i)
	{
		cb_dstr_append_from(s, s->size, strings[i], strlen(strings[i]));
	}
}

CB_INTERNAL void
cb_dstr_append_str(cb_dstr* s, const char* str)
{
	cb_dstr_append_from(s, s->size, str, strlen(str));
}

CB_INTERNAL void
cb_dstr_append_strv(cb_dstr* s, cb_strv sv)
{
	cb_dstr_append_from(s, s->size, sv.data, sv.size);
}

CB_INTERNAL int
cb_dstr_append_f(cb_dstr* s, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int len = cb_dstr_append_from_fv(s, s->size, fmt, args);
	va_end(args);
	return len;
}

/*-----------------------------------------------------------------------*/
/* cb_hash */
/*-----------------------------------------------------------------------*/

CB_INTERNAL unsigned long
djb2_strv(const char* str, int count)
{
	unsigned long hash = 5381;
	int i = 0;
	while (i < count)
	{
		hash = ((hash << 5) + hash) + str[i]; /* hash * 33 + c */
		i++;
	}

	return hash;
}

CB_INTERNAL cb_id
cb_hash_strv(cb_strv sv)
{
	return djb2_strv(sv.data, sv.size);
}

/*-----------------------------------------------------------------------*/
/* cb_kv */
/*-----------------------------------------------------------------------*/

CB_INTERNAL void
cb_kv_init(cb_kv* kv, cb_strv sv)
{
	memset(kv, 0, sizeof(cb_kv));
	kv->hash = cb_hash_strv(sv);
	kv->key = sv;
}

CB_INTERNAL cb_kv
cb_kv_make_with_str(cb_strv sv, const char* value)
{
	cb_kv kv;
	cb_kv_init(&kv, sv);
	kv.is_dynamic_string = 0;
	kv.u.strv = cb_strv_make_str(value);
	return kv;
}

CB_INTERNAL cb_kv
cb_kv_make_with_ptr(cb_strv sv, const void* ptr)
{
	cb_kv kv;
	cb_kv_init(&kv, sv);
	kv.is_dynamic_string = 0;
	kv.u.ptr = ptr;
	return kv;
}

CB_INTERNAL cb_kv
cb_kv_make_with_dstr(cb_strv sv, cb_dstr value)
{
	cb_kv kv;
	cb_kv_init(&kv, sv);
	kv.is_dynamic_string = 1;
	kv.u.dstr = value;
	return kv;
}

CB_INTERNAL int
cb_kv_comp(const cb_kv* left, const cb_kv* right)
{
	return left->hash != right->hash
		? (right->hash < left->hash ? -1 : 1)
		: cb_strv_compare_strv(left->key, right->key);
}

CB_INTERNAL cb_bool
cb_kv_less(const cb_kv* left, const cb_kv* right)
{
	return cb_kv_comp(left, right) < 0;
}

/*-----------------------------------------------------------------------*/
/* cb_mmap - a multimap */
/*-----------------------------------------------------------------------*/

CB_INTERNAL void cb_mmap_init(cb_mmap* m) { cb_darrT_init(m); }
CB_INTERNAL void cb_mmap_destroy(cb_mmap* m) { cb_darrT_destroy(m); }

CB_INTERNAL void
cb_mmap_insert(cb_mmap* m, cb_kv kv)
{
	int index = cb_darr_lower_bound_predicate(&m->base, &kv, sizeof(cb_kv), (cb_predicate_t)cb_kv_less);

	cb_darrT_insert(m, index, kv);
}

/* cb_mmap_find does the same as cb_map_find, we should probably remove cb_map implementation if it's not used anymore */
CB_INTERNAL int
cb_mmap_find(const cb_mmap* m, const cb_kv* kv)
{
	int index = cb_darr_lower_bound_predicate(&m->base, kv, sizeof(cb_kv), (cb_predicate_t)cb_kv_less);

	if (index == m->base.size || cb_kv_less(kv, cb_darrT_ptr(m, index)))
	{
		index = m->base.size; /* not found */
	}

	return index;
}

CB_INTERNAL cb_bool
cb_mmap_try_get_first(const cb_mmap* m, cb_strv key, cb_kv* kv)
{
	cb_kv key_item = cb_kv_make_with_str(key, "");
	int index = cb_mmap_find(m, &key_item);

	if (index != m->darr.size) /* found */
	{
		*kv = cb_darrT_at(m, index);
		return cb_true;
	}

	return cb_false;
}

CB_INTERNAL cb_kv_range
cb_mmap_get_range(const cb_mmap* m, cb_strv key)
{
	cb_kv_range result = { 0, 0, 0 };

	cb_kv key_item = cb_kv_make_with_str(key, "");

	int index = cb_mmap_find(m, &key_item);

	if (index == m->darr.size)
	{
		return result;
	}

	/* An item has been found */

	result.begin = cb_darrT_ptr(m, index);

	result.count++;

	/* Check for other items */
	while (index != m->base.size
		&& cb_kv_comp(cb_darrT_ptr(m, index), &key_item) == 0)
	{
		index += 1;

		result.count++;
	}

	result.end = cb_darrT_ptr(m, index);
	return result;
}

CB_INTERNAL cb_kv_range
cb_mmap_get_range_str(const cb_mmap* m, const char* key)
{
	return cb_mmap_get_range(m, cb_strv_make_str(key));
}

CB_INTERNAL cb_bool
cb_mmap_range_get_next(cb_kv_range* range, cb_kv* next)
{
	CB_ASSERT(range->begin <= range->end);

	if (range->begin < range->end)
	{
		*next = *range->begin;

		range->begin += 1;
		return cb_true;
	}

	return cb_false;
}

CB_INTERNAL void
cb_mmap_remove_one(cb_mmap* m, int index)
{
	/* destroy dynamic string if needed */
	{
		cb_kv* kv = cb_darrT_ptr(m, index);
		if (kv->is_dynamic_string)
		{
			cb_dstr_destroy(&kv->u.dstr);
		}
	}
	cb_darrT_remove(m, index);
}

/* Remove all values found in keys, if the value was a dynamic string the dynamic string is destroyed */
CB_INTERNAL int
cb_mmap_remove(cb_mmap* m, cb_kv kv)
{
	cb_kv_range range = cb_mmap_get_range(m, kv.key);

	int count_to_remove = range.count;
	int current_index = range.begin - m->darr.data;

	if (range.count)
	{
		while (current_index < count_to_remove)
		{
			cb_mmap_remove_one(m, current_index);
			current_index += 1;
		}
	}
	return count_to_remove;
}


CB_INTERNAL cb_bool
cb_mmap_get_from_kv(cb_mmap* map, const cb_kv* item, cb_kv* result)
{
	int index = cb_mmap_find(map, item);
	if (index != map->base.size)
	{
		*result = cb_darrT_at(map, index);
		return cb_true;
	}

	return cb_false;
}

CB_INTERNAL void
cb_mmap_insert_ptr(cb_mmap* map, cb_strv key, const void* value_ptr)
{
	cb_mmap_insert(map, cb_kv_make_with_ptr(key, value_ptr));
}

CB_INTERNAL const void*
cb_mmap_get_ptr(cb_mmap* map, cb_strv key, const void* default_value)
{
	cb_kv key_item;
	cb_kv_init(&key_item, key);
	cb_kv result;

	return cb_mmap_get_from_kv(map, &key_item, &result) ? result.u.ptr : default_value;
}

CB_INTERNAL cb_strv
cb_mmap_get_strv(cb_mmap* map, cb_strv key, cb_strv default_value)
{
	cb_kv key_item;
	cb_kv_init(&key_item, key);
	cb_kv result;

	return cb_mmap_get_from_kv(map, &key_item, &result) ? result.u.strv : default_value;
}

CB_INTERNAL void
cb_context_init(cb_context* ctx)
{
	memset(ctx, 0, sizeof(cb_context));
	cb_mmap_init(&ctx->projects);
	ctx->current_project = 0;
}

CB_INTERNAL void
cb_context_destroy(cb_context* ctx)
{
	cb_mmap_destroy(&ctx->projects);
	cb_context_init(ctx);
}

CB_INTERNAL cb_bool
cb_try_find_project_by_name(cb_strv sv, cb_project_t** project)
{
	void* default_value = NULL;
	*project = (cb_project_t*)cb_mmap_get_ptr(&current_ctx->projects, sv, default_value);
	return (cb_bool)(*project != NULL);
}

CB_INTERNAL cb_project_t*
cb_find_project_by_name(cb_strv sv)
{
	cb_project_t* project = 0;
	if (!cb_try_find_project_by_name(sv, &project))
	{
		cb_log_error("Project not found '%.*s'", sv.size, sv.data);
		return NULL;
	}

	return project;
}

CB_INTERNAL cb_project_t* cb_find_project_by_name_str(const char* name) { return cb_find_project_by_name(cb_strv_make_str(name)); }
CB_INTERNAL cb_bool cb_try_find_project_by_name_str(const char* name, cb_project_t** project) { return cb_try_find_project_by_name(cb_strv_make_str(name), project); }

CB_INTERNAL void
cb_project_init(cb_project_t* project)
{
	memset(project, 0, sizeof(cb_project_t));

	cb_mmap_init(&project->mmap);
}

CB_INTERNAL void
cb_project_destroy(cb_project_t* project)
{
	cb_mmap_destroy(&project->mmap);
}

CB_INTERNAL cb_project_t*
cb_create_project(const char* name)
{
	cb_strv n = cb_strv_make_str(name);
    cb_id id = cb_hash_strv(n);
	
	cb_project_t* project = (cb_project_t*)CB_MALLOC(sizeof(cb_project_t));
	cb_project_init(project);
	project->context = current_ctx;
	project->id = id;
	project->name = n;
	
	cb_mmap_insert_ptr(&current_ctx->projects, n, project);
	
    return project;
}

CB_INTERNAL cb_project_t*
cb__current_project()
{
	CB_ASSERT(current_ctx->current_project);
	cb_project_t* p = current_ctx->current_project;
	CB_ASSERT(p);
	return p;
};

/* API */

#ifdef _WIN32
/* Any error would silently crash any application, this handler is just there to display a message and exit the application with a specific value */
__declspec(noinline) static LONG WINAPI exit_on_exception_handler(EXCEPTION_POINTERS* ex_ptr)
{
	(void)ex_ptr;
	int exit_code=1;
	printf("[CB] Error: unexpected error. exited with code %d\n", exit_code);
	exit(exit_code);
}

/* Convert utf-8 string to utf-16 string. String is allocated using the temp buffer */
CB_INTERNAL wchar_t*
cb_utf8_to_utf16(const char* str)
{
	/* Calculate len */
	int length = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	/* Allocate enough space */
	wchar_t* data = (wchar_t*)cb_tmp_alloc(length * sizeof(wchar_t));
	/* Write data */
	MultiByteToWideChar(CP_UTF8, 0, str, -1, data, length);
	/* Place null-terminated char to end the string */
	data[length] = L'\0';
	return data;
}

#endif

/* #file utils */

CB_INTERNAL cb_bool cb_is_directory_separator(char c) { return (c == '/' || c == '\\'); }

#define CB_NPOS (-1)

CB_INTERNAL int
cb_rfind(cb_strv s, char c)
{
	if (s.size == 0) return CB_NPOS;

	const char* begin = s.data;
	const char* end = s.data + s.size - 1;
	while (end >= begin && *end != c)
	{
		end--;
	}
	return end < begin ? CB_NPOS : (end - begin);
}

CB_INTERNAL int
cb_rfind2(cb_strv s, char c1, char c2)
{
	if (s.size == 0) return CB_NPOS;

	const char* begin = s.data;
	const char* end = s.data + s.size - 1;
	while (end >= begin && *end != c1 && *end != c2)
	{
		end--;
	}
	return end < begin ? CB_NPOS : (end - begin);
}

/* Get parent directory from a path */
CB_INTERNAL cb_strv
cb_path_directory(cb_strv path)
{
	int pos = cb_rfind2(path, '/', '\\');
	if (pos != CB_NPOS && pos > 0) {
		return cb_strv_make(path.data
			, pos && cb_is_directory_separator(path.data[pos -1]) ? pos  : pos + 1 /* Return the result with the trailing slash if there is one */
		);
	}
	return path;
}

CB_INTERNAL cb_strv
cb_path_directory_str(const char* str)
{
	return cb_path_directory(cb_strv_make_str(str));
}

CB_INTERNAL cb_strv
cb_path_filename(cb_strv path)
{
	int pos = cb_rfind2(path, '/', '\\');
	if (pos != CB_NPOS && pos > 0) {
		return cb_strv_make(path.data + pos + 1 /* plus one because we don't want the slash char */
			, path.size - pos);
	}
	return path;
}
CB_INTERNAL cb_strv
cb_path_filename_str(const char* str)
{
	return cb_path_filename(cb_strv_make_str(str));
}

CB_INTERNAL cb_strv
cb_path_basename(cb_strv s)
{
	cb_strv filename = cb_path_filename(s);

	if (!cb_strv_equals_str(filename, ".")
		&& !cb_strv_equals_str(filename, "..")) {
		int pos = cb_rfind(filename, '.');
		if (pos != CB_NPOS && pos > 0) {
			return cb_strv_make(filename.data, pos);
		}
	}
	return filename;
}

CB_INTERNAL cb_strv
cb_path_basename_str(const char* str)
{
	return cb_path_basename(cb_strv_make_str(str));
}
/*  #file_iterator */

#define CB_MAX_PATH 1024 /* this is an arbitrary limit */

#define cb_safe_strcpy(dst, src, index, max) cb_safe_strcpy_internal(dst, src, index, max, __FILE__, __LINE__)

CB_INTERNAL int
cb_safe_strcpy_internal(char* dst, const char* src, int index, int max, const char* file, int line)
{
	char c;
	const char* original = src;

	do {
		if (index >= max) {
			cb_log_error("[String \"%s\" too long to copy on line %d in file %s (max length of %d).", original, line, file, max);
			CB_ASSERT(0);
			break;
		}

		c = *src++;
		dst[index] = c;
		++index;
	} while (c);

	return index;
}

/* return cb_true if separator has been added */
CB_INTERNAL cb_bool
cb_add_trailing_dir_separator(char* path, int path_len)
{
	if (!path || path_len >= CB_MAX_PATH) return cb_false;

	if (path_len)
	{
		path += path_len;
		if (path[-1] != '/' && path[-1] != '\\')
		{
			*path++ = CB_PREFERRED_DIR_SEPARATOR_CHAR;
			*path = '\0';
			return cb_true;
		}
	}
	return cb_false;
}

#ifdef WIN32

CB_INTERNAL cb_bool
cb_path__exists(const wchar_t* path)
{
	DWORD attr = GetFileAttributesW(path);
	return attr != INVALID_FILE_ATTRIBUTES;
}

CB_INTERNAL cb_bool
cb_path_exists(const char* path)
{
	return cb_path__exists(cb_utf8_to_utf16(path));
}

CB_INTERNAL cb_bool
cb__create_directory(const wchar_t* path)
{
	return CreateDirectoryW(path, NULL);
}

CB_INTERNAL cb_bool
cb_create_directory(const char* path)
{
	return cb__create_directory(cb_utf8_to_utf16(path));
}
#else

CB_INTERNAL cb_bool cb_path_exists(const char* path) { return access(path, F_OK) == 0; }
CB_INTERNAL cb_bool cb_path__exists(const char* path) { return cb_path_exists(path); }

CB_INTERNAL cb_bool cb_create_directory(const char* path) { return mkdir(path, 0777) == 0; }
CB_INTERNAL cb_bool cb__create_directory(const char* path) { return cb_create_directory(path); }

#endif

CB_INTERNAL cb_bool
cb_path_is_absolute(cb_strv path)
{
	int len = path.size;

	if (len == 0) return cb_false;

#ifdef _WIN32
	// Check drive C:
	int i = 0;
	if (isalpha(path.data[0]) && path.data[1] == ':')
	{
		i = 2;
	}

	return (path.data[i] == '/' || path.data[i] == '\\');
#else

	return (path.data[0] == '/');
#endif
}

#ifdef _WIN32
#define getcwd(buf, size) GetCurrentDirectoryA((DWORD)size, buf)
#else
#include <unistd.h> /* getcwd */
#endif

CB_INTERNAL cb_bool
cb_path_get_absolute(const char* path, cb_dstr* abs_path)
{
	if (!cb_path_is_absolute(cb_strv_make_str(path)))
	{
		/* skip ./ or .\ */
		if (path[0] == '.' && (path[1] == '\\' || path[1] == '/'))
			path += 2;

		char buffer[FILENAME_MAX];
		getcwd(buffer, FILENAME_MAX);
		if (buffer == NULL)
			return cb_false;

		cb_dstr_assign_str(abs_path, buffer);

		cb_dstr_append_str(abs_path, CB_PREFERRED_DIR_SEPARATOR);
	}

	cb_dstr_append_str(abs_path, path);

	int i = 2; /* start at 2 to dealing with volume name. */
	/* Replace slash with the preferred one. */
	while (i < abs_path->size)
	{
		if (abs_path->data[i] == CB_DEFAULT_DIR_SEPARATOR_CHAR)
		{
			abs_path->data[i] = CB_PREFERRED_DIR_SEPARATOR_CHAR;
		}
		i += 1;
	}

	return cb_true;
}

/* create directories recursively */
CB_INTERNAL void
cb_create_directories_core(const char* path, int size)
{
	if (path == NULL || size <= 0) {
		cb_log_error("Could not create directory. Path is empty.");
		return;
	}

	if (size > CB_MAX_PATH) {
		cb_log_error("Could not create directory. Path is too long '%s'.", path);
		return;
	}
#ifdef _WIN32
	typedef wchar_t tchar;
	wchar_t* str = (wchar_t*)cb_utf8_to_utf16(path);
#else
	typedef char tchar;
	char* str = (char*)path;
#endif

	if (!cb_path__exists(str))
	{
		tchar* cur = str + 2; /* + 2 to avoid root on Windows (unix would require +1 for the original slash ) */
		tchar* end = str + size;
		while (cur < end)
		{
			/* go to next directory separator */
			while (*cur && *cur != '\\' && *cur != '/')
				cur++;

			if (*cur)
			{
				*cur = '\0'; /* terminate path at separator */
				if (!cb_path__exists(str))
				{
					if (!cb__create_directory(str))
					{
						cb_log_error("Could not create directory.");
						return;
					}
				}
				*cur = CB_PREFERRED_DIR_SEPARATOR_CHAR; /* put the separator back */
			}
			cur++;
		}
	}
}

CB_INTERNAL void
cb_create_directories(const char* path, int size)
{
	int index = cb_tmp_save();
	cb_create_directories_core(path, size);
	cb_tmp_restore(index);
}

CB_INTERNAL cb_bool
cb_copy_file(const char* src_path, const char* dest_path)
{
	/* create target directory if it does not exists */
	cb_create_directories(dest_path, strlen(dest_path));
	cb_log_debug("Copying '%s' to '%s'", src_path, dest_path);
#ifdef _WIN32

	DWORD attr = GetFileAttributesA(src_path);

	if (attr == INVALID_FILE_ATTRIBUTES) {
		cb_log_error("Could not retieve file attributes of file '%s' (%d).", src_path, GetLastError());
		return cb_false;
	}

	cb_bool is_directory = attr & FILE_ATTRIBUTE_DIRECTORY;
	BOOL fail_if_exists = FALSE;
	if (!is_directory && !CopyFileA(src_path, dest_path, fail_if_exists)) {
		cb_log_error("Could not copy file '%s', %lu", src_path, GetLastError());
		return cb_false;
	}
	return cb_true;
#else

	int src_fd = -1;
	int dst_fd = -1;
	src_fd = open(src_path, O_RDONLY);
	if (src_fd < 0)
	{
		cb_log_error("Could not open file '%s': %s", src_path, strerror(errno));
		return cb_false;
	}
	
    struct stat src_stat;
    if (fstat(src_fd, &src_stat) < 0)
	{
        cb_log_error("Could not get fstat of file '%s': %s", src_path, strerror(errno));
		close(src_fd);
		return cb_false;
    }
	
	dst_fd = open(dest_path, O_CREAT | O_TRUNC | O_WRONLY, src_stat.st_mode);

	if (dst_fd < 0)
	{
        cb_log_error("Could not open file '%s': %s", dest_path, strerror(errno));
		close(src_fd);
		return cb_false;
	}
	
    int64_t total_bytes_copied = 0;
    int64_t bytes_left = src_stat.st_size;
    while (bytes_left > 0)
    {
		off_t sendfile_off = total_bytes_copied;
		int64_t send_result = sendfile(dst_fd, src_fd, &sendfile_off, bytes_left);
		if(send_result <= 0)
		{
			break;
		}
		int64_t bytes_copied = (int64_t)send_result;
		bytes_left -= bytes_copied;
		total_bytes_copied += bytes_copied;
    }
  
	close(src_fd);
	close(dst_fd);
	return bytes_left == 0;
#endif
}

CB_INTERNAL cb_bool
cb_try_copy_file_to_dir(const char* file, const char* directory)
{
	if (!cb_path_exists(file))
	{
		return cb_false;
	}

	int index = cb_tmp_save();

	cb_strv filename = cb_path_filename_str(file);
	const char* destination_file = cb_tmp_sprintf("%s%.*s", directory, filename.size, filename.data);

	cb_bool result = cb_copy_file(file, destination_file);

	cb_tmp_restore(index);

	return result;
}

CB_INTERNAL cb_bool
cb_copy_file_to_dir(const char* file, const char* directory)
{
	if (!cb_try_copy_file_to_dir(file, directory))
	{
		cb_log_error("Could not copy file '%s' to directory %s", file, directory);
		return cb_false;
	}
	return cb_true;
}


CB_INTERNAL cb_bool
cb_delete_file(const char* src_path)
{
	cb_bool result = 0;
	cb_log_debug("Deleting file '%s'.", src_path);
#ifdef _WIN32
	int index = cb_tmp_save();
	result = DeleteFileW(cb_utf8_to_utf16(src_path));
	cb_tmp_restore(index);
#else
	result = remove(src_path) != -1;
#endif
	if (!result)
		cb_log_debug("Could not delete file '%s'.", src_path);

	return result;
}

CB_INTERNAL cb_bool
cb_move_file(const char* src_path, const char* dest_path)
{
	if (cb_copy_file(src_path, dest_path))
	{
		return cb_delete_file(src_path);
	}

	return cb_false;
}

CB_INTERNAL cb_bool
cb_try_move_file_to_dir(const char* file, const char* directory)
{
	if (!cb_path_exists(file))
	{
		return cb_false;
	}

	int index = cb_tmp_save();

	cb_strv filename = cb_path_filename_str(file);
	const char* destination_file = cb_tmp_sprintf("%s%.*s", directory, filename.size, filename.data);

	cb_bool result = cb_move_file(file, destination_file);

	cb_tmp_restore(index);

	return result;
}

CB_INTERNAL cb_bool
cb_move_file_to_dir(const char* file, const char* directory)
{
	if (!cb_try_move_file_to_dir(file, directory))
	{
		cb_log_error("Could not move file '%s' to '%s'", file, directory);
		return cb_false;
	}
	return cb_true;
}


/* Get absolute path, ends with a directory separator if it's a directory. */
CB_INTERNAL void
cb_dstr_append_absolute_path(cb_dstr* s, const char* path)
{
	cb_dstr abs;
	cb_dstr_init(&abs);

	cb_path_get_absolute(path, &abs);
	cb_dstr_append_from(s, s->size, abs.data, abs.size);

	cb_dstr_destroy(&abs);
}

CB_INTERNAL void
cb_dstr_ensure_trailing_slash(cb_dstr* s)
{
	/* Add trailing slash if necessary */
	cb_dstr_append_str(s, cb_is_directory_separator(s->data[s->size - 1]) ? "" : CB_PREFERRED_DIR_SEPARATOR);
}

CB_INTERNAL void cb__add(cb_kv kv);
CB_INTERNAL void cb__set(cb_kv kv);
CB_INTERNAL int cb__remove_all(cb_kv kv);
CB_INTERNAL cb_bool cb__remove_one(cb_kv kv);

CB_INTERNAL void
cb__add(cb_kv kv)
{
	cb_project_t* p = cb__current_project();
	cb_mmap_insert(&p->mmap, kv);
}

CB_INTERNAL void
cb__set(cb_kv kv)
{
	/* @FIXME this can easily be optimized, but we don't care about that right now. */
	cb__remove_all(kv);
	cb__add(kv);
}

CB_INTERNAL cb_bool
cb__remove_one(cb_kv kv)
{
	cb_project_t* p = cb__current_project();

	cb_kv_range range = cb_mmap_get_range(&p->mmap, kv.key);

	while (range.begin < range.begin)
	{
		if (cb_strv_equals_strv((*range.begin).u.strv, kv.u.strv))
		{
			int index = p->mmap.darr.data - range.begin;
			cb_mmap_remove_one(&p->mmap, index);
			return cb_true;
		}
		range.begin++;
	}
	return cb_false;
}

CB_INTERNAL int
cb__remove_all(cb_kv kv)
{
	cb_project_t* p = cb__current_project();
	return cb_mmap_remove(&p->mmap, kv);
}

CB_API void
cb_init()
{
	cb_context_init(&default_ctx);
	current_ctx = &default_ctx;
#ifdef _WIN32
	cb_bool exit_on_exception = cb_true; /* @TODO make this configurable */
	if (exit_on_exception)
	{
		SetUnhandledExceptionFilter(exit_on_exception_handler);
	}
#endif
}

CB_API void
cb_destroy()
{
	/* @TODO remove all projects from current context */
	cb_context_destroy(&default_ctx);
	cb_tmp_reset();
}

CB_API cb_project_t* cb_project(const char* name)
{
	cb_project_t* project;

	if (!cb_try_find_project_by_name_str(name, &project))
	{
		project = cb_create_project(name);
	}

	current_ctx->current_project = project;
	return project;
}

CB_API void
cb_add(const char* key, const char* value)
{
	cb_project_t* p = cb__current_project();

	cb_kv kv = cb_kv_make_with_str(cb_strv_make_str(key), value);

	cb_mmap_insert(&p->mmap, kv);
}

CB_API void
cb_add_f(const char* key, const char* fmt, ...)
{
	cb_dstr s;
	va_list args;
	va_start(args, fmt);

	cb_dstr_init(&s);
	cb_dstr_append_from_fv(&s, s.size, fmt, args);

	cb__add(cb_kv_make_with_dstr(cb_strv_make_str(key), s));

	va_end(args);
}

CB_API void
cb_set(const char* key, const char* value)
{
	/* @FIXME this can easily be optimized, but we don't care about that right now. */
	cb_remove_all(key);
	cb_add(key, value);
}

CB_API void
cb_set_f(const char* key, const char* fmt, ...)
{
	cb_dstr s;
	va_list args;
	va_start(args, fmt);

	cb_dstr_init(&s);
	cb_dstr_append_from_fv(&s, s.size, fmt, args);

	cb__set(cb_kv_make_with_dstr(cb_strv_make_str(key), s));

	va_end(args);
}

CB_API int
cb_remove_all(const char* key)
{
	cb_kv kv = cb_kv_make_with_str(cb_strv_make_str(key), "");
	return cb__remove_all(kv);
}

CB_API int
cb_remove_all_f(const char* key, const char* fmt, ...)
{
	cb_dstr s;
	va_list args;
	int count;
	va_start(args, fmt);

	cb_dstr_init(&s);
	cb_dstr_append_from_fv(&s, s.size, fmt, args);

	count = cb__remove_all(cb_kv_make_with_dstr(cb_strv_make_str(key), s));

	va_end(args);
	return count;
}

CB_API cb_bool
cb_remove_one(const char* key, const char* value)
{
	cb_kv kv = cb_kv_make_with_str(cb_strv_make_str(key), value);
	return cb__remove_one(kv);
}

CB_API cb_bool
cb_remove_one_f(const char* key, const char* fmt, ...)
{
	cb_dstr s;
	va_list args;
	cb_bool was_removed;
	va_start(args, fmt);

	cb_dstr_init(&s);
	cb_dstr_append_from_fv(&s, s.size, fmt, args);

	was_removed = cb__remove_one(cb_kv_make_with_dstr(cb_strv_make_str(key), s));

	va_end(args);
	return was_removed;
}

CB_API void
cb_add_file(const char* file)
{
	cb_dstr absolute_file;
	cb_dstr_init(&absolute_file);
	cb_path_get_absolute(file, &absolute_file);

	cb__add(cb_kv_make_with_dstr(cb_strv_make_str(cbk_FILES), absolute_file));
}

/* Properties are just (strv) values from the map of a project. */
CB_INTERNAL cb_bool
try_get_property(cb_project_t* project, const char* key, cb_kv* result)
{
	if (cb_mmap_try_get_first(&project->mmap, cb_strv_make_str(key), result))
	{
		return cb_true;
	}
	return cb_false;
}

CB_INTERNAL cb_bool
try_get_property_strv(cb_project_t* project, const char* key, cb_strv* result)
{
	cb_kv kv_result;
	if (cb_mmap_try_get_first(&project->mmap, cb_strv_make_str(key), &kv_result))
	{
		*result = kv_result.u.strv;
		return cb_true;
	}
	return cb_false;
}

CB_INTERNAL cb_bool
cb_property_equals(cb_project_t* project, const char* key, const char* comparison_value)
{
	cb_strv result;
	return try_get_property_strv(project, key, &result)
		&& cb_strv_equals_str(result, comparison_value);
}

CB_API const char*
cb_bake_with(cb_toolchain toolchain, const char* project_name)
{
	const char* result = toolchain.bake(&toolchain, project_name);
	cb_log_important("%s", result);
	return result;
}

CB_API const char*
cb_bake(const char* project_name)
{
	return cb_bake_with(cb_toolchain_default(), project_name);
}

CB_INTERNAL void
cb_dstr_add_output_path(cb_dstr* s, cb_project_t* project, const char* default_output_directory)
{
	cb_dstr o;
	cb_dstr_init(&o);

	cb_strv out_dir;
	if (try_get_property_strv(project, cbk_OUTPUT_DIR, &out_dir))
	{
		cb_dstr_append_v(&o, out_dir.data);
	}
	else /* Get default output directory */
	{
		cb_dstr_append_v(&o, default_output_directory, CB_PREFERRED_DIR_SEPARATOR);
		cb_dstr_append_strv(&o, project->name);
	}

	cb_path_get_absolute(o.data, s);

	cb_dstr_ensure_trailing_slash(s);

	cb_dstr_destroy(&o);
}

CB_API const char*
cb_bake_and_run_with(cb_toolchain toolchain, const char* project_name)
{
	const char* result = cb_bake_with(toolchain, project_name);
	if (!result)
	{
		return NULL;
	}
	
	cb_project_t* project = cb_find_project_by_name_str(project_name);

	if (!project)
	{
		return NULL;
	}

	if (!cb_property_equals(project, cbk_BINARY_TYPE, cbk_exe))
	{
		cb_log_error("Cannot use 'cb_bake_and_run_with' for non-executable project");
		return NULL;
	}

	CB_ASSERT(cb_path_exists(result));

	const char* output_dir = cb_tmp_strv_to_str(cb_path_directory_str(result));

	/* Run executable */
	if (!cb_subprocess_with_starting_directory(result, output_dir))
	{
		return NULL;
	}

	return result;
}

CB_API const char*
cb_bake_and_run(const char* project_name)
{
	return cb_bake_and_run_with(cb_toolchain_default(), project_name);
}

#if _WIN32

/* #process #subprocess */

/* the char* cmd should be writtable */
CB_API cb_bool
cb_subprocess_with_starting_directory(const char* cmd, const char* starting_directory)
{
	wchar_t* cmd_w = cb_utf8_to_utf16(cmd);
	wchar_t* starting_directory_w = NULL;

	cb_log_debug("Running subprocess '%s'", cmd);
	if (starting_directory && starting_directory[0])
	{
		starting_directory_w = cb_utf8_to_utf16(starting_directory);
		cb_log_debug("Subprocess started in directory '%s'", starting_directory);
	}

	/* Ensure that everything is written into the outputs before creating a new process that will also write in those outputs */
	fflush(stdout);
	fflush(stderr);

	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	/* Start the child process.
	*  NOTE: we use CreateProcessW because CreateProcessA (with /utf-8) cannot create path with unicode.
	*/
	if (!CreateProcessW(
		NULL,                 /* No module name (use command line) */
		cmd_w,                /* Command line */
		NULL,                 /* Process handle not inheritable */
		NULL,                 /* Thread handle not inheritable */ 
		FALSE,                /* Set handle inheritance to FALSE */
		0,                    /* No creation flags */
		NULL,                 /* Use parent's environment block */
		starting_directory_w, /* Use parent's starting directory */
		&si,                  /* Pointer to STARTUPINFO structure */
		&pi)                  /* Pointer to PROCESS_INFORMATION structure */
		)
	{
		cb_log_error("CreateProcessW failed: %d", GetLastError());
		/* @FIXME cleanup objects */
		return cb_false;
	}

	DWORD result = WaitForSingleObject(
					  pi.hProcess, /* HANDLE hHandle, */
					  INFINITE     /* DWORD  dwMilliseconds */
	);

	if (result == WAIT_FAILED) {
		cb_log_error("Could not wait on child process: %lu", GetLastError());
		/* Close process and thread handles. */
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return cb_false;
	}

	DWORD exit_status = 0;
	if (!GetExitCodeProcess(pi.hProcess, &exit_status)) {
		cb_log_error("Could not get process exit code: %lu", GetLastError());
	}

	if (exit_status != 0) {
		cb_log_error("Command exited with exit code %lu", exit_status);
	}

	/* Close process and thread handles. */
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	
	return exit_status == 0;
}
#else
	

/* space or tab */
CB_INTERNAL cb_bool cb_is_space(char c) { return c == ' ' || c == '\t'; }

CB_INTERNAL cb_bool
cb_is_end_of_quote(const char* str, char quote_type)
{
	return *str != '\0'
		&& *str == quote_type
		&& (str[1] == '\0' || cb_is_space(str[1]));

}
/* Parse arguments, space is a separator and everything between <space or begining of string><quote> and <quote><space or end of string>
* Quotes can be double quotes or simple quotes.
a b c   => 3 arguments a b c
abc     => 1 argument "abc"
a "b" c => 3 arguments a b c;
a"b"c   => 1 argument a"b"c
"a"b"c" => 1 argument a"b"c
*/
CB_INTERNAL const char*
cb_get_next_arg(const char* str, cb_strv* sv)
{
	sv->data = str;
	sv->size = 0;
	if (str == NULL || *str == '\0')
		return NULL;
		
	while (*str != '\0')
	{
		/* Skip spaces */
		while (*str != '\0' && cb_is_space(*str))
			str += 1;

		/* Return early if end of string */
		if (*str == '\0') 
			return sv->size > 0 ? str: NULL;

		/* Handle quotes */
		if (*str == '\'' || *str == '\"')
		{
			const char* quote = str;
			str += 1; /* skip quote */
			
			/* Return early if end of string */
			if (*str == '\0')
				return sv->size > 0 ? str : NULL;

			/* Quote next the previous one so it's an empty content, we look for another item */
			if (cb_is_end_of_quote(str, *quote))
			{
				str += 1; /* Skip quote */
				continue;
			}

			sv->data = str; /* The next argument will begin right after the quote */
			/* Skip everything until the next unescaped quote */
			while (*str != '\0' && !cb_is_end_of_quote(str, *quote))
				str += 1;

			/* Either it's the end of the quoted string, either we reached the null terminating string */
			sv->size = (str - quote) - 1;

			/* Eat trailing quote so that next argument can start with space */
			while (*str != '\0' && *str == *quote)
				str += 1;

			return str;
		}
		else /* is char */
		{
			const char* ch = str;
			while (*str != '\0' && !cb_is_space(*str))
				str += 1;

			sv->data = ch; /* remove quote */
			sv->size = str - ch;
			return str;
		}
	}
	return NULL;
}

#define CB_INVALID_PROCESS (-1)

CB_INTERNAL pid_t
cb_fork_process(char* args[], const char* starting_directory)
{
	pid_t pid = fork();
	if (pid < 0) {
		cb_log_error("Could not fork child process: %s", strerror(errno));
		return CB_INVALID_PROCESS;
	}

	if (pid == 0) {
		/* Change directory in the fork */
		if(starting_directory && starting_directory[0] && chdir(starting_directory) < 0) {
			cb_log_error("Could not change directory to '%s': %s", starting_directory, strerror(errno));
			return CB_INVALID_PROCESS;
		}
		if (execvp(args[0], args) == CB_INVALID_PROCESS) {
			cb_log_error("Could not exec child process: %s", strerror(errno));
			exit(1);
		}
		CB_ASSERT(0 && "unreachable");
	}
	return pid;
}

CB_API cb_bool
cb_subprocess_with_starting_directory(const char* str, const char* starting_directory)
{
	cb_log_debug("Running subprocess '%s'", str);
	if (starting_directory && starting_directory[0])
	{
		cb_log_debug("Subprocess started in directory '%s'", starting_directory);
	}

	/* Ensure that everything is written into the outputs before creating a new process that will also write in those outputs */
	fflush(stdout);
	fflush(stderr);

	cb_dstr cmd;
	cb_dstr_init(&cmd);
	/* cmd will be populated later, but we don't want to reallocate here
	* to avoid getting invalid pointers.
	*/
	/* @FIXME maybe there is better way to create an array of args that does not involve those kind of tricks */
	cb_dstr_reserve(&cmd, strlen(str) + 1); /* +1 for the last space */
	cb_darrT(const char*) args;
	cb_darrT_init(&args);

	cb_strv arg;
	const char* cursor = str;
	while ((cursor = cb_get_next_arg(cursor, &arg)) != NULL)
	{
		const char* arg_ptr = cmd.data + cmd.size;
		cb_darrT_push_back(&args, arg_ptr);

		/* Add string with trailing space */
		cb_dstr_append_f(&cmd, "%.*s ", arg.size, arg.data);

		/* Replace previously added space with \0 */
		cmd.data[cmd.size - 1] = '\0';
	}

	/* last value of the args should be a null value, only append if necessary */
	if (args.darr.size > 0)
	{
		cb_darrT_push_back(&args, NULL);
	}

	pid_t pid  = cb_fork_process((char**)args.darr.data, starting_directory);
	if (pid == CB_INVALID_PROCESS)
	{
		cb_darrT_destroy(&args);
		cb_dstr_destroy(&cmd);
		return cb_false;
	}

	/* wait for process to be done */
	for (;;) {
		int wstatus = 0;
		if (waitpid(pid, &wstatus, 0) < 0) {
			cb_log_error("Could not wait on command (pid %d): '%s'", pid, strerror(errno));
			cb_darrT_destroy(&args);
			cb_dstr_destroy(&cmd);
			return cb_false;
		}

		if (WIFEXITED(wstatus)) {
			int exit_status = WEXITSTATUS(wstatus);
			if (exit_status != 0) {
				cb_log_error("Command exited with exit code '%d'", exit_status);
				cb_darrT_destroy(&args);
				cb_dstr_destroy(&cmd);
				return cb_false;
			}

			break;
		}

		if (WIFSIGNALED(wstatus)) {
			cb_log_error("Command process was terminated by '%s'", strsignal(WTERMSIG(wstatus)));
			cb_darrT_destroy(&args);
			cb_dstr_destroy(&cmd);
			return cb_false;
		}
	}
	
	cb_darrT_destroy(&args);
	cb_dstr_destroy(&cmd);
	return cb_true;
}

#endif

CB_API cb_bool
cb_subprocess(const char* str)
{
	return cb_subprocess_with_starting_directory(str, NULL);
}

#ifdef _WIN32

/* ================================================================ */
/* toolchain MSVC */
/* ================================================================ */

/* #msvc #toolchain */

CB_API const char*
cb_toolchain_msvc_bake(cb_toolchain* tc, const char* project_name)
{
	cb_project_t* project = cb_find_project_by_name_str(project_name);

	if (!project)
	{
		return NULL;
	}
	
	const char* _ = "  "; /* Space to separate command arguments. */

	const char* artefact = ""; /* Resulting artefact path */

	/* cl.exe command */
	cb_dstr str;
	cb_dstr_init(&str);
	
	cb_dstr str_obj; /* to keep track of the .obj generated and copy them.*/
	cb_dstr_init(&str_obj);

	/* Output path - contains the directory path or binary file path (it just depends on how deep we are) */
	cb_dstr str_ouput_path;
	cb_dstr_init(&str_ouput_path);

	/* Format output directory */
	cb_dstr_add_output_path(&str_ouput_path, project, tc->default_directory_base);

	/* Create output directory if it does not exist yet. */
	cb_create_directories(str_ouput_path.data, str_ouput_path.size);

	/* Get working directory, this will be set right before calling the compiler. */
	cb_dstr str_wd;
	cb_dstr_init(&str_wd);

	cb_strv working_dir;
	if (try_get_property_strv(project, cbk_WORKING_DIRECTORY, &working_dir))
	{
		cb_dstr_append_absolute_path(&str_wd, working_dir.data);
	}

	cb_dstr_append_v(&str, "cl.exe /utf-8", _);

	/* Handle binary type */

	cb_bool is_exe = cb_property_equals(project, cbk_BINARY_TYPE, cbk_exe);
	cb_bool is_shared_library = cb_property_equals(project, cbk_BINARY_TYPE, cbk_shared_lib);
	cb_bool is_static_library = cb_property_equals(project, cbk_BINARY_TYPE, cbk_static_lib);

	CB_ASSERT(is_exe || is_shared_library || is_static_library && "Unknown library type");

	if (is_static_library) {
		cb_dstr_append_v(&str, "/c", _);
	}
	else if (is_shared_library) {
		cb_dstr_append_v(&str, "/LD", _);
	}

	/* Add output directory to cl.exe command */
	cb_dstr_append_v(&str, "/Fo", "\"", str_ouput_path.data, "\"", _);

	const char* ext = "";
	ext = is_exe ? ".exe" : ext;
	ext = is_static_library ? ".lib" : ext;
	ext = is_shared_library ? ".dll" : ext;

	artefact = cb_tmp_sprintf("%s%s%s", str_ouput_path.data, project_name, ext);

	/* Define binary output for executable and shared library. Static library is set with the link.exe command*/
	if (is_exe || is_shared_library)
	{
		cb_dstr_append_v(&str, "/Fe", "\"", artefact, "\"", _);
	}

	/* Append compiler flags */
	{
		cb_kv_range range = cb_mmap_get_range_str(&project->mmap, cbk_CXFLAGS);
		cb_kv current;
		while (cb_mmap_range_get_next(&range, &current))
		{
			cb_dstr_append_strv(&str, current.u.strv);
			cb_dstr_append_str(&str,  _);
		}
	}

	/* Append include directories */
	{
		cb_kv_range range = cb_mmap_get_range_str(&project->mmap, cbk_INCLUDE_DIR);
		cb_kv current;
		while (cb_mmap_range_get_next(&range, &current))
		{
			cb_dstr_append_v(&str, "/I", "\"");
			cb_dstr_append_absolute_path(&str, current.u.strv.data);
			cb_dstr_append_v(&str, "\"", _);
		}
	}
	
	/* Append preprocessor definition */
	{
		cb_kv_range range = cb_mmap_get_range_str(&project->mmap, cbk_DEFINES);
		cb_kv current;
		while (cb_mmap_range_get_next(&range, &current))
		{
			cb_dstr_append_v(&str, "/D", "\"");
			cb_dstr_append_strv(&str, current.u.strv);

			cb_dstr_append_v(&str, "\"", _);
		}
	}

	/* Append files and .obj */
	{
		cb_kv_range range = cb_mmap_get_range_str(&project->mmap, cbk_FILES);
		cb_kv current;
		while (cb_mmap_range_get_next(&range, &current))
		{
			cb_dstr_append_v(&str, "\"", current.u.dstr.data, "\"", _);

			cb_strv basename = cb_path_basename(current.u.strv);

			cb_dstr_append_strv(&str_obj, basename);
			cb_dstr_append_str(&str_obj, ".obj");
			cb_dstr_append_str(&str_obj, _);
		}
	}

	/* For each linked project we add the link information to the cl.exe command */
	cb_kv_range range = cb_mmap_get_range_str(&project->mmap, cbk_LINK_PROJECT);
	if (range.count > 0)
	{
		cb_dstr linked_output_dir; /* to keep track of the .obj generated */
		cb_dstr_init(&linked_output_dir);

		cb_dstr_append_v(&str, "/link", _);
		/* Add linker flags */
		{
			cb_kv_range lflag_range = cb_mmap_get_range_str(&project->mmap, cbk_LFLAGS);
			cb_kv lflag;
			while (cb_mmap_range_get_next(&lflag_range, &lflag))
			{
				cb_dstr_append_strv(&str, lflag.u.strv);
				cb_dstr_append_str(&str, _);
			}
		}

		cb_kv current;
		while (cb_mmap_range_get_next(&range, &current))
		{
			cb_strv linked_project_name = current.u.strv;
			cb_dstr_clear(&linked_output_dir);

			cb_project_t* linked_project = cb_find_project_by_name(linked_project_name);
			if (!project)
			{
				continue;
			}

			CB_ASSERT(linked_project);

			cb_bool link_project_is_shared_libary = cb_property_equals(linked_project, cbk_BINARY_TYPE, cbk_shared_lib);
			cb_bool link_project_is_static_libary = cb_property_equals(linked_project, cbk_BINARY_TYPE, cbk_static_lib);

			cb_dstr_add_output_path(&linked_output_dir, linked_project, tc->default_directory_base);

			if (link_project_is_static_libary || link_project_is_shared_libary )
			{
				/* /LIBPATH:"output/dir/" "mlib.lib" */
				cb_dstr_append_v(&str, "/LIBPATH:", "\"", linked_output_dir.data, "\"", _ ,"\"");
				cb_dstr_append_strv(&str, linked_project_name);
				cb_dstr_append_v(&str, ".lib", "\"", _);
			}

			if (link_project_is_shared_libary)
			{
				const char* path = cb_tmp_sprintf("%s%.*s", linked_output_dir.data, linked_project_name.size, linked_project_name.data);
				const char* dll = cb_tmp_sprintf("%s.dll", path);
				const char* lib = cb_tmp_sprintf("%s.lib", path);
				const char* obj = cb_tmp_sprintf("%s.obj", path);
				const char* pdb = cb_tmp_sprintf("%s.pdb", path);
				const char* exp = cb_tmp_sprintf("%s.exp", path);

				if (!cb_copy_file_to_dir(dll, str_ouput_path.data))
				{
					/* @FIXME: Release all allocated objects here. */
					return NULL;
				}
				if (!cb_copy_file_to_dir(obj, str_ouput_path.data))
				{
					/* @FIXME: Release all allocated objects here. */
					return NULL;
				}
				if (!cb_copy_file_to_dir(exp, str_ouput_path.data))
				{
					cb_log_warning("Missing .exp. Shared libraries usually need to have some symbol exported with '__declspec(dllexport)'");
					/* @FIXME: Release all allocated objects here. */
					return NULL;
				}
				if (!cb_copy_file_to_dir(lib, str_ouput_path.data))
				{
					cb_log_warning("Missing .lib file. Shared libraries must create a .lib file for other program to be linked with at compile time.");
					/* @FIXME: Release all allocated objects here. */
					return NULL;
				}

				/* Copy .pdb if there is any. */
				cb_try_copy_file_to_dir(pdb, str_ouput_path.data);
			}
		}

		cb_dstr_destroy(&linked_output_dir);
	}

	/* execute cl.exe */
	if (!cb_subprocess_with_starting_directory(str.data, str_wd.data))
	{
		/* @FIXME: Release all allocated objects here. */
		return NULL;
	}
	
	if (is_static_library)
	{
		cb_dstr_clear(&str);
		/* lib.exe /OUT:output/dir/project_name.lib /LIBPATH:output/dir/ a.obj b.obj c.obj etc. */
		cb_dstr_append_v(&str,
			"lib.exe", _,
			"/OUT:", artefact, _,
			"/LIBPATH:", str_ouput_path.data, _, str_obj.data); /* @TODO add space here? */
	
		if (!cb_subprocess_with_starting_directory(str.data, str_wd.data))
		{
			/* @FIXME: Release all allocated objects here. */
			cb_log_error("Could not execute command to build static library\n");
			return NULL;
		}
	}

	cb_dstr_destroy(&str);
	cb_dstr_destroy(&str_obj);
	cb_dstr_destroy(&str_ouput_path);
	cb_dstr_destroy(&str_wd);

	return artefact;
}

CB_API cb_toolchain
cb_toolchain_msvc()
{
	cb_toolchain tc;
	tc.bake = cb_toolchain_msvc_bake;
	tc.name = "msvc";
	tc.default_directory_base = ".build/msvc";
	return tc;
}

#else

/* ================================================================ */
/* toolchain GCC */
/* ================================================================ */

/* #gcc #toolchain */

CB_INTERNAL cb_bool
cb_strv_ends_with(cb_strv sv, cb_strv rhs)
{
	if (sv.size < rhs.size)
	{
		return cb_false;
	}

	cb_strv sub = cb_strv_make(sv.data + (sv.size - rhs.size), rhs.size);
	return cb_strv_equals_strv(sub, rhs);
}

CB_API const char*
cb_toolchain_gcc_bake(cb_toolchain* tc, const char* project_name)
{
	cb_project_t* project = cb_find_project_by_name_str(project_name);

	if (!project)
	{
		return NULL;
	}

	const char* _ = "  "; /* Space to separate command arguments */
	
	const char* artefact = NULL; /* Resulting artifact path */

	/* gcc command */
	cb_dstr str;
	cb_dstr_init(&str);

	cb_dstr str_obj; /* to keep track of the .o generated */
	cb_dstr_init(&str_obj);

	cb_darrT(const char*) objects;
	cb_darrT_init(&objects);

	/* Output path - contains the directory path or binary file path (it just depends on how deep we are) */
	cb_dstr str_ouput_path;
	cb_dstr_init(&str_ouput_path);

	/* Format output directory */
	cb_dstr_add_output_path(&str_ouput_path, project, tc->default_directory_base);

	/* Create output directory if it does not exist yet. */
	cb_create_directories(str_ouput_path.data, str_ouput_path.size);

	/* Get working directory, this will be set right before calling the compiler. */
	cb_dstr str_wd;
	cb_dstr_init(&str_wd);

	{
		cb_strv working_dir;
		if (try_get_property_strv(project, cbk_WORKING_DIRECTORY, &working_dir))
		{
			cb_dstr_append_absolute_path(&str_wd, working_dir.data);
			cb_dstr_ensure_trailing_slash(&str_wd);
		}
	}

	/* Start command */
	cb_dstr_append_v(&str, "cc ", _);

	/* Handle binary type */
	cb_bool is_exe = cb_property_equals(project, cbk_BINARY_TYPE, cbk_exe);
	cb_bool is_shared_library = cb_property_equals(project, cbk_BINARY_TYPE, cbk_shared_lib);
	cb_bool is_static_library = cb_property_equals(project, cbk_BINARY_TYPE, cbk_static_lib);

	CB_ASSERT(is_exe || is_shared_library || is_static_library && "Unknown library type");

	const char* ext = "";
	ext = is_exe ? "" : ext; /* do not provide extension to executables on linux */
	ext = is_static_library ? ".a" : ext;
	ext = is_shared_library ? ".so" : ext;

	/* Append compiler flags */
	{
		cb_kv_range range = cb_mmap_get_range_str(&project->mmap, cbk_CXFLAGS);
		cb_kv current;
		while (cb_mmap_range_get_next(&range, &current))
		{
			cb_dstr_append_strv(&str, current.u.strv);
			cb_dstr_append_str(&str, _);
		}
	}

	/* Append include directories */
	{
		cb_kv_range range = cb_mmap_get_range_str(&project->mmap, cbk_INCLUDE_DIR);
		cb_kv current;
		while (cb_mmap_range_get_next(&range, &current))
		{
			cb_dstr_append_v(&str, "-I", _, "\"");
			cb_dstr_append_absolute_path(&str, current.u.strv.data);
			cb_dstr_append_v(&str, "\"", _);
		}
	}

	/* Append preprocessor definition */
	{
		cb_kv_range range = cb_mmap_get_range_str(&project->mmap, cbk_DEFINES);
		cb_kv current;
		while (cb_mmap_range_get_next(&range, &current))
		{
			cb_dstr_append_v(&str, "-D");
			cb_dstr_append_strv(&str, current.u.strv);

			cb_dstr_append_v(&str, _);
		}
	}

	if (is_static_library)
	{
		artefact = cb_tmp_sprintf("%slib%s%s", str_ouput_path.data, project_name, ext);
		cb_dstr_append_str(&str, "-c ");
	}

	if (is_shared_library)
	{
		cb_dstr_append_str(&str, "-shared ");
		artefact = cb_tmp_sprintf("%slib%s%s", str_ouput_path.data, project_name, ext);
		cb_dstr_append_f(&str, "-o \"%s\" ", artefact);
	}

	if (is_exe)
	{
		artefact = cb_tmp_sprintf("%s/%s", str_ouput_path.data, project_name);
		cb_dstr_append_f(&str, "-o \"%s\" ", artefact);
	}

	/* Append .c files and .obj */
	{
		cb_kv_range range = cb_mmap_get_range_str(&project->mmap, cbk_FILES);
		cb_kv current;
		while (cb_mmap_range_get_next(&range, &current))
		{
			cb_dstr_append_v(&str, "\"", current.u.dstr.data, "\"", _);

			cb_strv basename = cb_path_basename(current.u.strv);

			if (is_exe || is_static_library)
			{
				cb_dstr_append_v(&str_obj, "\"", str_ouput_path.data, );
				cb_dstr_append_strv(&str_obj, basename);
				cb_dstr_append_v(&str_obj, ".o", "\"", _);

				/* my_object.o */
				cb_darrT_push_back(&objects, cb_tmp_sprintf("%.*s.o", basename.size, basename.data));
			}
		}
	}

	/* For each linked project we add the link information to the gcc command */
	cb_kv_range range = cb_mmap_get_range_str(&project->mmap, cbk_LINK_PROJECT);
	if (range.count > 0)
	{
		/* Add linker flags */
		{
			cb_kv_range lflag_range = cb_mmap_get_range_str(&project->mmap, cbk_LFLAGS);
			cb_kv lflag;
			while (cb_mmap_range_get_next(&lflag_range, &lflag))
			{
				cb_dstr_append_strv(&str, lflag.u.strv);
				cb_dstr_append_str(&str, _);
			}
		}

		/* Give some parameters to the linker to  look for the shared library next to the binary being built */
		cb_dstr_append_str(&str, " -Wl,-rpath,$ORIGIN ");

		cb_dstr linked_output_dir; /* to keep track of the .obj generated and copy them*/
		cb_dstr_init(&linked_output_dir);

		cb_kv current;
		while (cb_mmap_range_get_next(&range, &current))
		{
			cb_strv linked_project_name = current.u.strv;
			cb_dstr_clear(&linked_output_dir);

			cb_project_t* linked_project = cb_find_project_by_name(linked_project_name);
			if (!project)
			{
				continue;
			}

			CB_ASSERT(linked_project);

			cb_bool linked_project_is_shared_libary = cb_property_equals(linked_project, cbk_BINARY_TYPE, cbk_shared_lib);
			cb_bool linked_project_is_static_libary = cb_property_equals(linked_project, cbk_BINARY_TYPE, cbk_static_lib);

			if (linked_project_is_static_libary || linked_project_is_shared_libary)
			{
				cb_dstr_add_output_path(&linked_output_dir, linked_project, tc->default_directory_base);

				/* -L "my/path/" */ 
				cb_dstr_append_v(&str, "-L", _, "\"", linked_output_dir.data, "\"", _);
				/* -l "my_proj" */
				cb_dstr_append_f(&str, "-l \"%.*s\" ", linked_project_name.size, linked_project_name.data);
			}

			if (linked_project_is_shared_libary)
			{
				/* libmy_project.so*/
				const char* so = cb_tmp_sprintf("%slib%.*s.so", linked_output_dir.data, linked_project_name.size, linked_project_name.data);

				/* TODO create copy_files_to_dir */
				if (!cb_copy_file_to_dir(so, str_ouput_path.data))
				{
					/* @FIXME: Release all allocated objects here. */
					return NULL;
				}
			}
		}

		cb_dstr_destroy(&linked_output_dir);
	}

	/* Example: gcc <includes> -c  <c source files> */
	if (!cb_subprocess_with_starting_directory(str.data, str_wd.data))
	{
		/* @FIXME: Release all allocated objects here. */
		return NULL;
	}

	if (is_exe && cb_path_exists(project_name))
	{
		/* set executable permission - only the ownder can read/write/execute the binary */
		char mode_str[] = "0777";
		int mode = strtol(mode_str, 0, 8);

		if (chmod(project_name, mode) < 0)
		{
			cb_log_error("Could not give executable permission to '%s'.", project_name);
			/* @FIXME: Release all allocated objects here. */
			return cb_false;
		}

		cb_dstr_assign_f(&str, "%s%s", str_ouput_path.data, project_name);

		cb_dstr abs_generated_file;
		cb_dstr_clear(&abs_generated_file);
		cb_dstr_append_absolute_path(&abs_generated_file, project_name);

		/* move executable to the output directory */
		if (!cb_move_file(abs_generated_file.data, str.data))
		{
			return NULL;
		}
	}

	else if (is_static_library || is_shared_library)
	{
		/* Move all generated .o file in the working directory to the output directory */
		int i = 0;
		for (; i < cb_darrT_size(&objects); ++i)
		{
			const char* o = cb_darrT_at(&objects, i);
			int index = cb_tmp_save();
			/* /working/directory/my_lib.o */
			const char* o_path = cb_tmp_sprintf("%s%s", str_wd.data, o);
			
			if (!cb_move_file_to_dir(o_path, str_ouput_path.data))
			{
				/* @FIXME cleanup allocated objects */
				return NULL; 
			}

			cb_tmp_restore(index);
		}

		if (is_static_library)
		{
			/* Create libXXX.a in the output directory */
			/* Example: ar -crs libMyLib.a MyObjectAo MyObjectB.o */
			cb_dstr_assign_f(&str, "ar -crs %s %s", artefact, str_obj.data);
			if (!cb_subprocess_with_starting_directory(str.data, str_wd.data))
			{
				/* @FIXME: Release all allocated objects here. */
				return NULL;
			}
		}
	}

	cb_dstr_destroy(&str);
	cb_dstr_destroy(&str_obj);
	cb_dstr_destroy(&str_ouput_path);

	return artefact;
}

CB_API cb_toolchain
cb_toolchain_gcc()
{
	cb_toolchain tc;
	tc.bake = cb_toolchain_gcc_bake;
	tc.name = "gcc";
	tc.default_directory_base = ".build/gcc";
	return tc;
}

#endif /* #else of _WIN32 */

CB_API cb_toolchain
cb_toolchain_default()
{
#ifdef _WIN32
	return cb_toolchain_msvc();
#else
	return cb_toolchain_gcc();
#endif
}

#endif /* CB_IMPLEMENTATION_CPP  */

#ifdef CB_EXTENSIONS
#include "cb_extensions.h"
#endif /* CB_EXTENSIONS */

#endif /* CB_IMPLEMENTATION */