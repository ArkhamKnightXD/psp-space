#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>

const int SCREEN_WIDTH = 480;
const int SCREEN_HEIGHT = 272;

int startSDL(SDL_Window *window, SDL_Renderer *renderer);
