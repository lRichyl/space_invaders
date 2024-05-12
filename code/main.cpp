// Space invaders aspect ratio.
#define WINDOW_WIDTH  875
#define WINDOW_HEIGHT 1000

#define internal static
#define local_persist static
#define global_variable static

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t it16;
typedef int32_t i32;
typedef int64_t i64;
typedef i32 b32;

typedef float s32;
typedef double s64;

#include <stdio.h>
#include <string.h>

#include "defines.h"
#include "arena.cpp"
#include "file_handling.cpp"
#include "cpu.cpp"
#include "space_invaders.cpp"

#include "SDL.h"
#include <windows.h>


int main(int argc, char **argv){
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        printf("SDL2 Initialization failed: %s", SDL_GetError());
        return 0;
    }

    // SDL_Window *window;
    SDL_Window *window = SDL_CreateWindow("Hola", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window) {
        printf("Error creating the window: %s", SDL_GetError());
        return 0;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Error creating the renderer: %s", SDL_GetError());
        return 0;
    }

    // Arena arena;
    // init_arena(&arena, 100000);
    // char *file = text_file_to_string(&arena, "win32_handmade.txt");
    // printf("%s\n", file);

    LARGE_INTEGER perf_count_frequency_result;
    QueryPerformanceFrequency(&perf_count_frequency_result);
    i64 perf_count_frequency = perf_count_frequency_result.QuadPart;

    u32 count = 1;

    LARGE_INTEGER last_counter;
    QueryPerformanceCounter(&last_counter);

    b32 is_running = true;
    while (is_running) { // Main loop
        // Input gathering
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                is_running = false;
            }
        }


        // @TODO: Implement the timing https://www.reddit.com/r/EmuDev/comments/ksbvgx/how_to_set_clock_speed_in_c/
        RunSpaceInvaders(&is_running);// Pass cycles delta.

        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);

        LARGE_INTEGER end_counter;
        QueryPerformanceCounter(&end_counter);

        i64 counter_elapsed = end_counter.QuadPart - last_counter.QuadPart; 
        s32 ms_per_frame    = (s32)((1000.0f*(s32)counter_elapsed) / (s32)perf_count_frequency);
        s32 fps = (s32)perf_count_frequency/(s32)counter_elapsed;


        last_counter = end_counter;

        count++;
        if(count == 10000){
            count = 1;
            printf("Milliseconds/frame: %.02fms,  %.02fFPS\n", ms_per_frame, fps);
        }
    }

    SDL_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}