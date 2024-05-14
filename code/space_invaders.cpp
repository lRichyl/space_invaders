
global_variable Arena arena;

internal u32 RunSpaceInvaders(b32 *is_running){
    local_persist CPU cpu = {};
    if(!cpu.is_initialized){
        cpu.clock_speed = 2000000; // 2MHz
        cpu.is_initialized = true;

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
        cpu.wide_register_map[3] = &cpu.SP;
        cpu.wide_register_map[4] = &cpu.PSW;

        cpu.register_map[0] = &cpu.B;
        cpu.register_map[1] = &cpu.C;
        cpu.register_map[2] = &cpu.D;
        cpu.register_map[3] = &cpu.E;
        cpu.register_map[4] = &cpu.H;
        cpu.register_map[5] = &cpu.L;
        cpu.register_map[6] = &cpu.memory[cpu.HL];
        cpu.register_map[7] = &cpu.A;
    }

    if(cpu.PC < cpu.rom_size){
        u32 cycles = ExecuteInstruction(&cpu);

        return cycles;
    }
    else{
        *is_running = false;
        return 0;
    }
}