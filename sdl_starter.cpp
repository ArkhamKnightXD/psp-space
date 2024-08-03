#include "sdl_starter.h"

int startSDL(SDL_Window *window, SDL_Renderer *renderer) {

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0)
    {
        std::cout << "SDL crashed. Error: " << SDL_GetError();
        return 1;
    }

    if (window == nullptr)
    {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    if (renderer == nullptr)
    {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!IMG_Init(IMG_INIT_PNG))
    {
        std::cout << "SDL_image crashed. Error: " << SDL_GetError();
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return 1;
    }

    if (TTF_Init() == -1)
    {
        return 1;
    }

    return 0;
}