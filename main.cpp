#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include "sdl_starter.h"
#include "sdl_assets_loader.h"

bool isGamePaused;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_GameController *controller = NULL;

Mix_Chunk *laserSound = nullptr;
Mix_Chunk *explosionSound = nullptr;
Mix_Chunk *pauseSound = nullptr;
Mix_Music *music = nullptr;

TTF_Font *fontSquare = nullptr; 

SDL_Texture *scoreTexture = nullptr;
SDL_Rect scoreBounds;

SDL_Texture *liveTexture = nullptr;
SDL_Rect liveBounds;

SDL_Texture *pauseTexture = nullptr;
SDL_Rect pauseBounds;

Sprite shipSprite;
Sprite playerSprite;
Sprite alienSprite1;
Sprite alienSprite2;
Sprite alienSprite3;
Sprite structureSprite;

typedef struct
{
    SDL_Rect bounds;
    bool isDestroyed;
} Laser;

std::vector<Laser> playerLasers;
std::vector<Laser> alienLasers;

float lastTimePlayerShoot;
float lastTimeAliensShoot;

typedef struct
{
    Sprite sprite;
    int lives;
    int speed;
    int score;
} Player;

Player player;

typedef struct
{
    float x;
    Sprite sprite;
    int points;
    int velocityX;
    bool shouldMove;
    bool isDestroyed;
} MysteryShip;

MysteryShip mysteryShip;

float lastTimeMysteryShipSpawn;

typedef struct
{
    Sprite sprite;
    int lives;
    bool isDestroyed;
} Structure;

std::vector<Structure> structures;

typedef struct
{
    float x;
    Sprite sprite;
    int points;
    int velocity;
    bool isDestroyed;
} Alien;

std::vector<Alien> aliens;

bool shouldChangeVelocity = false;

std::vector<Alien> createAliens()
{
    alienSprite1 = loadSprite(renderer, "alien_1.png", 0, 0);
    alienSprite2 = loadSprite(renderer, "alien_2.png", 0, 0);
    alienSprite3 = loadSprite(renderer, "alien_3.png", 0, 0);

    std::vector<Alien> aliens;

    //5*11 Aliens
    aliens.reserve(55);

    int positionX;
    int positionY = 25;
    int alienPoints = 8;

    Sprite actualSprite;

    for (int row = 0; row < 5; row++)
    {
        positionX = 75;

        switch (row)
        {
            case 0:
                actualSprite = alienSprite3;
                break;

            case 1:
            case 2:
                actualSprite = alienSprite2;
                break;

            default:
                actualSprite = alienSprite1;
        }

        for (int columns = 0; columns < 11; columns++)
        {
            actualSprite.textureBounds.x = positionX;
            actualSprite.textureBounds.y = positionY;

            Alien actualAlien = {(float)positionX, actualSprite, alienPoints, 50, false};

            aliens.push_back(actualAlien);
            positionX += 30;
        }

        alienPoints--;
        positionY += 25;
    }

    return aliens;
}

void aliensMovement(float deltaTime)
{
    for (Alien &alien : aliens)
    {
        alien.x += alien.velocity * deltaTime;
        alien.sprite.textureBounds.x = alien.x;

        float alienPosition = alien.sprite.textureBounds.x + alien.sprite.textureBounds.w;

        if ((!shouldChangeVelocity && alienPosition > SCREEN_WIDTH) || alienPosition < alien.sprite.textureBounds.w)
        {
            shouldChangeVelocity = true;
            break;
        }
    }

    if (shouldChangeVelocity)
    {
        for (Alien &alien : aliens)
        {
            alien.velocity *= -1;
            alien.sprite.textureBounds.y += 5;
        }

        shouldChangeVelocity = false;
    }
}

