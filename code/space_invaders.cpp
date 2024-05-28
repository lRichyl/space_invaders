static Arena arena;

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
                int pixel_index = (y * pitch) + ((x + bit) * 4);
                
                // Set the RGB values (white for set bit, black for unset bit)
                pixels[pixel_index] = pixel_value;      // Red
                pixels[pixel_index + 1] = pixel_value;  // Green
                pixels[pixel_index + 2] = pixel_value;  // Blue
                pixels[pixel_index + 3] = 0;  // Blue
            }
        }
    }

    

    SDL_UnlockTexture(game_texture);
    // SDL_RenderCopy(renderer, game_texture, NULL, NULL);
    SDL_Rect src_rect = { 0, 0, GAME_WIDTH, GAME_HEIGHT };
    SDL_Rect dst_rect = { -60, 70, WINDOW_HEIGHT, WINDOW_WIDTH}; // Swapped width and height for rotation
    double angle = 90; // 90 degrees counterclockwise rotation
    SDL_RendererFlip flip = (SDL_RendererFlip)(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);

    SDL_RenderCopyEx(renderer, game_texture, NULL, &dst_rect, angle, NULL, flip);
}

static void PrintCPUInfo(CPU *cpu){
    bool sign   = cpu->flags & FLAG_SIGN;
    bool zero   = cpu->flags & FLAG_ZERO;
    bool half   = cpu->flags & FLAG_AUXCARRY;
    bool parity = cpu->flags & FLAG_PARITY;
    bool carry  = cpu->flags & FLAG_CARRY;

    fprintf(file, "A: %X\tBC: %X\tDE: %X\tHL: %X\tSP: %X\tS%X\tZ%X\tA%X\tP%X\tC%X\t\n\n", cpu->A, cpu->BC, cpu->DE, cpu->HL, cpu->SP, sign, zero, half, parity, carry);
}

static void RunSpaceInvaders(b32 *is_running, LARGE_INTEGER starting_time, i64 perf_count_frequency, const u8 *input, SDL_Renderer *renderer){
    static SDL_Texture *game_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, GAME_WIDTH, GAME_HEIGHT);
    static CPU cpu = {};
    static SoundState sound_state = {};
    
    static InstructionInfo instructions_info[INSTRUCTION_COUNT];
    // static 

    static float frame_time = 1000.0f/60.0f; // In milliseconds.
    static int cycles_delta = 0;
    
    static double cpu_period;
    static int cycles_per_frame;

    if(!cpu.is_initialized){
        InitInstructionsInfo(instructions_info);
        InitSounds(&sound_state);


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

        SDL_SetTextureScaleMode(game_texture, SDL_ScaleModeNearest);
    }

    while(cycles_delta < cycles_per_frame){
        UpdateDevices(&cpu, input, &sound_state);
        u8 previous;
        if(cpu.PC < cpu.rom_size){
            previous = cpu.memory[0x90];
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
        if(cpu.PC == 0x1679)
            int a = 0;
    }

    // VBLANK interrupt. Always called after 33333 cycles.
    cpu.instruction = 0xD7; // RST 2
    cpu.call_interrupt = true;
    cpu.interrupts_enabled = false;

    cpu.mid_screen_interrupt_handled = false;

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
}