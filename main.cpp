#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>

const int SCREEN_WIDTH = 480;
const int SCREEN_HEIGHT = 272;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_GameController *controller = NULL;

Mix_Chunk *laserSound = nullptr;
Mix_Chunk *explosionSound = nullptr;
Mix_Music *music = nullptr;

TTF_Font *fontSquare = nullptr; 

SDL_Texture *scoreTexture = nullptr;
SDL_Rect scoreBounds;

SDL_Texture *liveTexture = nullptr;
SDL_Rect liveBounds;

SDL_Color fontColor = {255, 255, 255};

SDL_Texture *shipSprite = nullptr;
SDL_Texture *playerSprite = nullptr;
SDL_Texture *alienSprite1 = nullptr;
SDL_Texture *alienSprite2 = nullptr;
SDL_Texture *alienSprite3 = nullptr;
SDL_Texture *structureSprite = nullptr;

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
    SDL_Rect bounds;
    SDL_Texture *sprite;
    int lives;
    int speed;
    int score;
} Player;

Player player;

typedef struct
{
    SDL_Rect bounds;
    SDL_Texture *sprite;
    int points;
    int velocityX;
    bool shouldMove;
    bool isDestroyed;
} MysteryShip;

MysteryShip mysteryShip;

float lastTimeMysteryShipSpawn;

typedef struct
{
    SDL_Rect bounds;
    SDL_Texture *sprite;
    int lives;
    bool isDestroyed;
} Structure;

std::vector<Structure> structures;

SDL_Texture *loadSprite(const char *file)
{
    SDL_Texture *texture = IMG_LoadTexture(renderer, file);
    return texture;
}

typedef struct
{
    float x;
    float y;
    SDL_Rect bounds;
    SDL_Texture *sprite;
    int points;
    int velocity;
    bool isDestroyed;
} Alien;

std::vector<Alien> aliens;

bool shouldChangeVelocity = false;
bool shouldAliensGoDown = false;

std::vector<Alien> createAliens()
{
    alienSprite1 = loadSprite("alien_1.png");
    alienSprite2 = loadSprite("alien_2.png");
    alienSprite3 = loadSprite("alien_3.png");

    std::vector<Alien> aliens;

    int positionX;
    int positionY = 25;
    int alienPoints = 8;

    SDL_Texture *actualSprite;

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
            SDL_Rect alienBounds = {positionX, positionY, 20, 20};

            Alien actualAlien = {(float)positionX, (float)positionY, alienBounds, actualSprite, alienPoints, 50, false};

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
        alien.bounds.x = alien.x;

        float alienPosition = alien.bounds.x + alien.bounds.w;

        if ((!shouldChangeVelocity && alienPosition > SCREEN_WIDTH) || alienPosition < alien.bounds.w)
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
            alien.bounds.y += 5;
        }

        shouldChangeVelocity = false;
    }
}

Mix_Chunk *loadSound(const char *p_filePath)
{
    Mix_Chunk *sound = nullptr;

    sound = Mix_LoadWAV(p_filePath);
    if (sound == nullptr)
    {
        printf("Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError());
    }

    return sound;
}

Mix_Music *loadMusic(const char *p_filePath)
{
    Mix_Music *music = nullptr;

    music = Mix_LoadMUS(p_filePath);
    if (music == nullptr)
    {
        printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
    }

    return music;
}


void quitGame()
{
    SDL_DestroyTexture(shipSprite);
    SDL_DestroyTexture(playerSprite);
    SDL_DestroyTexture(structureSprite);
    SDL_DestroyTexture(alienSprite1);
    SDL_DestroyTexture(alienSprite2);
    SDL_DestroyTexture(alienSprite3);
    SDL_DestroyTexture(scoreTexture);
    SDL_DestroyTexture(liveTexture);
    
    // Close SDL_image
    IMG_Quit();

    Mix_FreeChunk(laserSound);
    Mix_FreeChunk(explosionSound);
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
    }
}

