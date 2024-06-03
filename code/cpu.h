#pragma once

#include "common.h"
#include "sound.h"
#include "arena.h"

#define INPUT_DEVICES_AMOUNT 4 
#define OUTPUT_DEVICES_AMOUNT 5 

struct TimingInfo{
    float frame_time = 1000.0f/60.0f; // In milliseconds.
    int cycles_delta = 0;
    
    double cpu_period;
    int cycles_per_frame;
};

struct CPU {
    TimingInfo timing;
    b32 is_initialized;
    float clock_speed;

    union{
        u8 instruction;
        u8 data_byte;
    };
    // Registers
    union{
        u16 BC;
        struct{
            u8 C;
            u8 B;
        };
    };
    union{
        u16 DE;
        struct{
            u8 E;
            u8 D;
        };
    };
    union{
        u16 HL;
        struct{
            u8 L;
            u8 H;
        };
    };

    union{
        u16 PSW;
        struct{
            u8 flags;
            u8 A; // Accumulator
        };
    };
    u8 M; // Pseudo register that contains the dereferenced memory pointed by HL.
    b32 halt;

    u16 SP; // Stack pointer
    u16 PC; // Program counter


    u8 memory[Kilobytes(64)]; // 64 Kilobytes of memory.

    u16 *wide_register_map[4];
    u8  *register_map[8];

    b32 interrupts_enabled;
    b32 call_interrupt;
    b32 mid_screen_interrupt_handled;

    u8 input_devices[INPUT_DEVICES_AMOUNT];
    u8 output_devices[OUTPUT_DEVICES_AMOUNT];
    u8 previous_output_devices[OUTPUT_DEVICES_AMOUNT]; 

    u16 shift_register;


    u32 instruction_size;
    u32 rom_size; // For testing.
};

enum Flag {
    FLAG_CARRY    = 0x01,
    FLAG_PARITY   = 0x04,
    FLAG_AUXCARRY = 0x10,
    FLAG_ZERO     = 0x40,
    FLAG_SIGN     = 0x80
};

void InitCpu(CPU *cpu, Arena *arena);
void UpdateDevices(CPU *cpu, const u8 *input, SoundState *sound_state);
void FetchNextInstructionByte(CPU *cpu);
u32 ExecuteInstruction(CPU *cpu);