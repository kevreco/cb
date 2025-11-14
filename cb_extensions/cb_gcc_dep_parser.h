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
    size_t pos;
    size_t end;         // current position and end
    cb_bool done;
    char* read_buffer;  // read buffer
    size_t read_buffer_size;
    char* dep_buffer;   // dependency buffer
    size_t dep_buffer_size;
};

/* Initialize parser and skip target */
CB_API void cb_gcc_dep_parser_init(cb_gcc_dep_parser* p, char* read_buffer, size_t read_buffer_size, char* dep_buffer, size_t dep_buffer_size);
CB_API void cb_gcc_dep_parser_reset(cb_gcc_dep_parser* p, FILE* file);
/* Get next dependency */
CB_API cb_bool cb_gcc_dep_parser_get_next(cb_gcc_dep_parser* p, FILE* file, cb_strv* dep);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CB_GCC_DEP_PARSER_H */

#ifdef CB_IMPLEMENTATION

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
    p->done = 0;
    

    memset(p->read_buffer, 0, p->read_buffer_size);
    memset(p->dep_buffer, 0, p->dep_buffer_size);
    
    
    cb_log_important("AAA");
    /* Skip target (skip everything until the next ':') */
     while ((c = next_char(p, file)) != EOF
        && c != ':')
    {
        printf("%c", (char)c);
        continue;
    }
     printf("\n");

    CB_ASSERT(c == EOF || c == ':');

/*
    while ((c = next_char(p, file)) && isspace(c)) 
    {
        continue;
    };
    */
     cb_log_important("LAST CHAR after init: %c", (char)c);
}

CB_API cb_bool cb_gcc_dep_parser_get_next(cb_gcc_dep_parser* p, FILE* file, cb_strv* dep)
{
    typedef int (*cb_gcc_dep_get_next_char_t)(cb_gcc_dep_parser*, FILE*);
    cb_gcc_dep_get_next_char_t next_char = cb_gcc_dep_get_next_char;
    
    char *dst = p->dep_buffer;
    size_t i = 0;
    int c;
    
    *dep = (cb_strv){0};
    
    cb_log_important("next: A");
      
    if (p->done)
    {
        return cb_false;
    }

    /* //Skip whitespaces
    while ((c = next_char(p, file)) 
    {
        continue;
    };
    */
    
    while ((c = next_char(p, file)) != EOF
        && isspace(c) )
    {
        continue;
    }
    
    if (c == EOF)
    {
        return cb_false;
    }
    p->pos -= 1; /* rollback the previous character. */
    cb_log_important("next: B");
    while ((c = next_char(p, file)) != EOF)
    {
         cb_log_important("next: B1");
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
                /* @TODO comment */
                case ' ':
                    c = ' ';
                    break;
                /* @TODO comment */
                case '\r':
                case '\n':
                {
                    cb_log_important("skipped1: '%c'", (char)next);
                    while ((next = next_char(p, file))
                        && isspace(next) )
                    {
                        cb_log_important("skipped2: '%c'", (char)next);
                        continue;
                    }
                    if (next == EOF)
                    {
                        continue;
                    }
                    c = next;
                    break;
                }
                    
                /* @TODO comment */
                case EOF:
                    c = '\\';
                    break;
                /* @TODO comment */
                default: {
                    c = '\\';
                    p->pos--;
                    break;
                }
            }
        }
        
        cb_log_important("CHAR1: %c", (char)c);
        /*cb_log_important("I: %d %d", (int)i, (int)p->dep_buffer_size);*/
        if (i + 1 < p->dep_buffer_size)
        {
            cb_log_important("CHAR2: %c", (char)c);
            dst[i] = (char)c;
            i += 1;
        }
        else
            break;
    }
    
    cb_log_important("next: Y (%d)", (int)i);

    if (i == 0)
    {
        return cb_false;
    }
    
    cb_log_important("next: Z");

/*
    while ((c = next_char(p, file)))
    {
        continue;
    };*/

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
          
            if (feof(file))
            {
               cb_log_important("HALLO 3");
               return EOF; 
            }
            
           
            if (ferror(file))
            {
                cb_log_error("cb_gcc_dep_get_next_char: fread error");
                return EOF;
            }
             cb_log_important("HALLO 4");
            /* File is empty */
            return EOF;
        }
        p->pos = 0;
        p->end = n;
    }

    return p->read_buffer[p->pos++];
}

#endif /* CB_IMPLEMENTATION */