struct CPU {
    b32 is_initialized;
    s32 clock_speed;

    union{
        u8 instruction;
        u8 data_byte;
    };
    // Registers
    u8 A; // Accumulator
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
    u8 *M; // Pseudo register that contains the dereferenced memory pointed by HL.

    u16 SP; // Stack pointer
    u16 PC; // Program counter

    u8 flags;

    u8 memory[Kilobytes(64)]; // 64 Kilobytes of memory.

    u16 *wide_register_map[4];
    u8  *register_map[8];

    u32 rom_size; // For testing.
};

enum Flag {
    FLAG_CARRY    = 0x01,
    FLAG_PARITY   = 0x04,
    FLAG_AUXCARRY = 0x10,
    FLAG_ZERO     = 0x40,
    FLAG_SIGN     = 0x80
};

internal void SetFlag(CPU *cpu, Flag flag){
    cpu->flags = cpu->flags | flag;
}

internal void UnSetFlag(CPU *cpu, Flag flag){
    cpu->flags = cpu->flags & ~(flag);
}

internal void FetchNextInstructionByte(CPU *cpu){
    cpu->instruction = cpu->memory[cpu->PC];
    cpu->PC++;
}

internal u8 SumAndSetFlags(CPU *cpu, u8 summand_left, u8 summand_right, b32 check_carry = false){
    u8 result = summand_left + summand_right;
    if(result == 0)     
        SetFlag(cpu, FLAG_ZERO);
    else
        UnSetFlag(cpu, FLAG_ZERO);

    if(result & 0x80)
        SetFlag(cpu, FLAG_SIGN);
    else
        UnSetFlag(cpu, FLAG_SIGN);

    if(result % 2 == 0 && result > 0)
        SetFlag(cpu, FLAG_PARITY);
    else
        UnSetFlag(cpu, FLAG_PARITY);

    if(summand_left < 0x10 && result >= 0x10) 
        SetFlag(cpu, FLAG_AUXCARRY);
    else
        UnSetFlag(cpu, FLAG_AUXCARRY);

    if(check_carry){
        if(summand_left > result) 
            SetFlag(cpu, FLAG_CARRY);
        else
            UnSetFlag(cpu, FLAG_CARRY);
    }

    return result;
}

internal u8 SubstractAndSetFlags(CPU *cpu, u8 minuend, u8 sustrahend, b32 check_carry = false){
    u8 result = minuend - sustrahend;
    if(result == 0)     
        SetFlag(cpu, FLAG_ZERO);
    else
        UnSetFlag(cpu, FLAG_ZERO);

    if(result & 0x80)
        SetFlag(cpu, FLAG_SIGN);
    else
        UnSetFlag(cpu, FLAG_SIGN);

    if(result % 2 == 0 && result > 0)
        SetFlag(cpu, FLAG_PARITY);
    else
        UnSetFlag(cpu, FLAG_PARITY);

    u8 nibble_minuend    = minuend    & 0x0F;
    u8 nibble_sustrahend = sustrahend & 0x0F;
    if(nibble_minuend < nibble_sustrahend) 
        SetFlag(cpu, FLAG_AUXCARRY);
    else
        UnSetFlag(cpu, FLAG_AUXCARRY);

    if(check_carry){
        if(minuend < sustrahend) 
            SetFlag(cpu, FLAG_CARRY);
        else
            UnSetFlag(cpu, FLAG_CARRY);
    }

    return result;
}

