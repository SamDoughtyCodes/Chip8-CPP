#ifndef PLATFORM_H
#define PLATFORM_H
#include <SDL3/SDL.h>

class Platform {
    public:
        // Attributes
        SDL_Window* window;  // Pointer to the sdl window
        SDL_Renderer* renderer; // Pointer to the sdl renderer
        SDL_Texture* texture; // Pointer to the sdl texture

        // Methods
    	Platform(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight);
        ~Platform();  // Destructor
        void Update(void const* buffer, int pitch);  // Update the display
        bool ProcessInput(uint8_t* keys);  // You guessed it, process some input!
};

#endif