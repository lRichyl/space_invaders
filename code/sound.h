#pragma once
#include "SDL_mixer.h"

struct SoundEffect{
    Mix_Chunk *sound;
    bool already_played;
};

struct SoundState{
    Mix_Chunk *ufo;
    Mix_Chunk *shot;
    Mix_Chunk *player_die;
    Mix_Chunk *invader_die;
    Mix_Chunk *fleet_1;
    Mix_Chunk *fleet_2;
    Mix_Chunk *fleet_3;
    Mix_Chunk *fleet_4;
    // Mix_Chunk *ufo_hit;
};

Mix_Chunk *LoadSound(const char *name, const char *path);
void InitSounds(SoundState *sound_state);
