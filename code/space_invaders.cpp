

global_variable Arena arena;

// internal void RenderSpaceInvaders(CPU *cpu, SDL_Renderer *renderer, SDL_Texture *game_texture){
//     u8 *pixels;
//     int pitch;
//     u16 vram_address = 0x2400;

//     SDL_LockTexture(game_texture, NULL, (void**)&pixels, &pitch);
//     for(int i = 0; i < pitch * GAME_HEIGHT; i += 24){
//         pixels[i]   = (cpu->memory[vram_address + (i / 8 / 3)] & 0x80) * 255;
//         pixels[i+1] = (cpu->memory[vram_address + (i / 8 / 3)] & 0x80) * 255; 
//         pixels[i+2] = (cpu->memory[vram_address + (i / 8 / 3)] & 0x80) * 255;

//         pixels[i+3] = (cpu->memory[vram_address + (i / 8 / 3)] & 0x40) * 255;
//         pixels[i+4] = (cpu->memory[vram_address + (i / 8 / 3)] & 0x40) * 255;
//         pixels[i+5] = (cpu->memory[vram_address + (i / 8 / 3)] & 0x40) * 255;

//         pixels[i+6] = (cpu->memory[vram_address + (i / 8 / 3)] & 0x20) * 255;
//         pixels[i+7] = (cpu->memory[vram_address + (i / 8 / 3)] & 0x20) * 255;
//         pixels[i+8] = (cpu->memory[vram_address + (i / 8 / 3)] & 0x20) * 255;

//         pixels[i+9]  = (cpu->memory[vram_address + (i / 8 / 3)] & 0x10) * 255;
//         pixels[i+10] = (cpu->memory[vram_address + (i / 8 / 3)] & 0x10) * 255;
//         pixels[i+11] = (cpu->memory[vram_address + (i / 8 / 3)] & 0x10) * 255;

//         pixels[i+12] = (cpu->memory[vram_address + (i / 8 / 3)] & 0x08) * 255;
//         pixels[i+13] = (cpu->memory[vram_address + (i / 8 / 3)] & 0x08) * 255;
//         pixels[i+14] = (cpu->memory[vram_address + (i / 8 / 3)] & 0x08) * 255;

//         pixels[i+15] = (cpu->memory[vram_address + (i / 8 / 3)] & 0x04) * 255;
//         pixels[i+16] = (cpu->memory[vram_address + (i / 8 / 3)] & 0x04) * 255;
//         pixels[i+17] = (cpu->memory[vram_address + (i / 8 / 3)] & 0x04) * 255;

//         pixels[i+18] = (cpu->memory[vram_address + (i / 8 / 3)] & 0x02) * 255;
//         pixels[i+19] = (cpu->memory[vram_address + (i / 8 / 3)] & 0x02) * 255;
//         pixels[i+20] = (cpu->memory[vram_address + (i / 8 / 3)] & 0x02) * 255;

//         pixels[i+21] = (cpu->memory[vram_address + (i / 8 / 3)] & 0x01) * 255;
//         pixels[i+22] = (cpu->memory[vram_address + (i / 8 / 3)] & 0x01) * 255;
//         pixels[i+23] = (cpu->memory[vram_address + (i / 8 / 3)] & 0x01) * 255;

//     }
//     SDL_UnlockTexture(game_texture);

//     SDL_RenderCopy(renderer, game_texture, NULL, NULL);
// }

void RenderSpaceInvaders(CPU *cpu, SDL_Renderer *renderer, SDL_Texture *game_texture) {
    u8 *pixels;
    int pitch;
    u16 vram_address = 0x2400;

    SDL_LockTexture(game_texture, NULL, (void**)&pixels, &pitch);

    for (int y = 0; y < GAME_HEIGHT; ++y) {
        for (int x = 0; x < GAME_WIDTH; x += 8) {
            u8 byte = cpu->memory[vram_address + (y * (GAME_WIDTH / 8)) + (x / 8)];
            for (int bit = 0; bit < 8; ++bit) {
                u8 pixel_value = (byte & (0x01 << bit)) ? 0xFF : 0x00;
                int pixel_index = (y * pitch) + ((x + bit) * 3);
                
                // Set the RGB values (white for set bit, black for unset bit)
                pixels[pixel_index] = pixel_value;      // Red
                pixels[pixel_index + 1] = pixel_value;  // Green
                pixels[pixel_index + 2] = pixel_value;  // Blue
            }
        }
    }

    SDL_UnlockTexture(game_texture);
    // SDL_RenderCopy(renderer, game_texture, NULL, NULL);
    SDL_Rect src_rect = { 0, 0, GAME_WIDTH, GAME_HEIGHT };
    SDL_Rect dst_rect = { 0, 0, GAME_HEIGHT, GAME_WIDTH }; // Swapped width and height for rotation
    double angle = -90; // 90 degrees counterclockwise rotation

    SDL_RenderCopyEx(renderer, game_texture, &src_rect, &dst_rect, angle, NULL, SDL_FLIP_NONE);
}

internal void PrintCPUInfo(CPU *cpu){
    bool sign   = cpu->flags & FLAG_SIGN;
    bool zero   = cpu->flags & FLAG_ZERO;
    bool half   = cpu->flags & FLAG_AUXCARRY;
    bool parity = cpu->flags & FLAG_PARITY;
    bool carry  = cpu->flags & FLAG_CARRY;

    printf("A: %X\tBC: %X\tDE: %X\tHL: %X\tSP: %X\tS%X\tZ%X\tA%X\tP%X\tC%X\t\n\n", cpu->A, cpu->BC, cpu->DE, cpu->HL, cpu->SP, sign, zero, half, parity, carry);
}