void quitGame()
{
    SDL_DestroyTexture(shipSprite.texture);
    SDL_DestroyTexture(playerSprite.texture);
    SDL_DestroyTexture(structureSprite.texture);
    SDL_DestroyTexture(alienSprite1.texture);
    SDL_DestroyTexture(alienSprite2.texture);
    SDL_DestroyTexture(alienSprite3.texture);
    SDL_DestroyTexture(scoreTexture);
    SDL_DestroyTexture(liveTexture);
    SDL_DestroyTexture(pauseTexture);

    IMG_Quit();

    Mix_FreeChunk(laserSound);
    Mix_FreeChunk(explosionSound);
    Mix_FreeChunk(pauseSound);
    Mix_FreeMusic(music);

    Mix_CloseAudio();
    Mix_Quit();

    TTF_Quit();

    SDL_GameControllerClose(controller);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void handleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            quitGame();
            exit(0);
        }

        if (event.type == SDL_CONTROLLERBUTTONDOWN && event.cbutton.button == SDL_CONTROLLER_BUTTON_START)
        {
            isGamePaused = !isGamePaused;
            Mix_PlayChannel(-1, pauseSound, 0);
        }
    }
}

void checkCollisionBetweenStructureAndLaser(Laser &laser)
{
    for (Structure &structure : structures)
    {
        if (!structure.isDestroyed && SDL_HasIntersection(&structure.sprite.textureBounds, &laser.bounds))
        {
            laser.isDestroyed = true;

            structure.lives--;

            if (structure.lives == 0)
            {
                structure.isDestroyed = true;
            }

            Mix_PlayChannel(-1, explosionSound, 0);

            break;
        }
    }
}

void removeDestroyedElements()
{
    for (auto iterator = aliens.begin(); iterator != aliens.end();)
    {
        if (iterator->isDestroyed)
        {
            aliens.erase(iterator);
        }
        else
        {
            iterator++;
        }
    }

    for (auto iterator = playerLasers.begin(); iterator != playerLasers.end();)
    {
        if (iterator->isDestroyed)
        {
            playerLasers.erase(iterator);
        }
        else
        {
            iterator++;
        }
    }

    for (auto iterator = alienLasers.begin(); iterator != alienLasers.end();)
    {
        if (iterator->isDestroyed)
        {
            alienLasers.erase(iterator);
        }
        else
        {
            iterator++;
        }
    }
}

