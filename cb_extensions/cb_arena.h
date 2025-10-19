#ifndef CB_ARENA_H
#define CB_ARENA_H

#include <stddef.h>

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

void cb_arena_init(cb_arena* a);
void cb_arena_destroy(cb_arena* a);
void* cb_arena_alloc(cb_arena* a, size_t size);
void cb_arena_reset(cb_arena* a);


/* CB_ARENA_IMPLEMENTATION */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define CB_ARENA_ALIGNMENT 8
#define CB_MIN_BLOCK_SIZE (8 * 1024) // 8 KB

static size_t cb_arena_align_up(size_t n, size_t align) {
    return (n + (align - 1)) & ~(align - 1);
}

// Allocate a block (arena_block + memory) in a single malloc
static cb_arena_block* cb_arena_create_block(size_t size) {
    size = (size < CB_MIN_BLOCK_SIZE) ? CB_MIN_BLOCK_SIZE : size;

    // Allocate both block metadata and memory in one call
    size_t total_size = sizeof(cb_arena_block) + size;
    cb_arena_block* block = (cb_arena_block*)malloc(total_size);
    if (!block)
    {
        CB_ASSERT(0 && "cb_arena_create_block failed");
        exit(1);
    };

    block->memory = (char*)(block + 1); // memory follows the struct
    block->size = size;
    block->offset = 0;
    block->next = NULL;

    return block;
}

void cb_arena_init(cb_arena* a) {

    a->first_block = NULL;
    a->current_block = NULL;
}

void cb_arena_destroy(cb_arena* a)
{
    cb_arena_block* block = a->first_block;
    while (block)
    {
        cb_arena_block* next = block->next;
        free(block); /* memory and metadata were allocated together */
        block = next;
    }

    a->first_block = NULL;
    a->current_block = NULL;
}

void cb_arena_reset(cb_arena* a)
{
    for (cb_arena_block* block = a->first_block; block; block = block->next)
    {
        block->offset = 0;
    }
    a->current_block = a->first_block;
}

void* cb_arena_alloc(cb_arena* a, size_t size)
{
    size = cb_arena_align_up(size, CB_ARENA_ALIGNMENT);

    // Lazy allocation on first use
    if (!a->current_block) {
        cb_arena_block* block = cb_arena_create_block(size);
        a->first_block = a->current_block = block;
    }

    cb_arena_block* block = a->current_block;

    // Try to allocate in current block
    if (block->offset + size <= block->size) {
        void* ptr = block->memory + block->offset;
        block->offset += size;
        return ptr;
    }

    // Allocate a new block
    cb_arena_block* new_block = cb_arena_create_block(size);

    block->next = new_block;
    a->current_block = new_block;

    void* ptr = new_block->memory;
    new_block->offset = size;
    return ptr;
}

#endif /* CB_ARENA_H */