#pragma once

#ifdef __cplusplus
#define BAR_LIB_EXTERN_C extern "C"
#else
#define BAR_LIB_EXTERN_C
#endif

#ifdef _WIN32

#ifdef BAR_LIB_EXPORT
	#define BAR_LIB_API BAR_LIB_EXTERN_C __declspec(dllexport)
	#else
	#define BAR_LIB_API BAR_LIB_EXTERN_C __declspec(dllimport)
#endif

#else
	#define BAR_LIB_API BAR_LIB_EXTERN_C
#endif

BAR_LIB_API int bar_value();