#include <stdio.h>

#include "sound.h"


Mix_Chunk *LoadSound(const char *name, const char *path){
    Mix_Chunk *sound = Mix_LoadWAV(path);
    if(!sound){
        printf("Failed to load sound effect: %s\t SDL_mixer Error: %s\n", name, Mix_GetError());
    }
    return sound;
}

void InitSounds(SoundState *sound_state){
    sound_state->ufo         = LoadSound("UFO", "data/ufo_lowpitch.wav");
    sound_state->shot        = LoadSound("Shot", "data/shoot.wav");
    sound_state->player_die  = LoadSound("Player die", "data/explosion.wav");
    sound_state->invader_die = LoadSound("Invader killed", "data/invaderkilled.wav");
    sound_state->fleet_1     = LoadSound("Fleet 1", "data/fastinvader1.wav");
    sound_state->fleet_2     = LoadSound("Fleet 2", "data/fastinvader2.wav");
    sound_state->fleet_3     = LoadSound("Fleet 3", "data/fastinvader3.wav");
    sound_state->fleet_4     = LoadSound("Fleet 4", "data/fastinvader4.wav");
}
