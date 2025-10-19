#ifndef CB_ARENA_H
#define CB_ARENA_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cb_arena_block cb_arena_block;

struct cb_arena_block {
    char* memory;
    size_t size;
    size_t offset;
    struct cb_arena_block* next;
};

typedef struct cb_arena cb_arena;
struct cb_arena {
    cb_arena_block* first_block;
    cb_arena_block* current_block;
};

CB_API void cb_arena_init(cb_arena* a);
CB_API void cb_arena_destroy(cb_arena* a);
CB_API void* cb_arena_alloc(cb_arena* a, size_t size);
/* Reset the arena but keep all allocated chunk. */
CB_API void cb_arena_reset(cb_arena* a);

#ifdef __cplusplus
}
#endif

#endif /* CB_ARENA_H */

#ifdef CB_IMPLEMENTATION

#ifndef CB_ARENA_IMPL
#define CB_ARENA_IMPL

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifndef CB_ARENA_ALIGNMENT
#define CB_ARENA_ALIGNMENT 8
#endif
#ifndef CB_MIN_BLOCK_SIZE
/* 8 KB */
#define CB_MIN_BLOCK_SIZE (8 * 1024)
#endif

CB_INTERNAL size_t cb_arena_align_up(size_t n, size_t align) {
    return (n + (align - 1)) & ~(align - 1);
}

/* Allocate a block (arena_block + memory) in a single malloc */
CB_INTERNAL cb_arena_block* cb_arena_create_block(size_t size)
{
    size_t total_size = 0;
    cb_arena_block* block = NULL;
    
    size = (size < CB_MIN_BLOCK_SIZE) ? CB_MIN_BLOCK_SIZE : size;

    /* Allocate both block metadata and memory in one call */
    total_size = sizeof(cb_arena_block) + size;
    block = (cb_arena_block*)CB_MALLOC(total_size);
    if (!block)
    {
        CB_ASSERT(0 && "cb_arena_create_block failed");
        exit(1);
    };

    block->memory = (char*)(block + 1); /* Memory is right after the struct */
    block->size = size;
    block->offset = 0;
    block->next = NULL;

    return block;
}

/*-----------------------------------------------------------------------*/
/* API implementation */
/*-----------------------------------------------------------------------*/

CB_API void cb_arena_init(cb_arena* a)
{
    memset(a, 0, sizeof(cb_arena));
}

CB_API void cb_arena_destroy(cb_arena* a)
{
    cb_arena_block* block = a->first_block;
    while (block)
    {
        cb_arena_block* next = block->next;
        CB_FREE(block); /* Memory and metadata which were allocated together */
        block = next;
    }

    a->first_block = NULL;
    a->current_block = NULL;
}

CB_API void cb_arena_reset(cb_arena* a)
{
    cb_arena_block* block = 0;
    
    for (block = a->first_block; block; block = block->next)
    {
        block->offset = 0;
    }
    a->current_block = a->first_block;
}

CB_API void* cb_arena_alloc(cb_arena* a, size_t size)
{
    cb_arena_block* block = NULL;
    cb_arena_block* new_block = NULL;
    void* ptr = NULL;
    
    size = cb_arena_align_up(size, CB_ARENA_ALIGNMENT);

    /* Lazy allocation on first use. */
    if (!a->current_block)
    {
        a->current_block = cb_arena_create_block(size);
        a->first_block = a->current_block;
    }

    block = a->current_block;

    /* Try to allocate in current block. */
    if (block->offset + size <= block->size)
    {
        ptr = block->memory + block->offset;
        block->offset += size;
        return ptr;
    }

    /* Allocate a new block */
    new_block = cb_arena_create_block(size);

    block->next = new_block;
    a->current_block = new_block;

    ptr = new_block->memory;
    new_block->offset = size;
    return ptr;
}

#endif /* CB_ARENA_IMPL */

#endif /* CB_IMPLEMENTATION */