void update(float deltaTime)
{
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) && player.sprite.textureBounds.x > 0)
    {
        player.sprite.textureBounds.x -= player.speed * deltaTime;
    }

    else if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) && player.sprite.textureBounds.x < SCREEN_WIDTH - player.sprite.textureBounds.w)
    {
        player.sprite.textureBounds.x += player.speed * deltaTime;
    }

    if (!mysteryShip.shouldMove)
    {
        lastTimeMysteryShipSpawn += deltaTime;

        if (lastTimeMysteryShipSpawn >= 10)
        {
            lastTimeMysteryShipSpawn = 0;

            mysteryShip.shouldMove = true;
        }
    }

    if (mysteryShip.shouldMove)
    {
        if (mysteryShip.sprite.textureBounds.x > SCREEN_WIDTH + mysteryShip.sprite.textureBounds.w || mysteryShip.sprite.textureBounds.x < -80)
        {
            mysteryShip.velocityX *= -1;
            mysteryShip.shouldMove = false;
        }

        mysteryShip.x += mysteryShip.velocityX * deltaTime;
        mysteryShip.sprite.textureBounds.x = mysteryShip.x;
    }

    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A))
    {
        lastTimePlayerShoot += deltaTime;

        if (lastTimePlayerShoot >= 0.35)
        {
            SDL_Rect laserBounds = {player.sprite.textureBounds.x + 10, player.sprite.textureBounds.y - player.sprite.textureBounds.h, 2, 8};

            playerLasers.push_back({laserBounds, false});

            lastTimePlayerShoot = 0;

            Mix_PlayChannel(-1, laserSound, 0);
        }
    }

    for (Laser &laser : playerLasers)
    {
        laser.bounds.y -= 200 * deltaTime;

        if (laser.bounds.y < 0)
            laser.isDestroyed = true;

        if (!mysteryShip.isDestroyed && SDL_HasIntersection(&mysteryShip.sprite.textureBounds, &laser.bounds))
        {
            laser.isDestroyed = true;

            player.score += mysteryShip.points;

            std::string scoreString = "score: " + std::to_string(player.score);

            updateTextureText(scoreTexture, scoreString.c_str(), fontSquare, renderer);

            mysteryShip.isDestroyed = true;

            Mix_PlayChannel(-1, explosionSound, 0);

            break;
        }

        for (Alien &alien : aliens)
        {
            if (!alien.isDestroyed && SDL_HasIntersection(&alien.sprite.textureBounds, &laser.bounds))
            {
                alien.isDestroyed = true;
                laser.isDestroyed = true;

                player.score += alien.points;

                std::string scoreString = "score: " + std::to_string(player.score);

                updateTextureText(scoreTexture, scoreString.c_str(), fontSquare, renderer);

                Mix_PlayChannel(-1, explosionSound, 0);

                break;
            }
        }

        checkCollisionBetweenStructureAndLaser(laser);
    }

    lastTimeAliensShoot += deltaTime;

    if (aliens.size() > 0 && lastTimeAliensShoot >= 0.6)
    {
        int randomAlienIndex = rand() % aliens.size();

        Alien alienShooter = aliens[randomAlienIndex];

        SDL_Rect laserBounds = {alienShooter.sprite.textureBounds.x + 10, alienShooter.sprite.textureBounds.y + alienShooter.sprite.textureBounds.h, 2, 8};

        alienLasers.push_back({laserBounds, false});

        lastTimeAliensShoot = 0;

        Mix_PlayChannel(-1, laserSound, 0);
    }

    for (Laser &laser : alienLasers)
    {
        laser.bounds.y += 200 * deltaTime;

        if (laser.bounds.y > SCREEN_HEIGHT)
            laser.isDestroyed = true;

        if (player.lives > 0 && SDL_HasIntersection(&player.sprite.textureBounds, &laser.bounds))
        {
            laser.isDestroyed = true;

            player.lives--;

            std::string liveString = "lives: " + std::to_string(player.lives);

            updateTextureText(liveTexture, liveString.c_str(), fontSquare, renderer);

            Mix_PlayChannel(-1, explosionSound, 0);

            break;
        }

        checkCollisionBetweenStructureAndLaser(laser);
    }

    aliensMovement(deltaTime);

    removeDestroyedElements();
}

void renderSprite(Sprite &sprite)
{
    SDL_RenderCopy(renderer, sprite.texture, NULL, &sprite.textureBounds);
}

void render()
{
    SDL_SetRenderDrawColor(renderer, 29, 29, 27, 255);
    SDL_RenderClear(renderer);

    SDL_QueryTexture(scoreTexture, NULL, NULL, &scoreBounds.w, &scoreBounds.h);
    scoreBounds.x = 100;
    scoreBounds.y = scoreBounds.h / 2;
    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreBounds);

    SDL_QueryTexture(liveTexture, NULL, NULL, &liveBounds.w, &liveBounds.h);
    liveBounds.x = 300;
    liveBounds.y = liveBounds.h / 2;
    SDL_RenderCopy(renderer, liveTexture, NULL, &liveBounds);

    if (isGamePaused)
    {
        SDL_RenderCopy(renderer, pauseTexture, NULL, &pauseBounds);
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    if (!mysteryShip.isDestroyed)
    {
        renderSprite(mysteryShip.sprite);
    }

    for (Alien &alien : aliens)
    {
        if (!alien.isDestroyed)
        {
            renderSprite(alien.sprite);
        }
    }

    SDL_SetRenderDrawColor(renderer, 243, 216, 63, 255);

    for (Laser &laser : alienLasers)
    {
        if (!laser.isDestroyed)
        {
            SDL_RenderFillRect(renderer, &laser.bounds);
        }
    }

    for (Laser &laser : playerLasers)
    {
        if (!laser.isDestroyed)
        {
            SDL_RenderFillRect(renderer, &laser.bounds);
        }
    }

    for (Structure &structure : structures)
    {
        if (!structure.isDestroyed)
        {
            renderSprite(structure.sprite);
        }
    }

    renderSprite(player.sprite);

    SDL_RenderPresent(renderer);
}

