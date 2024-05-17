
global_variable Arena arena;

internal void RunSpaceInvaders(b32 *is_running, LARGE_INTEGER starting_time, i64 perf_count_frequency){
    local_persist CPU cpu = {};
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

        // Flags with constant values.
        cpu.flags = cpu.flags | 0x02;    // 1 
        cpu.flags = cpu.flags & ~(0x08); // 0
        cpu.flags = cpu.flags & ~(0x20); // 0

        init_arena(&arena, 100000);

        u8 *loaded_rom = load_binary_file(&arena, "test.asm", &cpu.rom_size);
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
        cpu.register_map[6] = &cpu.memory[cpu.HL];
        cpu.register_map[7] = &cpu.A;
        
        cpu.is_initialized = true;
    }

    while(cycles_delta < cycles_per_frame){
        if(cpu.PC < cpu.rom_size){
            cycles_delta += ExecuteInstruction(&cpu);

            
        }
        else{ // If the program counter goes outside the memory range exit.
            *is_running = false;
        }
    }

    while(1){
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
}