#include <stdio.h>

#include "space_invaders.h"
#include "file_handling.h"
#include "sound.h"
#include "disassembler.h"

static void RenderSpaceInvaders(SpaceInvaders *invaders) {
    u8 *pixels;
    int pitch;
    u16 vram_address = 0x2400;
    SDL_LockTexture(invaders->game_texture, NULL, (void**)&pixels, &pitch);

    for (int y = 0; y < GAME_HEIGHT; ++y) {
        for (int x = 0; x < GAME_WIDTH; x += 8) {
            u8 byte = invaders->cpu.memory[vram_address + (y * (GAME_WIDTH / 8)) + (x / 8)];
            for (int bit = 0; bit < 8; ++bit) {
                u8 pixel_value = (byte & (0x01 << bit)) ? 0xFF : 0x00;
                int pixel_index = (y * pitch) + ((x + bit) * 4);
                
                // Set the RGB values (white for set bit, black for unset bit)
                pixels[pixel_index] = pixel_value;      // Red
                pixels[pixel_index + 1] = pixel_value;  // Green
                pixels[pixel_index + 2] = pixel_value;  // Blue
                pixels[pixel_index + 3] = 0;  // Blue
            }
        }
    }

    

    SDL_UnlockTexture(invaders->game_texture);
    // SDL_RenderCopy(renderer, game_texture, NULL, NULL);
    SDL_Rect dst_rect = { -60, 70, WINDOW_HEIGHT, WINDOW_WIDTH}; // Swapped width and height for rotation
    double angle = 90; // 90 degrees counterclockwise rotation
    SDL_RendererFlip flip = (SDL_RendererFlip)(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);

    SDL_RenderCopyEx(invaders->renderer, invaders->game_texture, NULL, &dst_rect, angle, NULL, flip);
}

// static void PrintCPUInfo(CPU *cpu){
//     bool sign   = cpu->flags & FLAG_SIGN;
//     bool zero   = cpu->flags & FLAG_ZERO;
//     bool half   = cpu->flags & FLAG_AUXCARRY;
//     bool parity = cpu->flags & FLAG_PARITY;
//     bool carry  = cpu->flags & FLAG_CARRY;

//     // printf("A: %X\tBC: %X\tDE: %X\tHL: %X\tSP: %X\tS%X\tZ%X\tA%X\tP%X\tC%X\t\n\n", cpu->A, cpu->BC, cpu->DE, cpu->HL, cpu->SP, sign, zero, half, parity, carry);
// }

void InitSpaceInvaders(SpaceInvaders *invaders, SDL_Renderer *renderer){
    init_arena(&invaders->arena, 100000);
    invaders->game_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, GAME_WIDTH, GAME_HEIGHT);
    invaders->cpu = {};
    invaders->sound_state = {};
    invaders->renderer = renderer;
    // invaders->InstructionInfo instructions_info[INSTRUCTION_COUNT];
    // invaders->
    // InitInstructionsInfo(instructions_info);

    InitSounds(&invaders->sound_state);
    InitCpu(&invaders->cpu, &invaders->arena);

    invaders->timing.cpu_period = 1000.0f/(double)(invaders->cpu.clock_speed); // In milliseconds.
    invaders->timing.cycles_per_frame = (int)(invaders->timing.frame_time/invaders->timing.cpu_period);
    invaders->timing.frame_time = 1000.0f/60.0f; // In milliseconds.
    invaders->timing.cycles_delta = 0;

    printf("Frame time: %f\n",       invaders->timing.frame_time);
    printf("Period: %f\n",           invaders->timing.cpu_period);
    printf("Cycles per frame: %d\n", invaders->timing.cycles_per_frame);
    printf("Cycles delta: %d\n",     invaders->timing.cycles_delta);

    SDL_SetTextureScaleMode(invaders->game_texture, SDL_ScaleModeNearest);
}

void RunSpaceInvaders(SpaceInvaders *invaders, b32 *is_running, LARGE_INTEGER starting_time, i64 perf_count_frequency, const u8 *input){
    CPU *cpu = &invaders->cpu;
    while(invaders->timing.cycles_delta < invaders->timing.cycles_per_frame){
        UpdateDevices(&invaders->cpu, input, &invaders->sound_state);
        if(cpu->PC < cpu->rom_size){
            if(!cpu->call_interrupt){
                SDL_PumpEvents();

                FetchNextInstructionByte(cpu);
                invaders->timing.cycles_delta += ExecuteInstruction(cpu);
                // printf("Instruction size: %d\n", cpu->instruction_size);
            }else{
                cpu->halt = false;
                cpu->call_interrupt = false;
                invaders->timing.cycles_delta += ExecuteInstruction(cpu);
            }

            if(cpu->interrupts_enabled){
                if(invaders->timing.cycles_delta >= 16667 && !cpu->mid_screen_interrupt_handled){ // Mid screen interrupt.
                    cpu->instruction = 0xCF; // RST 1
                    cpu->call_interrupt = true;
                    cpu->interrupts_enabled = false;
                    cpu->mid_screen_interrupt_handled = true;
                }
            }
        }
        else{ // If the program counter goes outside the memory range exit.
            *is_running = false;
            SDL_DestroyTexture(invaders->game_texture);
            return;
        }
    }

    // VBLANK interrupt. Always called after 33333 cycles.
    cpu->instruction = 0xD7; // RST 2
    cpu->call_interrupt = true;
    cpu->interrupts_enabled = false;

    cpu->mid_screen_interrupt_handled = false;

    while(1){// Busy wait.
        LARGE_INTEGER end_counter;
        QueryPerformanceCounter(&end_counter);

        i64 counter_elapsed = end_counter.QuadPart - starting_time.QuadPart; 
        f32 ms_elapsed     = (f32)((1000.0f*(f32)counter_elapsed) / (f32)perf_count_frequency);
        if(ms_elapsed >= invaders->timing.frame_time){
            // printf("Milliseconds elapsed: \t%f\n", ms_elapsed);
            // printf("Cycles ran last frame: \t%d\n", cycles_delta);
            break;  
        } 
    }


    invaders->timing.cycles_delta -= invaders->timing.cycles_per_frame;


    RenderSpaceInvaders(invaders);
}