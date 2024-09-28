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
                    switch(e.key.keysym.sym) {
                        case SDLK_1: chip8.keypad[0x1] = 1; break; // 1
                        case SDLK_2: chip8.keypad[0x2] = 1; break; // 2
                        case SDLK_3: chip8.keypad[0x3] = 1; break; // 3
                        case SDLK_4: chip8.keypad[0x4] = 1; break; // 4
                        case SDLK_5: chip8.keypad[0x5] = 1; break; // 5
                        case SDLK_6: chip8.keypad[0x6] = 1; break; // 6
                        case SDLK_7: chip8.keypad[0x7] = 1; break; // 7
                        case SDLK_8: chip8.keypad[0x8] = 1; break; // 8
                        case SDLK_9: chip8.keypad[0x9] = 1; break; // 9
                        case SDLK_0: chip8.keypad[0x0] = 1; break; // 0
                        case SDLK_a: chip8.keypad[0xA] = 1; break; // a
                        case SDLK_b: chip8.keypad[0xB] = 1; break; // b
                        case SDLK_c: chip8.keypad[0xC] = 1; break; // c
                        case SDLK_d: chip8.keypad[0xD] = 1; break; // d
                        case SDLK_e: chip8.keypad[0xE] = 1; break; // e
                        case SDLK_f: chip8.keypad[0xF] = 1; break; // f
                    }
                    break;
                case SDL_KEYUP:
                    switch(e.key.keysym.sym) {
                        case SDLK_1: chip8.keypad[0x1] = 0; break; // 1
                        case SDLK_2: chip8.keypad[0x2] = 0; break; // 2
                        case SDLK_3: chip8.keypad[0x3] = 0; break; // 3
                        case SDLK_4: chip8.keypad[0x4] = 0; break; // 4
                        case SDLK_5: chip8.keypad[0x5] = 0; break; // 5
                        case SDLK_6: chip8.keypad[0x6] = 0; break; // 6
                        case SDLK_7: chip8.keypad[0x7] = 0; break; // 7
                        case SDLK_8: chip8.keypad[0x8] = 0; break; // 8
                        case SDLK_9: chip8.keypad[0x9] = 0; break; // 9
                        case SDLK_0: chip8.keypad[0x0] = 0; break; // 0
                        case SDLK_a: chip8.keypad[0xA] = 0; break; // a
                        case SDLK_b: chip8.keypad[0xB] = 0; break; // b
                        case SDLK_c: chip8.keypad[0xC] = 0; break; // c
                        case SDLK_d: chip8.keypad[0xD] = 0; break; // d
                        case SDLK_e: chip8.keypad[0xE] = 0; break; // e
                        case SDLK_f: chip8.keypad[0xF] = 0; break; // f
                    }
                    break;
            }       
        }
        std::this_thread::sleep_for(std::chrono::microseconds(1500));
    }

    return 0;
}