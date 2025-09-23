#include "Platform.h"

// Create constructor
Platform::Platform(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight) {
    SDL_Init(SDL_INIT_VIDEO);  // Initialise video
    window = SDL_CreateWindow(title, windowWidth, windowHeight,SDL_WINDOW_ALWAYS_ON_TOP); // Create the window
    renderer = SDL_CreateRenderer(window, NULL); // Create the renderer
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, textureWidth, textureHeight); // Create the texture
}

// Create destructor
Platform::~Platform() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}