struct CPU {
    b32 is_initialized;
    s32 clock_speed;

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
    u8 *M; // Pseudo register that contains the dereferenced memory pointed by HL.
    b32 halt;

    u16 SP; // Stack pointer
    u16 PC; // Program counter


    u8 memory[Kilobytes(64)]; // 64 Kilobytes of memory.

    u16 *wide_register_map[4];
    u8  *register_map[8];

    b32 interrupts_enabled;

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

void PushToStack(CPU *cpu, i32 register_pair_index){
    assert(register_pair_index <= 3);

    if(register_pair_index == -1){  // Push the program counter to the stack.
        cpu->memory[cpu->SP - 1] = (u8)((cpu->PC & 0xFF00) >> 8);
        cpu->memory[cpu->SP - 2] = (u8)((cpu->PC & 0x00FF));
    }else if(register_pair_index <= 2){
        cpu->memory[cpu->SP - 1] = (u8)((*cpu->wide_register_map[register_pair_index] & 0xFF00) >> 8);
        cpu->memory[cpu->SP - 2] = (u8)((*cpu->wide_register_map[register_pair_index] & 0x00FF));
    }else if(register_pair_index == 3){
        cpu->memory[cpu->SP - 1] = cpu->A;
        cpu->memory[cpu->SP - 2] = cpu->flags;
    }

    cpu->SP -= 2;
}

void PopFromStack(CPU *cpu, i32 register_pair_index){
    u8 second;
    u8 first;

    assert(register_pair_index <= 4 && register_pair_index >= -1);

    if(register_pair_index == -1){ // Pop to program counter.  For RET instructions.
        second = cpu->memory[cpu->SP];
        first  = cpu->memory[cpu->SP + 1];

        cpu->PC = (first << 8) | second;
        cpu->PC++;
    }else if(register_pair_index <= 3){
        second = cpu->memory[cpu->SP];
        first  = cpu->memory[cpu->SP + 1];

        *cpu->wide_register_map[register_pair_index] = (first << 8) | second;
    }else if(register_pair_index == 4){
        cpu->flags = cpu->memory[cpu->SP];
        cpu->A = cpu->memory[cpu->SP + 1];
    }

    cpu->SP += 2;
}

internal bool CheckParityBits(u8 byte){
    i32 count = 0;

        // Count the set bits using Kernighan's Algorithm
        while (byte) {
            byte &= (byte - 1);
            count++;
        }

        // Check if the count of set bits is even
        return (count % 2 == 0);
}

internal void SetZeroSignParity(CPU *cpu, u8 value){
    if(value == 0)     
        SetFlag(cpu, FLAG_ZERO);
    else
        UnSetFlag(cpu, FLAG_ZERO);

    if(value & 0x80)
        SetFlag(cpu, FLAG_SIGN);
    else
        UnSetFlag(cpu, FLAG_SIGN);

    if(CheckParityBits((u8)value))
        SetFlag(cpu, FLAG_PARITY);
    else
        UnSetFlag(cpu, FLAG_PARITY);
}

internal u8 SumAndSetFlags(CPU *cpu, u8 summand_left, u8 summand_right, b32 check_carry = false){
    u16 result = (u16)summand_left + (u16)summand_right;
    
    SetZeroSignParity(cpu, (u8)result);

    u8 nibble_left_summand     = summand_left    & 0x0F;
    u8 nibble_right_summand    = summand_right   & 0x0F;
    if((nibble_left_summand + nibble_right_summand) & 0x10) 
        SetFlag(cpu, FLAG_AUXCARRY);
    else
        UnSetFlag(cpu, FLAG_AUXCARRY);

    if(check_carry){
        if(result & 0x100) 
            SetFlag(cpu, FLAG_CARRY);
        else
            UnSetFlag(cpu, FLAG_CARRY);
    }

    return (u8)result;
}

internal u8 SubstractAndSetFlags(CPU *cpu, u8 minuend, u8 sustrahend, b32 check_carry = false){
    u8 result = minuend - sustrahend;
    
    SetZeroSignParity(cpu, result);

    u8 nibble_minuend    = minuend    & 0x0F;
    u8 nibble_sustrahend = ((~sustrahend) & 0x0F) + 1;
    if((nibble_minuend + nibble_sustrahend) & 0x10) 
        SetFlag(cpu, FLAG_AUXCARRY);
    else
        UnSetFlag(cpu, FLAG_AUXCARRY);

    u16 result_carry = (u16)nibble_minuend + (u16)nibble_sustrahend;
    if(!(result_carry & 0x100)){
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

    if(cpu->halt) return 7;

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
            (cpu->flags & FLAG_CARRY) ? UnSetFlag(cpu, FLAG_CARRY) : SetFlag(cpu, FLAG_CARRY);

            return 4;
        }


        // RNZ instruction.  
        case 0xC0:{
            if(!(cpu->flags & FLAG_ZERO)){ // Return if not zero.
                PopFromStack(cpu, -1);
                return 11;
            }else{
                return 5;
            }
        }

        // RNC instruction.  
        case 0xD0:{
            if(!(cpu->flags & FLAG_CARRY)){ // Return if the carry is not set.
                PopFromStack(cpu, -1);
                return 11;
            }else{
                return 5;
            }
        }

        // RPO instruction.  
        case 0xE0:{
            if(!(cpu->flags & FLAG_PARITY)){ // Return if the parity is odd.
                PopFromStack(cpu, -1);
                return 11;
            }else{
                return 5;
            }
        }

        // RP instruction.
        case 0xF0:{
            if(!(cpu->flags & FLAG_SIGN)){ // Return if the sign flag is not set.
                PopFromStack(cpu, -1);
                return 11;
            }else{
                return 5;
            }
        }

        // Pop instructions.
        case 0xC1:
        case 0xD1:
        case 0xE1:
        case 0xF1:{
            u8 register_pair_index = (cpu->instruction & 0x30) >> 4;
            if(register_pair_index <= 2){
                PopFromStack(cpu, register_pair_index);
            }else if(register_pair_index == 3){
                PopFromStack(cpu, 4);
            }

            return 10;
        }

        // JNZ instruction. Jump if not zero.
        case 0xC2:{
            FetchNextInstructionByte(cpu);
            u8 low = cpu->data_byte;
            FetchNextInstructionByte(cpu);
            u8 high = cpu->data_byte;

            if(!(cpu->flags & FLAG_ZERO)){ 
                cpu->PC = (high << 8) | low;
            }
            return 10;
        }

        // JNC instruction. Jump if the carry bit is not set.
        case 0xD2:{
            FetchNextInstructionByte(cpu);
            u8 low = cpu->data_byte;
            FetchNextInstructionByte(cpu);
            u8 high = cpu->data_byte;

            if(!(cpu->flags & FLAG_CARRY)){ 
                cpu->PC = (high << 8) | low;
            }
            return 10;
        }

        // JPO instruction.   Jump if the parity is odd.
        case 0xE2:{
            FetchNextInstructionByte(cpu);
            u8 low = cpu->data_byte;
            FetchNextInstructionByte(cpu);
            u8 high = cpu->data_byte;

            if(!(cpu->flags & FLAG_PARITY)){ 
                cpu->PC = (high << 8) | low;
            }
            return 10;
        }

        // JP instruction.   Jump if the Sign flag is not set.
        case 0xF2:{
            FetchNextInstructionByte(cpu);
            u8 low = cpu->data_byte;
            FetchNextInstructionByte(cpu);
            u8 high = cpu->data_byte;

            if(!(cpu->flags & FLAG_SIGN)){ 
                cpu->PC = (high << 8) | low;
            }
            return 10;
        }

        // JMP instruction.   Jump unconditionally to the immediate address.
        case 0xC3:{
            FetchNextInstructionByte(cpu);
            u8 low = cpu->data_byte;
            FetchNextInstructionByte(cpu);
            u8 high = cpu->data_byte;

            cpu->PC = (high << 8) | low;

            return 10;
        }

        // // OUT instruction. Send the byte stored in the accumulator to device number [d8]. 
        // case 0xD3:{
                // @TODO: Implement.

        //     return 10;
        // }


        // XTHL instruction.  Exchange stack.
        case 0xE3:{
            u8 previous_L = cpu->L;
            cpu->L = cpu->memory[cpu->SP];
            cpu->memory[cpu->SP] = previous_L;

            u8 previous_H = cpu->H;
            cpu->H = cpu->memory[cpu->SP + 1];
            cpu->memory[cpu->SP + 1] = previous_H;

            return 18;
        }

        // DI instruction.   Disable interrupts.
        case 0xF3:{
            cpu->interrupts_enabled = false;  // @TODO: Implement disabling interrupts.

            return 4;
        }

        // CNZ instruction.  Go to subroutine if the zero bit is not set.
        case 0xC4:{
            FetchNextInstructionByte(cpu);
            u8 low = cpu->data_byte;
            FetchNextInstructionByte(cpu);
            u8 high = cpu->data_byte;

            if(!(cpu->flags & FLAG_ZERO)){ 
                PushToStack(cpu, -1);
                cpu->PC = (high << 8) | low;
                return 17;
            }
            return 11;

        }

        // CNC instruction.  Go to subroutine if the carry bit is not set.
        case 0xD4:{
            FetchNextInstructionByte(cpu);
            u8 low = cpu->data_byte;
            FetchNextInstructionByte(cpu);
            u8 high = cpu->data_byte;

            if(!(cpu->flags & FLAG_CARRY)){ 
                PushToStack(cpu, -1);
                cpu->PC = (high << 8) | low;
                return 17;
            }
            return 11;

        }

        // CPO instruction.  Go to subroutine if the parity is odd. (Parity bit is zero).
        case 0xE4:{
            FetchNextInstructionByte(cpu);
            u8 low = cpu->data_byte;
            FetchNextInstructionByte(cpu);
            u8 high = cpu->data_byte;

            if(!(cpu->flags & FLAG_PARITY)){ 
                PushToStack(cpu, -1);
                cpu->PC = (high << 8) | low;
                return 17;
            }
            return 11;

        }

        // CP instruction.  Go to subroutine if the sign is positive. (Sign bit is zero).
        case 0xE4:{
            FetchNextInstructionByte(cpu);
            u8 low = cpu->data_byte;
            FetchNextInstructionByte(cpu);
            u8 high = cpu->data_byte;

            if(!(cpu->flags & FLAG_SIGN)){ 
                PushToStack(cpu, -1);
                cpu->PC = (high << 8) | low;
                return 17;
            }
            return 11;

        }

        default:{
            // printf("Instruction: %X not implemented\n", cpu->instruction);
            break;   
        }
    }


