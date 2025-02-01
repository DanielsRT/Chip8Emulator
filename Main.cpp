#include <iostream>
#include <chrono>
#include <thread>
#include "stdint.h"
#include "SDL3/SDL.h"


#include "chip8.h"

using namespace std;

// Keypad keymap
uint8_t keymap[16] = {
    SDLK_X,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_Q,
    SDLK_W,
    SDLK_E,
    SDLK_A,
    SDLK_S,
    SDLK_D,
    SDLK_Z,
    SDLK_C,
    SDLK_4,
    SDLK_R,
    SDLK_F,
    SDLK_V,
};

int main(int argc, char** argv) {

    // Command usage
    /*if (argc != 2) {
        cout << "Usage: chip8 <ROM file>" << endl;
        return 1;
    }*/

    Chip8 chip8 = Chip8();

    int windowWidth = 1024;
    int windowHeight = 512;

    SDL_Window* window;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }
    window = SDL_CreateWindow(
        "CHIP-8 Emulator",
		windowWidth, windowHeight, SDL_WINDOW_RESIZABLE
    );
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n",
            SDL_GetError());
        exit(2);
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    SDL_SetRenderLogicalPresentation(renderer, windowWidth, windowHeight, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    // Texture that stores frame buffer
    SDL_Texture* sdlTexture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        64, 32);

    // Temporary pixel buffer
    uint32_t pixels[2048];


    if (!chip8.LoadROM("PONG"))
        return 2;

    while (true) {
        chip8.Cycle();

        // Process SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) exit(0);

            // Process keydown events
            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE)
                    exit(0);

                for (int i = 0; i < 16; ++i) {
                    if (event.key.key == keymap[i]) {
                        chip8.keypad[i] = 1;
                    }
                }
            }
            // Process keyup events
            if (event.type == SDL_EVENT_KEY_UP) {
                for (int i = 0; i < 16; ++i) {
                    if (event.key.key == keymap[i]) {
                        chip8.keypad[i] = 0;
                    }
                }
            }
        }

        // If draw occurred, redraw SDL screen
        if (chip8.drawFlag) {
            chip8.drawFlag = false;

            // Store pixels in temporary buffer
            for (int i = 0; i < 2048; ++i) {
                uint8_t pixel = chip8.video[i];
                pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
            }
            // Update SDL texture
            SDL_UpdateTexture(sdlTexture, NULL, pixels, 64 * sizeof(Uint32));
            // Clear screen and render
            SDL_RenderClear(renderer);
            SDL_RenderTexture(renderer, sdlTexture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }

        // Sleep to slow down emulation speed
        std::this_thread::sleep_for(std::chrono::microseconds(1000));

    }
}