int main(int argc, char *args[])
{
    window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
   
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
 
    if(startSDL(window, renderer) > 0) 
    {
        return 1;
    }

    if (SDL_NumJoysticks() < 1) 
    {
        printf("No game controllers connected!\n");
        return -1;
    } 
    else 
    {
        controller = SDL_GameControllerOpen(0);
        if (controller == NULL)
        {
            printf("Unable to open game controller! SDL Error: %s\n", SDL_GetError());
            return -1;
        }
    }

    fontSquare = TTF_OpenFont("square_sans_serif_7.ttf", 16);

    updateTextureText(scoreTexture, "Score: 0", fontSquare, renderer);
    updateTextureText(liveTexture, "Lives: 2", fontSquare, renderer);    
    updateTextureText(pauseTexture, "Game Paused", fontSquare, renderer);

    SDL_QueryTexture(pauseTexture, NULL, NULL, &pauseBounds.w, &pauseBounds.h);
    pauseBounds.x = 175;
    pauseBounds.y = pauseBounds.h / 2 + 10;

    laserSound = loadSound("laser.wav");
    pauseSound = loadSound("magic.wav");
    explosionSound = loadSound("explosion.wav");

    Mix_VolumeChunk(explosionSound, MIX_MAX_VOLUME / 2);

    music = loadMusic("music.wav");
    
    //reduce music volume
    // Mix_VolumeMusic(MIX_MAX_VOLUME / 2);

    Mix_PlayMusic(music, -1);
    
    shipSprite = loadSprite(renderer, "mystery.png", SCREEN_WIDTH, 20);

    mysteryShip = {SCREEN_WIDTH, shipSprite, 50, -100, false, false};

    aliens = createAliens();

    playerSprite = loadSprite(renderer, "spaceship.png", SCREEN_WIDTH / 2, SCREEN_HEIGHT - 20);

    player = {playerSprite, 2, 300, 0};

    SDL_Rect structureBounds = {60, SCREEN_HEIGHT - 60, 28, 17};
    SDL_Rect structureBounds2 = {175, SCREEN_HEIGHT - 60, 28, 17};
    SDL_Rect structureBounds3 = {300, SCREEN_HEIGHT - 60, 28, 17};
    SDL_Rect structureBounds4 = {400, SCREEN_HEIGHT - 60, 28, 17};

    structureSprite = loadSprite(renderer, "structure.png", 60, SCREEN_HEIGHT - 60);

    structures.push_back({{structureSprite.texture, structureBounds}, 5, false});
    structures.push_back({{structureSprite.texture, structureBounds2}, 5, false});
    structures.push_back({{structureSprite.texture, structureBounds3}, 5, false});
    structures.push_back({{structureSprite.texture, structureBounds4}, 5, false});

    Uint32 previousFrameTime = SDL_GetTicks();
    Uint32 currentFrameTime = previousFrameTime;
    float deltaTime = 0.0f;

    srand(time(NULL));

    while (true)
    {
        currentFrameTime = SDL_GetTicks();
        deltaTime = (currentFrameTime - previousFrameTime) / 1000.0f;
        previousFrameTime = currentFrameTime;

        SDL_GameControllerUpdate();

        handleEvents();

        if(!isGamePaused) 
        {
            update(deltaTime);
        }
        
        render();
    }

    quitGame();
}