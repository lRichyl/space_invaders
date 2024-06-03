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
    sound_state->ufo         = LoadSound("UFO", "ufo_lowpitch.wav");
    sound_state->shot        = LoadSound("Shot", "shoot.wav");
    sound_state->player_die  = LoadSound("Player die", "explosion.wav");
    sound_state->invader_die = LoadSound("Invader killed", "invaderkilled.wav");
    sound_state->fleet_1     = LoadSound("Fleet 1", "fastinvader1.wav");
    sound_state->fleet_2     = LoadSound("Fleet 2", "fastinvader2.wav");
    sound_state->fleet_3     = LoadSound("Fleet 3", "fastinvader3.wav");
    sound_state->fleet_4     = LoadSound("Fleet 4", "fastinvader4.wav");
}
