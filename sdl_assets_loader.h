#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>

typedef struct
{
    SDL_Texture *texture;
    SDL_Rect textureBounds;
} Sprite;

Sprite loadSprite(SDL_Renderer *renderer, const char *filePath, int positionX, int positionY);

Mix_Chunk *loadSound(const char *filePath);

Mix_Music *loadMusic(const char *filePath);

void updateTextureText(SDL_Texture *&texture, const char *text, TTF_Font *&fontSquare, SDL_Renderer *renderer);