void checkCollisionBetweenStructureAndLaser(Laser &laser)
{
    for (Structure &structure : structures)
    {
        if (!structure.isDestroyed && SDL_HasIntersection(&structure.bounds, &laser.bounds))
        {
            laser.isDestroyed = true;

            structure.lives--;

            if (structure.lives == 0)
            {
                structure.isDestroyed = true;
            }

            Mix_VolumeChunk(explosionSound, MIX_MAX_VOLUME / 2);
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

void updateTextureText(SDL_Texture *&texture, const char *text) {

    if (fontSquare == nullptr) {
        printf("TTF_OpenFont fontSquare: %s\n", TTF_GetError());
    }

    SDL_Surface *surface = TTF_RenderUTF8_Blended(fontSquare, text, fontColor);
    if (surface == nullptr) {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Unable to create text surface! SDL Error: %s\n", SDL_GetError());
        exit(3);
    }

    SDL_DestroyTexture(texture);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr) {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Unable to create texture from surface! SDL Error: %s\n", SDL_GetError());
    }

    SDL_FreeSurface(surface);
}

void update(float deltaTime)
{
    SDL_GameControllerUpdate();

    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) && player.bounds.x > 0)
    {
        player.bounds.x -= player.speed * deltaTime;
    }

    else if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) && player.bounds.x < SCREEN_WIDTH - player.bounds.w)
    {
        player.bounds.x += player.speed * deltaTime;
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
        if (mysteryShip.bounds.x > SCREEN_WIDTH + mysteryShip.bounds.w || mysteryShip.bounds.x < -80)
        {
            mysteryShip.velocityX *= -1;
            mysteryShip.shouldMove = false;
        }

        mysteryShip.bounds.x += mysteryShip.velocityX * deltaTime;
    }

    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A))
    {
        lastTimePlayerShoot += deltaTime;

        if (lastTimePlayerShoot >= 0.35)
        {
            SDL_Rect laserBounds = {player.bounds.x + 10, player.bounds.y, 2, 8};

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

        if (!mysteryShip.isDestroyed && SDL_HasIntersection(&mysteryShip.bounds, &laser.bounds))
        {
            laser.isDestroyed = true;

            player.score += mysteryShip.points;

            std::string scoreString = "score: " + std::to_string(player.score);

            updateTextureText(scoreTexture, scoreString.c_str());

            mysteryShip.isDestroyed = true;
            
            Mix_VolumeChunk(explosionSound, MIX_MAX_VOLUME / 2);
            Mix_PlayChannel(-1, explosionSound, 0);

            break;
        }

        for (Alien &alien : aliens)
        {
            if (!alien.isDestroyed && SDL_HasIntersection(&alien.bounds, &laser.bounds))
            {
                alien.isDestroyed = true;
                laser.isDestroyed = true;

                player.score += alien.points;

                std::string scoreString = "score: " + std::to_string(player.score);

                updateTextureText(scoreTexture, scoreString.c_str());

                Mix_VolumeChunk(explosionSound, MIX_MAX_VOLUME / 2);
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

        SDL_Rect laserBounds = {alienShooter.bounds.x + 10, alienShooter.bounds.y + alienShooter.bounds.h, 2, 8};

        alienLasers.push_back({laserBounds, false});

        lastTimeAliensShoot = 0;

        Mix_PlayChannel(-1, laserSound, 0);
    }

        for (Laser &laser : alienLasers)
    {
        laser.bounds.y += 200 * deltaTime;

        if (laser.bounds.y > SCREEN_HEIGHT)
            laser.isDestroyed = true;

        if (player.lives > 0 && SDL_HasIntersection(&player.bounds, &laser.bounds))
        {
            laser.isDestroyed = true;

            player.lives--;

            std::string liveString = "lives: " + std::to_string(player.lives);

            updateTextureText(liveTexture, liveString.c_str());

            Mix_VolumeChunk(explosionSound, MIX_MAX_VOLUME / 2);
            Mix_PlayChannel(-1, explosionSound, 0);

            break;
        }

        checkCollisionBetweenStructureAndLaser(laser);
    }

    aliensMovement(deltaTime);

    removeDestroyedElements();
}

void renderSprite(SDL_Texture *sprite, SDL_Rect spriteBounds)
{
    SDL_QueryTexture(sprite, NULL, NULL, &spriteBounds.w, &spriteBounds.h);
    SDL_RenderCopy(renderer, sprite, NULL, &spriteBounds);
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

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    if (!mysteryShip.isDestroyed)
    {
        renderSprite(mysteryShip.sprite, mysteryShip.bounds);
    }

    for (Alien alien : aliens)
    {
        if (!alien.isDestroyed)
        {
            renderSprite(alien.sprite, alien.bounds);
        }
    }

    SDL_SetRenderDrawColor(renderer, 243, 216, 63, 255);

    for (Laser laser : alienLasers)
    {
        if (!laser.isDestroyed)
        {
            SDL_RenderFillRect(renderer, &laser.bounds);
        }
    }

    for (Laser laser : playerLasers)
    {
        if (!laser.isDestroyed)
        {
            SDL_RenderFillRect(renderer, &laser.bounds);
        }
    }

    for (Structure structure : structures)
    {
        if (!structure.isDestroyed)
        {
            renderSprite(structure.sprite, structure.bounds);
        }
    }

    renderSprite(player.sprite, player.bounds);

    SDL_RenderPresent(renderer);
}

int main(int argc, char *args[])
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0)
    {
        std::cout << "SDL crashed. Error: " << SDL_GetError();
        return 1;
    }

    window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr)
    {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr)
    {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (SDL_NumJoysticks() < 1) {
        printf("No game controllers connected!\n");
        return -1;
    } 
    else {

        controller = SDL_GameControllerOpen(0);
        if (controller == NULL) {

            printf("Unable to open game controller! SDL Error: %s\n", SDL_GetError());
            return -1;
        }
    }

    if (!IMG_Init(IMG_INIT_PNG))
    {
        std::cout << "SDL_image crashed. Error: " << SDL_GetError();
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
    }

    if (TTF_Init() == -1)
    {
        return 1;
    }

    fontSquare = TTF_OpenFont("square_sans_serif_7.ttf", 16);

    updateTextureText(scoreTexture, "Score: 0");
    updateTextureText(liveTexture, "Lives: 2");

    laserSound = loadSound("laser.wav");
    explosionSound = loadSound("explosion.wav");

    music = loadMusic("music.wav");
    
    //reduce music volume
    // Mix_VolumeMusic(MIX_MAX_VOLUME / 2);

    Mix_PlayMusic(music, -1);
    
    shipSprite = loadSprite("mystery.png");

    SDL_Rect shipBounds = {SCREEN_WIDTH, 20, 29, 13};

    mysteryShip = {shipBounds, shipSprite, 50, -75, false, false};

    aliens = createAliens();

    playerSprite = loadSprite("spaceship.png");

    SDL_Rect playerBounds = {SCREEN_WIDTH / 2, SCREEN_HEIGHT - 20, 22, 14};

    player = {playerBounds, playerSprite, 2, 300, 0};

    SDL_Rect structureBounds = {60, SCREEN_HEIGHT - 60, 28, 17};
    SDL_Rect structureBounds2 = {175, SCREEN_HEIGHT - 60, 28, 17};
    SDL_Rect structureBounds3 = {300, SCREEN_HEIGHT - 60, 28, 17};
    SDL_Rect structureBounds4 = {400, SCREEN_HEIGHT - 60, 28, 17};

    structureSprite = loadSprite("structure.png");

    structures.push_back({structureBounds, structureSprite, 5, false});
    structures.push_back({structureBounds2, structureSprite, 5, false});
    structures.push_back({structureBounds3, structureSprite, 5, false});
    structures.push_back({structureBounds4, structureSprite, 5, false});

    Uint32 previousFrameTime = SDL_GetTicks();
    Uint32 currentFrameTime = previousFrameTime;
    float deltaTime = 0.0f;

    srand(time(NULL));

    while (true)
    {
        currentFrameTime = SDL_GetTicks();

        deltaTime = (currentFrameTime - previousFrameTime) / 1000.0f;

        previousFrameTime = currentFrameTime;

        handleEvents();
        update(deltaTime);
        render();
    }

    quitGame();
    return 0;
}