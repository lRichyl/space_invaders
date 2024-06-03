#pragma once
#include "common.h"

#define INSTRUCTION_COUNT 256

struct InstructionInfo{
    i32 byte_size;
    const char *name;
};

void InitInstructionsInfo(InstructionInfo *instructions_info);