#pragma once
#include <windows.h>

#include "common.h"
#include "SDL.h"
#include "cpu.h"
#include "arena.h"

// Space invaders aspect ratio.
#define GAME_WIDTH  256 // 875
#define GAME_HEIGHT 224 // 1000
#define WINDOW_WIDTH  GAME_HEIGHT * 4
#define WINDOW_HEIGHT GAME_WIDTH  * 4 

struct TimingInfo{
    float frame_time = 1000.0f/60.0f; // In milliseconds.
    int cycles_delta = 0;
    
    double cpu_period;
    int cycles_per_frame;
};

struct SpaceInvaders{
    Arena arena;
    SDL_Renderer *renderer;
    SDL_Texture *game_texture;
    CPU cpu;
    SoundState sound_state;
    TimingInfo timing;
};

void InitSpaceInvaders(SpaceInvaders *invaders, SDL_Renderer *renderer);
void RunSpaceInvaders(SpaceInvaders *inv, b32 *is_running, LARGE_INTEGER starting_time, i64 perf_count_frequency, const u8 *input);