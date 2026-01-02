#ifndef CB_DEP_PARSER_H
#define CB_DEP_PARSER_H

/*
    - Use the cb_msvc_parser_xxx API to parse gcc dependencies format.
    
    Possible msvc dependency file content:
    
    file.cpp
    Note: including file: include1.h
    Note: including file:  include2.h
    Note: including file:   include3.h
    Note: including file: include4.h
                       
    - Use the cb_gcc_parser_xxx API to parse gcc dependencies format.
    
    Possible gcc dependency file content:
    
    path/to/target.o : path/to/mytarget.h \
                       path/to/my\ second\ target.h \
                       path/to/my\ third\ target.h
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cb_dep_parser cb_dep_parser;
struct cb_dep_parser {
    
    /* Current position. */
    size_t pos;
    
    /* End of content of the read_buffer. */
    size_t end;
    
    /* Buffer containing a part of the file being parsed. */
    char* read_buffer;
    size_t read_buffer_size;
    
    /* Buffer containing the dependency path if found. */
    char* dep_buffer;   
    size_t dep_buffer_size;
};

/* Initialize parser and skip target */
CB_API void cb_gcc_dep_parser_init(cb_dep_parser* p, char* read_buffer, size_t read_buffer_size, char* dep_buffer, size_t dep_buffer_size);

/* Reset the parser with the file and skip target. */
CB_API void cb_gcc_dep_parser_reset(cb_dep_parser* p, FILE* file);

/* Get next dependency. */
CB_API cb_bool cb_gcc_dep_parser_get_next(cb_dep_parser* p, FILE* file, cb_strv* dep);


CB_API void cb_msvc_dep_parser_init(cb_dep_parser* p);

/* Reset the parser with the file and skip target. */
CB_API void cb_msvc_dep_parser_reset(cb_dep_parser* p, const char* str);

/* Get next dependency. */
CB_API cb_bool cb_msvc_dep_parser_get_next(cb_dep_parser* p, const char* str, cb_strv* dep);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CB_DEP_PARSER_H */

#ifdef CB_IMPLEMENTATION

#ifndef CB_DEP_PARSER_IMPL
#define CB_DEP_PARSER_IMPL

CB_INTERNAL int cb_gcc_dep_get_next_char(cb_dep_parser *p, FILE* file);

CB_API void cb_gcc_dep_parser_init(cb_dep_parser* p, char* read_buffer, size_t read_buffer_size, char* dep_buffer, size_t dep_buffer_size)
{
    memset(p, 0, sizeof(cb_dep_parser));
    p->read_buffer_size = read_buffer_size;
    p->read_buffer = read_buffer;
    
    p->dep_buffer_size = dep_buffer_size;
    p->dep_buffer = dep_buffer;
}

CB_API void cb_gcc_dep_parser_reset(cb_dep_parser* p, FILE* file)
{
    /* Current char */
    int c = 0;
    typedef int (*cb_dep_get_next_char_t)(cb_dep_parser*, FILE*);
    cb_dep_get_next_char_t next_char = cb_gcc_dep_get_next_char;

    p->pos = 0;
    p->end = 0;

    memset(p->read_buffer, 0, p->read_buffer_size);
    memset(p->dep_buffer, 0, p->dep_buffer_size);

    /* Skip target (skip everything until the next ':') */
    while ((c = next_char(p, file)) != EOF
        && c != ':')
    {
        continue;
    }

    CB_ASSERT(c == EOF || c == ':');
}

CB_API cb_bool cb_gcc_dep_parser_get_next(cb_dep_parser* p, FILE* file, cb_strv* dep)
{
    typedef int (*cb_dep_get_next_char_t)(cb_dep_parser*, FILE*);
    cb_dep_get_next_char_t next_char = cb_gcc_dep_get_next_char;
    
    char *dst = p->dep_buffer;
    size_t i = 0;
    int c;
    
    *dep = (cb_strv){0};


    while ((c = next_char(p, file)) != EOF
        && isspace(c) )
    {
        continue;
    }
    
    if (c == EOF)
    {
        return cb_false;
    }
    
    /* Rollback the previous character. */
    p->pos -= 1; 

    while ((c = next_char(p, file)) != EOF)
    {
        /* Skip whitespaces */
        if (isspace(c)) 
        {
            break;
        };

        if (c == '\\')
        {
            int next = next_char(p, file);
            switch(next)
            {
                /* Consider back slash + space as space. */
                case ' ':
                    c = ' ';
                    break;
                /* Ignore back slash + new + any amount of whitespace. */
                case '\r':
                case '\n':
                {
                    while ((next = next_char(p, file))
                        && isspace(next) )
                    {
                        continue;
                    }
                    if (next == EOF)
                    {
                        continue;
                    }
                    c = next;
                    break;
                }

                /* Consider normal backslash if it's followed by anything else than newline or space. */
                default: {
                    c = '\\';
                    p->pos--;
                    break;
                }
            }
        }

        if (i + 1 < p->dep_buffer_size)
        {
            dst[i] = (char)c;
            i += 1;
        }
        else
            break;
    }

    if (i == 0)
    {
        return cb_false;
    }

    dep->data = dst;
    dep->size = i;
    dst[i] = '\0';
    return cb_true;
}