internal void RunSpaceInvaders(b32 *is_running, LARGE_INTEGER starting_time, i64 perf_count_frequency, const u8 *input, SDL_Renderer *renderer){
    local_persist SDL_Texture *game_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, GAME_WIDTH, GAME_HEIGHT);
    local_persist CPU cpu = {};

    // Put this variables in a struct.
    local_persist float frame_time = 1000.0f/60.0f; // In milliseconds.
    local_persist int cycles_delta = 0;
    
    local_persist double cpu_period;
    local_persist int cycles_per_frame;
    if(!cpu.is_initialized){
        cpu.clock_speed = 2000000; // 2MHz

        cpu_period = 1000.0f/(double)(cpu.clock_speed); // In milliseconds.
        cycles_per_frame = (int)(frame_time/cpu_period);

        printf("Frame time: %f\n", frame_time);
        printf("Period: %f\n", cpu_period);
        printf("Cycles per frame: %d\n", cycles_per_frame);
        printf("Cycles delta: %d\n", cycles_delta);

        cpu.interrupts_enabled = true;
        cpu.call_interrupt     = false;
        cpu.mid_screen_interrupt_handled = false;

        // Flags with constant values.
        cpu.flags = cpu.flags | 0x02;    // 1 
        cpu.flags = cpu.flags & ~(0x08); // 0
        cpu.flags = cpu.flags & ~(0x20); // 0

        init_arena(&arena, 100000);

        u8 *loaded_rom = load_binary_file(&arena, "invaders.bin", &cpu.rom_size);
        memcpy(cpu.memory, loaded_rom, cpu.rom_size);

        cpu.wide_register_map[0] = &cpu.BC;
        cpu.wide_register_map[1] = &cpu.DE;
        cpu.wide_register_map[2] = &cpu.HL;
        cpu.wide_register_map[3] = &cpu.PSW;

        cpu.register_map[0] = &cpu.B;
        cpu.register_map[1] = &cpu.C;
        cpu.register_map[2] = &cpu.D;
        cpu.register_map[3] = &cpu.E;
        cpu.register_map[4] = &cpu.H;
        cpu.register_map[5] = &cpu.L;
        cpu.register_map[6] = &cpu.M;
        cpu.register_map[7] = &cpu.A;

        // Initialize the Input/Output devices.
        cpu.input_devices[0] = 0x0E;
        cpu.input_devices[1] = 0x08;
        cpu.input_devices[2] = 0x00;
        cpu.input_devices[3] = 0x00; 

        cpu.output_devices[0] = 0x00; // Shift amount
        cpu.output_devices[1] = 0x00; // Sounds
        cpu.output_devices[2] = 0x00; // Shift data
        cpu.output_devices[3] = 0x00; // More sounds
        cpu.output_devices[4] = 0x00; // Watchdog. Not necessary for emulation. Ignored.

        cpu.shift_register = 0x00;
        
        cpu.is_initialized = true;
    }

    while(cycles_delta < cycles_per_frame){
        u8 previous = cpu.memory[0x90];
        UpdateDevices(&cpu, input);
        if(cpu.PC < cpu.rom_size){
            if(!cpu.call_interrupt){
                SDL_PumpEvents();

                FetchNextInstructionByte(&cpu);
                cycles_delta += ExecuteInstruction(&cpu);
                // printf("Instruction size: %d\n", cpu.instruction_size);
            }else{
                cpu.halt = false;
                cycles_delta += ExecuteInstruction(&cpu);
                cpu.call_interrupt = false;
            }

            if(cpu.interrupts_enabled){
                if(cycles_delta >= 16667 && !cpu.mid_screen_interrupt_handled){ // Mid screen interrupt.
                    cpu.instruction = 0xCF; // RST 1
                    cpu.call_interrupt = true;
                    cpu.interrupts_enabled = false;
                    cpu.mid_screen_interrupt_handled = true;
                }
            }
        }
        else{ // If the program counter goes outside the memory range exit.
            *is_running = false;
            SDL_DestroyTexture(game_texture);
            return;
        }

        // PrintCPUInfo(&cpu);
    }

    // VBLANK interrupt. Always called after 33333 cycles.
    cpu.instruction = 0xD7; // RST 2
    cpu.call_interrupt = true;
    cpu.interrupts_enabled = false;

    cpu.mid_screen_interrupt_handled = false;

    local_persist i32 count = 0;
    if(count >= 1000){
        count = 0;
    }
    while(1){// Busy wait.
        LARGE_INTEGER end_counter;
        QueryPerformanceCounter(&end_counter);

        i64 counter_elapsed = end_counter.QuadPart - starting_time.QuadPart; 
        f32 ms_elapsed     = (f32)((1000.0f*(f32)counter_elapsed) / (f32)perf_count_frequency);
        if(ms_elapsed >= frame_time){
            // printf("Milliseconds elapsed: \t%f\n", ms_elapsed);
            // printf("Cycles ran last frame: \t%d\n", cycles_delta);
            break;  
        } 
    }


    cycles_delta -= cycles_per_frame;


    RenderSpaceInvaders(&cpu, renderer, game_texture);
    count++;
    if(count == 60){
        count = 0;
        printf("%X\n", cpu.memory[0x20C0]);
        printf("A\n");
    }
}