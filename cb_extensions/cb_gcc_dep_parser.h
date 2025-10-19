#ifndef CB_GCC_DEP_PARSER_H
#define CB_GCC_DEP_PARSER_H

/*
    @TODO write documentation 
    Possible gcc dependency file content:
    
    path/to/target.o : path/to/mytarget.h \
                       path/to/my\ second\ target.h \
                       path/to/my\ third\ target.h
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cb_gcc_dep_parser cb_gcc_dep_parser;
struct cb_gcc_dep_parser {
    
    /* Current position. */
    size_t pos;
    
    /* End of content of the read_buffer. */
    size_t end;
    
    /* Buffer containing a part of the file being parsed. */
    char* read_buffer;  // read buffer
    size_t read_buffer_size;
    
    /* Buffer containing the dependency path if found. */
    char* dep_buffer;   
    size_t dep_buffer_size;
};

/* Initialize parser and skip target */
CB_API void cb_gcc_dep_parser_init(cb_gcc_dep_parser* p, char* read_buffer, size_t read_buffer_size, char* dep_buffer, size_t dep_buffer_size);

/* Reset the parser with the file and skip target */
CB_API void cb_gcc_dep_parser_reset(cb_gcc_dep_parser* p, FILE* file);

/* Get next dependency */
CB_API cb_bool cb_gcc_dep_parser_get_next(cb_gcc_dep_parser* p, FILE* file, cb_strv* dep);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CB_GCC_DEP_PARSER_H */

#ifdef CB_IMPLEMENTATION

#ifndef CB_GCC_DEP_PARSER_IMPL
#define CB_GCC_DEP_PARSER_IMPL

CB_INTERNAL int cb_gcc_dep_get_next_char(cb_gcc_dep_parser *p, FILE* file);

CB_API void cb_gcc_dep_parser_init(cb_gcc_dep_parser* p, char* read_buffer, size_t read_buffer_size, char* dep_buffer, size_t dep_buffer_size)
{
    memset(p, 0, sizeof(cb_gcc_dep_parser));
    p->read_buffer_size = read_buffer_size;
    p->read_buffer = read_buffer;
    
    p->dep_buffer_size = dep_buffer_size;
    p->dep_buffer = dep_buffer;
}

CB_API void cb_gcc_dep_parser_reset(cb_gcc_dep_parser* p, FILE* file)
{
    /* Current char */
    int c = 0;
    typedef int (*cb_gcc_dep_get_next_char_t)(cb_gcc_dep_parser*, FILE*);
    cb_gcc_dep_get_next_char_t next_char = cb_gcc_dep_get_next_char;

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

CB_API cb_bool cb_gcc_dep_parser_get_next(cb_gcc_dep_parser* p, FILE* file, cb_strv* dep)
{
    typedef int (*cb_gcc_dep_get_next_char_t)(cb_gcc_dep_parser*, FILE*);
    cb_gcc_dep_get_next_char_t next_char = cb_gcc_dep_get_next_char;
    
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

CB_INTERNAL int cb_gcc_dep_get_next_char(cb_gcc_dep_parser *p, FILE* file)
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

#endif /* CB_GCC_DEP_PARSER_IMPL */

#endif /* CB_IMPLEMENTATION */