#pragma once

#include "common.h"

struct Arena{
    char* start;
    char* current;
    unsigned int size;
    bool initialized;
};

void init_arena(Arena *arena, unsigned int size);
void* arena_alloc(Arena *arena, unsigned int size);
void free_arena(Arena *arena);

