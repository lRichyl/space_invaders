// Space invaders aspect ratio.
#define WINDOW_WIDTH  875
#define WINDOW_HEIGHT 1000

#define internal static
#define local_persist static
#define global_variable static

#if BUILD_SLOW
#define assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define assert(Expression)
#endif

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef i32 b32;

typedef float f32;
typedef double f64;

#include <stdio.h>
#include <string.h>
#include "SDL.h"

#include "defines.h"
#include "arena.cpp"
#include "file_handling.cpp"
#include "cpu.cpp"

#include <windows.h>
#include "space_invaders.cpp"


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

    const u8 *input = SDL_GetKeyboardState(NULL);

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

        RunSpaceInvaders(&is_running, last_counter, perf_count_frequency, input);

        SDL_RenderClear(renderer);
        // RenderSpaceInvaders();
        SDL_RenderPresent(renderer);

        LARGE_INTEGER end_counter;
        QueryPerformanceCounter(&end_counter);

        i64 counter_elapsed = end_counter.QuadPart - last_counter.QuadPart; 
        f32 ms_per_frame    = (f32)((1000.0f*(f32)counter_elapsed) / (f32)perf_count_frequency);
        f32 fps = (f32)perf_count_frequency/(f32)counter_elapsed;



        last_counter = end_counter;

        count++;
        if(count == 100){
            count = 1;
            printf("Milliseconds/frame: %.02fms,  %.02fFPS\n", ms_per_frame, fps);
        }
    }

    SDL_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}