internal u32 ExecuteInstruction(CPU *cpu){
    // This function returns the duration in cycles of the current instruction.
    cpu->M = &cpu->memory[cpu->HL]; // @TODO: Verify this.
    FetchNextInstructionByte(cpu);
    switch(cpu->instruction){
    case 0x00:// NOP Instruction.
    case 0x10:
    case 0x20:
    case 0x30:
    case 0x08:
    case 0x18:
    case 0x28:
    case 0x38: {
        printf("NOP instruction\n");

        return 4; // Duration in cycles.
    }

    // LXI Instruction. LXI R, D16  :  RH <- byte 3, RL <- byte 2
    case 0x01:
    case 0x11:
    case 0x21:{
        u8 register_pair_index = (cpu->instruction & 0xF0) >> 4;
    
        FetchNextInstructionByte(cpu);
        u8 low  = cpu->data_byte;
        FetchNextInstructionByte(cpu);
        u8 high = cpu->data_byte;
        
        *cpu->wide_register_map[register_pair_index] = ((high << 8) | low);
        return 10;
    }
    case 0x31:{ // LXI Instruction, storing in the stack pointer.
        FetchNextInstructionByte(cpu);
        u8 low  = cpu->data_byte;
        FetchNextInstructionByte(cpu);
        u8 high = cpu->data_byte;

        cpu->SP = ((high << 8) | low);
        return 10;
    }

    // STAX Instruction.    STAX R  : (RP) <- A   .. BC and DE only.
    case 0x02:
    case 0x12:{
        u8 register_pair_index = (cpu->instruction & 0xF0) >> 4;
        *cpu->wide_register_map[register_pair_index] = cpu->A;

        return 7;
    }

    // SHLD Instrucion.  SHLD adr  : (adr) <-L; (adr+1)<-H
    case 0x22:{
        FetchNextInstructionByte(cpu);
        u8 low  = cpu->data_byte;
        FetchNextInstructionByte(cpu);
        u8 high = cpu->data_byte;
        u16 address = ((high << 8) | low);

        cpu->memory[address]     = cpu->L;
        cpu->memory[address + 1] = cpu->H;

        return 16;   
    }

    // STA Instruction.  STA adr  : (adr) <- A
    case 0x32:{
        FetchNextInstructionByte(cpu);
        u8 low  = cpu->data_byte;
        FetchNextInstructionByte(cpu);
        u8 high = cpu->data_byte;
        u16 address = ((high << 8) | low);

        cpu->memory[address] = cpu->A;

        return 13;
    }

    // INX Instrucion.  INX R  :  RP <- RP+1
    case 0x03:
    case 0x13:
    case 0x23:
    case 0x33:{
        u8 register_pair_index = (cpu->instruction & 0xF0) >> 4;
        (*cpu->wide_register_map[register_pair_index])++;

        return 5;
    }

    // INR Instruction.  INR R  :  Z, S, P, AC   :  R <- R+1   Where R is an even register.
    case 0x04:
    case 0x14:
    case 0x24:{
        u8 register_index = ((cpu->instruction & 0xF0) >> 4) * 2;
        *cpu->register_map[register_index] = SumAndSetFlags(cpu, *cpu->register_map[register_index], 1);

        return 5;
    }

    // INR Instruction.  INR M  :  Z, S, P, AC   :  M <- M+1
    case 0x34:{
        *cpu->M = SumAndSetFlags(cpu, *cpu->M, 1);

        return 10;
    }

    // DCR Instruction.  DCR R  :  Z, S, P, AC   :  R <- R-1     Where R is an even register.
    case 0x05:
    case 0x15:
    case 0x25:{
        u8 register_index = ((cpu->instruction & 0xF0) >> 4) * 2;
        *cpu->register_map[register_index] = SubstractAndSetFlags(cpu, *cpu->register_map[register_index], 1);

        return 5;
    }

    // DCR Instruction.  DCR M  :  Z, S, P, AC   :  M <- M-1 
    case 0x35:{
        *cpu->M = SubstractAndSetFlags(cpu, *cpu->M, 1);

        return 10;
    }

    // MVI Instruction.  MVI R, D8  :  R <- byte 2   Where R is an even register.
    case 0x06:
    case 0x16:
    case 0x26:{
        u8 register_index = ((cpu->instruction & 0xF0) >> 4) * 2;
        FetchNextInstructionByte(cpu);
        *cpu->register_map[register_index] = cpu->data_byte;


        return 7;
    }

    // MVI Instruction.  MVI M, D8  :  M <- byte 2
    case 0x36:{ 
        FetchNextInstructionByte(cpu);
        *cpu->M = cpu->data_byte;

        return 10;
    }

    // RLC Instruction.  RLC  :  A = A << 1; bit 0 = prev bit 7; CY = prev bit 7
    case 0x07:{
        u8 previous_bit_7 = (cpu->A & 0x80) >> 7;
        previous_bit_7 ? SetFlag(cpu, FLAG_CARRY) : UnSetFlag(cpu, FLAG_CARRY);
        cpu->A <<= 1;
        cpu->A = (cpu->A & (~(0x01))) | previous_bit_7;

        return 4;
    }

    // RAL Instruction.  RAL :   A = A << 1; bit 0 = prev CY; CY = prev bit 7
    case 0x17:{
        u8 previous_bit_7 = (cpu->A & 0x80) >> 7;
        
        cpu->A <<= 1;
        cpu->A = (cpu->A & (~(0x01))) | (cpu->flags & FLAG_CARRY);

        previous_bit_7 ? SetFlag(cpu, FLAG_CARRY) : UnSetFlag(cpu, FLAG_CARRY);

        return 4;
    }

    // DAA Instruction.  BCD adjust.
    case 0x27:{
        if((cpu->A & 0x0F) > 9 || cpu->flags & FLAG_AUXCARRY){
            cpu->A += 6;
            SetFlag(cpu, FLAG_AUXCARRY);
        } 
        else{
            UnSetFlag(cpu, FLAG_AUXCARRY);
        }

        if(cpu->A > 0x99 || cpu->flags & FLAG_CARRY) {
            cpu->A += 0x60;
            SetFlag(cpu, FLAG_CARRY);
        }
        else{
            UnSetFlag(cpu, FLAG_CARRY);
        }

        if(cpu->A == 0)     
            SetFlag(cpu, FLAG_ZERO);
        else
            UnSetFlag(cpu, FLAG_ZERO);

        if(cpu->A & 0x80)
            SetFlag(cpu, FLAG_SIGN);
        else
            UnSetFlag(cpu, FLAG_SIGN);

        if(cpu->A % 2 == 0 && cpu->A > 0)
            SetFlag(cpu, FLAG_PARITY);
        else
            UnSetFlag(cpu, FLAG_PARITY);

       

        return 4;        
    }

    // STC Instruction.  STC  :  CY  CY = 1
    case 0x37:{
        SetFlag(cpu, FLAG_CARRY);

        return 4;
    }


    // DAD Instruction.  DAD R :   HL = HL + RP
    case 0x09:
    case 0x19:
    case 0x29:
    case 0x39:{
        u8 register_pair_index = (cpu->instruction & 0xF0) >> 4;
        u16 previous_value = cpu->HL;
        cpu->HL += *cpu->wide_register_map[register_pair_index];

        if(previous_value > cpu->HL) 
            SetFlag(cpu, FLAG_CARRY);
        else
            UnSetFlag(cpu, FLAG_CARRY);

        return 10;
    }

    // LDAX Instrucion.  LDAX R  :  A <- (RP)
    case 0x0A:
    case 0x1A:{
        u8 register_pair_index = (cpu->instruction & 0xF0) >> 4;
        cpu->A = cpu->memory[*cpu->wide_register_map[register_pair_index]];

        return 7;
    }

    // LHLD Instruction. LHLD adr :  L <- (adr); H<-(adr+1)
    case 0x2A:{
        FetchNextInstructionByte(cpu);
        u8 low  = cpu->data_byte;
        FetchNextInstructionByte(cpu);
        u8 high = cpu->data_byte;
        u16 address = (high << 8) | low;

        cpu->L = cpu->memory[address];
        cpu->H = cpu->memory[address + 1];

        return 16;
    }

    // LDA Instruction.     LDA adr   :   A <- (adr)
    case 0x3A:{
        FetchNextInstructionByte(cpu);
        u8 low  = cpu->data_byte;
        FetchNextInstructionByte(cpu);
        u8 high = cpu->data_byte;
        u16 address = (high << 8) | low;

        cpu->A = cpu->memory[address];

        return 13;
    }
    

    // DCX Instruction.  DCX RP :  RP = RP-1
    case 0x0B:
    case 0x1B:
    case 0x2B:
    case 0x3B:{
        u8 register_pair_index = (cpu->instruction & 0xF0) >> 4;
        (*cpu->wide_register_map[register_pair_index])--;

        return 5;
    }


    // INR Instruction.  INR R  :  R <- R+1   Where R is an odd register.
    case 0x0C:
    case 0x1C:
    case 0x2C:
    case 0x3C:{
        u8 register_index = (((cpu->instruction & 0xF0) >> 4) * 2) + 1;
        *cpu->register_map[register_index] = SumAndSetFlags(cpu, *cpu->register_map[register_index], 1);

        return 5;
    }    


    // DCR Instruction.  DCR R  :  R <- R-1   Where R is an odd register.
    case 0x0D:
    case 0x1D:
    case 0x2D:
    case 0x3D:{
        u8 register_index = (((cpu->instruction & 0xF0) >> 4) * 2) + 1;
        *cpu->register_map[register_index] = SubstractAndSetFlags(cpu, *cpu->register_map[register_index], 1);

        return 5;
    }


    // MVI Instruction.  MVI R,D8  : R <- byte 2  Where R is an odd register.
    case 0x0E:
    case 0x1E:
    case 0x2E:
    case 0x3E:{
        u8 register_index = (((cpu->instruction & 0xF0) >> 4) * 2) + 1;
        FetchNextInstructionByte(cpu);
        *cpu->register_map[register_index] = cpu->data_byte;

        return 7;
    }


    // RRC Instruction.  RRC  :  A = A >> 1; bit 7 = prev bit 0; CY = prev bit 0
    case 0x0F:{
        u8 previous_bit_0 = (cpu->A & 0x01);
        previous_bit_0 ? SetFlag(cpu, FLAG_CARRY) : UnSetFlag(cpu, FLAG_CARRY);
        cpu->A >>= 1;
        cpu->A = (cpu->A & (~(0x80))) | (previous_bit_0 << 7);

        return 4;
    }


    // RAR Instruction.     RAR  :  A = A >> 1; bit 7 = prev CY 7; CY = prev bit 0
    case 0x1F:{
        u8 previous_bit_0 = (cpu->A & 0x01);
        
        cpu->A >>= 1;
        cpu->A = (cpu->A & (~(0x80))) | ((cpu->flags & FLAG_CARRY) << 7);

        previous_bit_0 ? SetFlag(cpu, FLAG_CARRY) : UnSetFlag(cpu, FLAG_CARRY);

        return 4;
    } 

    // CMA Instruction.  CMA   :   A <- !A
    case 0x2F:{
        cpu->A = ~cpu->A;

        return 4;
    }


    // CMC Instruction.   CMC  :  CY  CY=!CY
    case 0x3F:{
        SetFlag(cpu, FLAG_CARRY);
        (cpu->flags & FLAG_CARRY) ? UnSetFlag(cpu, FLAG_CARRY) : SetFlag(cpu, FLAG_CARRY);

        return 4;
    }


    

    default:{
        printf("Instruction: %X not implemented\n", cpu->instruction);
        return 0;   
    }

    }
}