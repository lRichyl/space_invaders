#pragma once

#include "common.h"
#include "arena.h"

char* text_file_to_string(Arena *arena, const char* file_name);
u8* load_binary_file(Arena *arena, const char* file_name, u32 *file_size);
bool file_exists(const char * filename);