#include <iostream>
#include "SDL.h"
#include "chip8.h"
#include <chrono>
#include <thread>


int main(int argc, char* argv[]) {
    // check for ROM file
    if (argc != 2) {
        std::cerr << "Usage: chip8 <ROM file>" << std::endl;
        return 1;
    }

    // init chip8 & load game
    Chip8 chip8;
    chip8.initialize();
    chip8.loadGame(argv[1]);

    // init SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Error: SDL could not initialize" << std::endl;
        return 1;
    }
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    
    // create window & renderer
    window = SDL_CreateWindow("Chip8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 320, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "Error: Window could not be created" << std::endl;
        return 1;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "Error: Renderer could not be created" << std::endl;
        return 1;
    }

    // map SDL keys to index
    uint8_t keypadMap[16] = {
        SDLK_0,
        SDLK_1,
        SDLK_2,
        SDLK_3,
        SDLK_4,
        SDLK_5,
        SDLK_6,
        SDLK_7,
        SDLK_8,
        SDLK_9,
        SDLK_a,
        SDLK_b,
        SDLK_c,
        SDLK_d,
        SDLK_e,
        SDLK_f
    };

    // emulate cycle
    while (true) {
        chip8.emulateCycle();

        // render if draw flag is set
        if (chip8.drawFlag) {
            // clear screen with black
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // black
            SDL_RenderClear(renderer);  

            // draw pixels if set in display
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);   // white
            for (int y = 0; y < 32; ++y) {
                for (int x = 0; x < 64; ++x) {
                    // because one dimensional array index = y * width + x
                    if (chip8.display[y * 64 + x]) {
                        SDL_Rect rect = { x * 10, y * 10, 10, 10 }; // scale up by 10
                        SDL_RenderFillRect(renderer, &rect);    // draw rectangle
                    }
                }
            }

            SDL_RenderPresent(renderer);    // update screen
            chip8.drawFlag = false; // reset draw flag
        }

        // get key press
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch(e.type) {
                case SDL_QUIT:
                    SDL_DestroyRenderer(renderer);
                    SDL_DestroyWindow(window);
                    SDL_Quit();
                    return 0;
                case SDL_KEYDOWN:
                    // match key press to keypad index, set to 1
                    for (uint8_t i = 0; i < 16; ++i) {
                        if (e.key.keysym.sym == keypadMap[i]) {
                            chip8.keypad[i] = 1;
                        }
                    }
                    break;
                case SDL_KEYUP:
                    // match key release to keypad index, set to 0
                    for (uint8_t i = 0; i < 16; ++i) {
                        if (e.key.keysym.sym == keypadMap[i]) {
                            chip8.keypad[i] = 0;
                        }
                    }
                    break;
            }       
        }
        std::this_thread::sleep_for(std::chrono::microseconds(1500));
    }

    return 0;
}