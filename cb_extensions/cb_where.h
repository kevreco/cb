/* @TODO */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#define PATH_SEPARATOR ';'
#define DIR_SEPARATOR '\\'
#else
#include <unistd.h>
#include <sys/stat.h>
#define PATH_SEPARATOR ':'
#define DIR_SEPARATOR '/'
#endif

int is_executable(const char *path) {
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(path);
    return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
#else
    return access(path, X_OK) == 0;
#endif
}

char *find_executable(const char *exe_name) {
    const char *path_env = getenv("PATH");
    if (!path_env) return NULL;

#ifdef _WIN32
    const char *exe_exts[] = {".exe", ".bat", ".cmd", ".com", ""}; // Windows extensions
#else
    const char *exe_exts[] = {""}; // No extension needed on Unix
#endif

    char *path_dup = strdup(path_env);
    char *token = strtok(path_dup, PATH_SEPARATOR == ';' ? ";" : ":");

    while (token) {
        for (int i = 0; exe_exts[i]; i++) {
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s%c%s%s", token, DIR_SEPARATOR, exe_name, exe_exts[i]);

            if (is_executable(full_path)) {
                free(path_dup);
                return strdup(full_path); // Found it
            }
        }
        token = strtok(NULL, PATH_SEPARATOR == ';' ? ";" : ":");
    }

    free(path_dup);
    return NULL; // Not found
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <executable_name>\n", argv[0]);
        return 1;
    }

    char *found = find_executable(argv[1]);
    if (found) {
        printf("%s\n", found);
        free(found);
        return 0;
    } else {
        printf("Executable '%s' not found in PATH.\n", argv[1]);
        return 1;
    }
}