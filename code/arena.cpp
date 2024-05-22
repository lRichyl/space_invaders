#include <stdlib.h>
#include <stdio.h>

typedef struct {
    char* start;
    char* current;
    unsigned int size;
    bool initialized;
} Arena;

// Initialize the arena
static void init_arena(Arena *arena, unsigned int size) {
    if(!arena->initialized){
        arena->start = (char*)malloc(size);
        arena->current = arena->start;
        arena->size = size;
        arena->initialized = true;
    }
}

// Allocate memory from the arena
static void* arena_alloc(Arena *arena, unsigned int size) {
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
static void free_arena(Arena *arena) {
    Assert(arena->initialized);
    free(arena->start);
    arena->start = arena->current = NULL;
    arena->size = 0;
}