    // MOV instructions.   MOV :  DR <- SR.   (DR: Destination register.   SR: Source register)
    switch(cpu->instruction & 0xF0){
        case 0x40:
        case 0x50:
        case 0x60:
        case 0x70:{
            u8 destination = (cpu->instruction & 0x38) >> 3; // The 4th to 6th bits encode the destination register.
            u8 source      = cpu->instruction & 0x07;        // The first 3LSB encode the source register.

            *cpu->register_map[destination] = *cpu->register_map[source];

            if(source == 0x06){ // Source register is register M
                if(((cpu->instruction & 0xF0) == 0x70)) cpu->halt = true;
                return 7; // Moving to the M register takes more cycles.
            } 
            return 5;
        }

        // ADD and ADC instructions.
        case 0x80:{
            u8 source = cpu->instruction & 0x07;

            // 4th bit encodes the instruction. 0 for ADD and 1 for ADC.
            if(!(cpu->instruction & 0x08)){ // ADD instruction.  ADD R  :  A <- A + R.   Affects all flags.
                cpu->A = SumAndSetFlags(cpu, cpu->A, *cpu->register_map[source], true);

            }else{ // ADC instruction.    ADC R  :  A <- A + R + CY
                cpu->A = SumAndSetFlags(cpu, cpu->A, *cpu->register_map[source] + (cpu->flags & FLAG_CARRY), true);
            }

            if(source == 0x06){ // Source register is register M
                return 7; 
            } 
            return 4;
        }

        // SUB and SBB instructions.
        case 0x90:{
            u8 source = cpu->instruction & 0x07;

            if(!(cpu->instruction & 0x08)){ // SUB instruction.  SUB R  :  A <- A - R   Affects all flags.
                cpu->A = SubstractAndSetFlags(cpu, cpu->A, *cpu->register_map[source], true);

            }else{ // SBB instruction.      SBB R  :  A <- A - R - CY
                cpu->A = SubstractAndSetFlags(cpu, cpu->A, *cpu->register_map[source] + (cpu->flags & FLAG_CARRY), true);
            }

            if(source == 0x06){ // Source register is register M
                return 7; 
            } 
            return 4;
        }

        // ANA and XRA instructions.
        case 0xA0:{
            u8 source = cpu->instruction & 0x07;

            if(!(cpu->instruction & 0x08)){ // ANA instruction.  ANA R  :  A <- A & R   Affects all flags.
                UnSetFlag(cpu, FLAG_CARRY);

                if((cpu->A & 0x08) | (*cpu->register_map[source]) & 0x08){ // Verify how the ANA instructions affects the half carry flag.
                    SetFlag(cpu, FLAG_AUXCARRY);
                }else{
                    UnSetFlag(cpu, FLAG_AUXCARRY);
                }

                cpu->A = cpu->A & *cpu->register_map[source];

                SetZeroSignParity(cpu, cpu->A);

            }else{ // XRA instruction.      XRA R  :  A <- A ^ R
                UnSetFlag(cpu, FLAG_CARRY);
                UnSetFlag(cpu, FLAG_AUXCARRY);

                cpu->A = cpu->A ^ *cpu->register_map[source];

                SetZeroSignParity(cpu, cpu->A);
            }

            if(source == 0x06){ // Source register is register M
                return 7; 
            } 
            return 4;
        }


        // ORA and CMP instructions.
        case 0xB0:{
            u8 source = cpu->instruction & 0x07;

            if(!(cpu->instruction & 0x08)){ // ORA instruction.   ORA R  :  A <- A | R
                UnSetFlag(cpu, FLAG_CARRY);
                UnSetFlag(cpu, FLAG_AUXCARRY);

                cpu->A = cpu->A | *cpu->register_map[source];

                SetZeroSignParity(cpu, cpu->A);

            }else{ // CMP instruction.  CMP R  :  A - R
                u8 difference = SubstractAndSetFlags(cpu, cpu->A, *cpu->register_map[source], true);
            }

            if(source == 0x06){ // Source register is register M
                return 7; 
            } 
            return 4;
        }

        default:
            break;
    }



}