#include <stdlib.h>
#include <stdio.h>
#include "arena.h"

// Initialize the arena
void init_arena(Arena *arena, unsigned int size) {
    if(!arena->initialized){
        arena->start = (char*)malloc(size);
        arena->current = arena->start;
        arena->size = size;
        arena->initialized = true;
    }
}

// Allocate memory from the arena
void* arena_alloc(Arena *arena, unsigned int size) {
    Assert(arena->initialized);

    if (arena->current + size <= arena->start + arena->size) {
        void* result = arena->current;
        arena->current += size;
        return result;
    } else {
        fprintf(stderr, "Arena overflow\n");
        Assert(false);
    }
   
    return NULL;
}

// Free the entire arena
void free_arena(Arena *arena) {
    Assert(arena->initialized);
    free(arena->start);
    arena->start = arena->current = NULL;
    arena->size = 0;
}
