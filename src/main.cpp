#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <pspdebug.h>
#include <pspkernel.h>

PSP_MODULE_INFO("SDL-Starter", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
 
int exitCallback(int arg1, int arg2, void* common) {

    sceKernelExitGame();
    return 0;
}

int callbackThread(SceSize args, void* argp) {

    int cbid = sceKernelCreateCallback("Exit Callback", exitCallback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();

    return 0;
}

int setupCallbacks(void) {

    int thid = sceKernelCreateThread("update_thread", callbackThread, 0x11, 0xFA0, 0, 0);
    if (thid >= 0) {
        sceKernelStartThread(thid, 0, 0);
    }

    return thid;
}

enum {
  SCREEN_WIDTH  = 480,
  SCREEN_HEIGHT = 272
};

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_GameController* controller = NULL;
SDL_Texture* sprite;
SDL_Rect spriteBounds = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 64, 64};

bool shouldRenderSprite = true;

const int SPEED = 200; 

void quitGame() {

    SDL_GameControllerClose(controller);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void handleEvents() {

    SDL_Event event;

    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_QUIT) {
            
            quitGame();
            exit(0);
        }
    }
}

SDL_Texture* loadSprite(const char* file, SDL_Renderer* renderer) {

    SDL_Texture* texture = IMG_LoadTexture(renderer, file);
    // if (texture == nullptr) {

    //     pspDebugScreenPrintf("Failed to create texture: %s\n", SDL_GetError());
    //     sceKernelDelayThread(2 * 1000 * 1000);
    //     return nullptr;
    // }

    // pspDebugScreenPrintf("Loaded image: %s\n", file);
    // sceKernelDelayThread(2 * 1000 * 1000);

    return texture;
}

void renderSprite(SDL_Texture* sprite, SDL_Renderer* renderer, SDL_Rect spriteBounds) {

    SDL_QueryTexture(sprite, NULL, NULL, &spriteBounds.w, &spriteBounds.h);
    SDL_RenderCopy(renderer, sprite, NULL, &spriteBounds);
}

void update(float deltaTime) {

    SDL_GameControllerUpdate();

    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP) && spriteBounds.y > 0) {
        spriteBounds.y -= SPEED * deltaTime;
    }

    else if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN) && spriteBounds.y < SCREEN_HEIGHT - spriteBounds.h) {
        spriteBounds.y += SPEED * deltaTime;
    }

    else if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) && spriteBounds.x > 0) {
        spriteBounds.x -= SPEED * deltaTime;
    }

    else if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) && spriteBounds.x < SCREEN_WIDTH - spriteBounds.w) {
        spriteBounds.x += SPEED * deltaTime;
    }
}

void render() {

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    renderSprite(sprite, renderer, spriteBounds);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    SDL_RenderPresent(renderer);  
}

int main() {

    setupCallbacks();
    pspDebugScreenInit();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        return -1;
    }

    if ((window = SDL_CreateWindow("SDL-STARTER", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0)) == NULL) {
        return -1;
    }

    if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)) == NULL) {
        return -1;
    }

    if (SDL_NumJoysticks() < 1) {
        pspDebugScreenPrintf("no game controller");
        return -1;
    } 
    
    else {

        controller = SDL_GameControllerOpen(0);
        if (controller == NULL) {

            pspDebugScreenPrintf("unable to open game controller");
            return -1;
        }
    }

    sprite = loadSprite("sprites/sprite.png", renderer);

    Uint32 previousFrameTime = SDL_GetTicks();
    Uint32 currentFrameTime;
    float deltaTime;

    while (true) {

        currentFrameTime = SDL_GetTicks();
        deltaTime = (currentFrameTime - previousFrameTime) / 1000.0f;
        previousFrameTime = currentFrameTime;

        handleEvents();
        update(deltaTime);
        render();
    }

    quitGame();
}