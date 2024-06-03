#include <stdio.h>

#include "SDL.h"
#include "SDL_mixer.h"
#include <windows.h>

#include "common.h"

#include "space_invaders.h"



int main(int argc, char **argv){
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO) < 0) {
        printf("SDL2 Initialization failed: %s", SDL_GetError());
        return 0;
    }

    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 ){
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
        return 0;
    }

    // SDL_Window *window;
    SDL_Window *window = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
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

    SpaceInvaders space_invaders;
    InitSpaceInvaders(&space_invaders, renderer);

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
      //  SDL_RenderClear(renderer);

        RunSpaceInvaders(&space_invaders, &is_running, last_counter, perf_count_frequency, input, renderer);

        SDL_RenderPresent(renderer);

        LARGE_INTEGER end_counter;
        QueryPerformanceCounter(&end_counter);

        i64 counter_elapsed = end_counter.QuadPart - last_counter.QuadPart; 
        f32 ms_per_frame    = (f32)((1000.0f*(f32)counter_elapsed) / (f32)perf_count_frequency);
        f32 fps = (f32)perf_count_frequency/(f32)counter_elapsed;



        last_counter = end_counter;

        // count++;
        // if(count == 100){
        //     count = 1;
        //     printf("Milliseconds/frame: %.02fms,  %.02fFPS\n", ms_per_frame, fps);
        // }
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    Mix_Quit();
    return 0;
}