CB_API void cb_msvc_dep_parser_init(cb_dep_parser* p)
{
    memset(p, 0, sizeof(cb_dep_parser));
}

CB_API void cb_msvc_dep_parser_reset(cb_dep_parser* p, const char* str)
{
    /* Current char */
    const char* c = str;

    p->pos = 0;

    /* Skip target (skip everything until the next new line) */
    while (*c != '\0'
        && *c != '\n'
        && *c != '\r')
    {
        c += 1;
    }
    
    CB_ASSERT(*c == '\0' || *c == '\n' || *c == '\r');
    
    p->pos += c - str;
}

/* WIN32-only
   If it's an 'include line entry' displayed by \showIncludes,
   returns the length of the line, otherwise returns 0 */
CB_INTERNAL cb_size cb_msvc_is_show_include_line(const char* line)
{
    static cb_strv prefixes[] = {
        CB_STRV("Note: including file: "),              /* English */
        CB_STRV("Remarque : inclusion du fichier :  "), /* French */
        CB_STRV("Hinweis: Einlesen der Datei: "),       /* German */
        CB_STRV("Nota: file incluso  "),                /* Italian */
        CB_STRV("注意: 包含文件:  "),                    /* Chinese */
        CB_STRV("メモ: インクルード ファイル:  ")               /* Japanese */
    };
    
    int sizeof_array = sizeof(prefixes) / sizeof(prefixes[0]);
    int i = 0;
    
    for(i = 0; i < sizeof_array; i += 1)
    {
        if (strncmp(line, prefixes[i].data, prefixes[i].size) == 0)
        {
            return prefixes[i].size;
        }
    }
    
    return 0;
}

CB_API cb_bool cb_msvc_dep_parser_get_next(cb_dep_parser* p, const char* str, cb_strv* dep)
{
    int include_len = 0;

    const char* dep_path_start = NULL;
    const char* dep_path_end = NULL;
    cb_strv dep_path = { 0 };
    cb_bool is_system_file = cb_false;
    
    const char* begin = str + p->pos;
    const char* c = begin;
   
    /* Parse all files output by \showIncludes */
    while(*c != '\0')
    {
        if (*c == 'N'     /* Starts with Note: or Nota: */
            || *c == 'R'  /* Starts with Remarque: */
            || *c == 'H'  /* Starts with Hinweis: */
            || *c == '\xE3' /* Starts with メ */
            || *c == '\xE6' /* Starts with 注 */
        )
        {
            /* Check if line starts with the marker of the /showIncludes */
            cb_size prefix_len = cb_msvc_is_show_include_line(c);
            
            if(prefix_len > 0)
            {
                /* Skip prefix. */
                c += prefix_len;

                /* Skip spaces. */
                while(*(c) == ' ')
                {
                    c += 1;
                }
              
                dep_path_start = c;
                
                /* Skip until end of end of line. */
                while( *(c) != '\0'
                    && *(c) != '\n'
                    && *(c) != '\r') 
                {
                    c += 1;
                }

                /* Get end path of the current include, removing \n or \r\n */
                dep_path_end = c;

                if (dep_path_end > dep_path_start)
                {
                    include_len = (int)(dep_path_end - dep_path_start);

                    dep_path = cb_strv_make(dep_path_start, include_len);

                    /* Write dep to dep_store_file if it's not a system include */
                    is_system_file =
                        cb_strv_contains_str(dep_path, "\\Microsoft Visual Studio\\")
                        || cb_strv_contains_str(dep_path, "\\Windows Kits\\");
                        
                    if (!is_system_file)
                    {
                        /* Update position. */
                        p->pos = c - str;
                        *dep = dep_path;
                        return cb_true;
                    }
                }
            }
        }
        
        c += 1;
    }
    
    /* Update position. */
    p->pos = c - str;
    return cb_false;
}

CB_INTERNAL int cb_gcc_dep_get_next_char(cb_dep_parser *p, FILE* file)
{
     size_t n = 0;

    /* If current pos reach the end, we refill the buffer */
    if (p->pos >= p->end)
    {
        n = fread(p->read_buffer, 1, p->read_buffer_size, file);

        if (n == 0)
        {
            /* File is empty */
            if (feof(file))
            {
               return EOF; 
            }
            
            if (ferror(file))
            {
                cb_log_error("cb_gcc_dep_get_next_char: fread error");
                return EOF;
            }
            /* File is empty */
            cb_log_error("cb_gcc_dep_get_next_char: ???");
            return EOF;
        }
        p->pos = 0;
        p->end = n;
    }

    return p->read_buffer[p->pos++];
}

#endif /* CB_DEP_PARSER_IMPL */

#endif /* CB_IMPLEMENTATION */