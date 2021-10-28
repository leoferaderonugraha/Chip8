#include "Chip8.h"
#include <unistd.h>
#include "SDL2/SDL.h"
#include <chrono>   // sleep in microseconds
#include <thread>

#define SCREEN_SIZE 2048 // (64 * 32)

int main()
{
    Chip8 chip8 = Chip8();

    // Keypad keymap
    uint8_t keymap[16] = {
        SDLK_x,
        SDLK_1,
        SDLK_2,
        SDLK_3,
        SDLK_q,
        SDLK_w,
        SDLK_e,
        SDLK_a,
        SDLK_s,
        SDLK_d,
        SDLK_z,
        SDLK_c,
        SDLK_4,
        SDLK_r,
        SDLK_f,
        SDLK_v,
    };

    if (chip8.load("roms/TETRIS")) {

        int width = 1024;
        int height = 512;

        //* SDL window for rendering the pixels
        SDL_Window *window;

        //* Initialize SDL
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
            fprintf(stderr, "Fail to initalize, SDL Error: %s\n", SDL_GetError());
        }

        //* Create window
        window = SDL_CreateWindow(
            "Chip-8 Emulator",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            width,
            height,
            SDL_WINDOW_SHOWN
        );

        if (!window) {
            fprintf(stderr, "Could not create window: SDL_Error: %s\n", SDL_GetError());
        }

        // Create renderer
        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
        SDL_RenderSetLogicalSize(renderer, width, height);

        // Create texture that stores frame buffer
        SDL_Texture* sdlTexture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            64, 32
        );

        //* Temporary pixels buffer
        uint32_t pixels[SCREEN_SIZE];

        while (true) {
            chip8.emulateCycle();

            if (chip8.updateScreen) {
                chip8.updateScreen = false;

                // Process SDL events
                SDL_Event e;
                while (SDL_PollEvent(&e)) {
                    switch (e.type) {
                        case SDL_QUIT:
                            exit(0);
                            break;
                        case SDL_KEYDOWN: {
                            if (e.key.keysym.sym == SDLK_ESCAPE) {
                                exit(0);
                            }

                            //* Check if user pressed a key that in the keymap 
                            for (int i = 0; i < 16; i++) {
                                if (e.key.keysym.sym == keymap[i]) {
                                    chip8.key[i] = 1;
                                }
                            }
                            break;
                        }

                        case SDL_KEYUP: {
                            for (int i = 0; i < 16; i++) {
                                if (e.key.keysym.sym == keymap[i]) {
                                    chip8.key[i] = 0;
                                }
                            }
                            break;
                        }
                    }
                }

                // Store pixels in temporary buffer
                for (int i = 0; i < SCREEN_SIZE; ++i) {
                    uint8_t pixel = chip8.gfx[i];
                    pixels[i] = (0x00FFFFFF * (pixel)) | 0xFF000000;
                }

                // Update SDL texture
                SDL_UpdateTexture(sdlTexture, NULL, pixels, 64 * sizeof(Uint32));
                // Clear screen and render
                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
                SDL_RenderPresent(renderer);
            }

            // Sleep to slow down emulation speed
            // sleep(1);
            std::this_thread::sleep_for(std::chrono::microseconds(10000));
        }
    }
}