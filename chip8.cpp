#include "chip8.h"
#include <iostream>
#include <fstream>
#include <cstdlib>

unsigned char chip8_fontset[80] =
{ 
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8() {}

void Chip8::initialize() {
    pc = 0x200;  // program counter starts at 0x200
    opcode = 0;  // reset current opcode
    I = 0;       // reset index register
    sp = 0;      // reset stack pointer
    drawFlag = false; // reset draw flag

    // clear display
    for (int i = 0; i < 2048; ++i) {
        display[i] = 0;
    }
    // clear stack, registers, and keypad
    for (int i = 0; i < 16; ++i) {
        stack[i] = 0;
        keypad[i] = V[i] = 0;
    }
    // clear memory
    for (int i = 0; i < 4096; ++i) {
        memory[i] = 0;
    }

    // ;load fontset
    for (int i = 0; i < 80; ++i) {
        memory[i] = chip8_fontset[i];
    }

    // reset timers
    delay_timer = 0;
    sound_timer = 0;
}

void Chip8::loadGame(const char* filename) {
    // open the file in binary mode and at end to get size
    std::ifstream file(filename, std::ios::binary  | std::ios::ate);
    if (file.is_open()) {
        // get position in file (size) & check if it fits in memory
        int size = file.tellg();
        if (size > 4096 - 512) {
            std::cerr << "Error: ROM too large to fit in memory" << std::endl;
            exit(1);
        }

        // create buffer & store
        char* buffer = new char[size];
        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();

        // load game into memory, starting at 0x200 (512)
        for (int i = 0; i < size; ++i) {
            memory[i + 0x200] = buffer[i];
        }
        // free buffer
        delete[] buffer;
    }
    else {
        std::cerr << "Error: Unable to open file" << std::endl;
        exit(1);
    }
}

void Chip8::emulateCycle() {
    // fetch 16 bit opcode --- opcodes are 2 bytes long so bit shift left 8 first then bitwise OR with next byte
    opcode = memory[pc] << 8 | memory[pc + 1];
    pc += 2;

    // decode opcode
    switch (opcode & 0xf000) {
        case 0x0000: {
            switch(opcode & 0x0fff) {
                // 00e0: clear the screen
                case 0x00e0: {
                    for (int i = 0; i < 2048; ++i) {
                        display[i] = 0;
                    }
                    drawFlag = true;
                    break;
                }
                // 00ee: return --- set pc to return address saved in stack
                case 0x00ee: {
                    --sp;   // return address stored in stack level below current 
                    pc = stack[sp];
                    break;
                }
                case 0x0000: break;
                default: {
                    std::cerr << "Unknown opcode: " << opcode << std::endl;
                    exit(1);
                }
            }
            break;
        }
        case 0x1000: {
            // 1nnn: jump to address nnn
            pc = opcode & 0x0fff;
            break;
        }
        case 0x2000: {
            // 2nnn: go to subroutine at nnn
            stack[sp] = pc; // store return address in stack
            ++sp;   // increment stack pointer to clear for subroutine
            pc = opcode & 0x0fff;
            break;
        }
        case 0x3000: {
            // 3xkk: skip next instruction if Vx == kk
            if (V[(opcode & 0x0f00) >> 8] == (opcode & 0x00ff)) {   
                pc += 2;
            }
            break;
        }
        case 0x4000: {
            // 4xkk: skip next instruction if Vx != kk
            if (V[(opcode & 0x0f00) >> 8] != (opcode & 0x00ff)) {
                pc += 2;
            }
            break;
        }
        case 0x5000: {
            // 5xy0: skip next instruction if Vx == Vy
            if (V[(opcode & 0x0f00) >> 8] == V[(opcode & 0x00f0) >> 4]) {
                pc += 2;
            }
            break;
        }
        case 0x6000: {
            // 6xkk: Vx = kk
            V[(opcode & 0x0f00) >> 8] = opcode & 0x00ff;
            break;
        }
        case 0x7000: {
            // 7xkk: Vx = Vx + kk
            V[(opcode & 0x0f00) >> 8] += opcode & 0x00ff;
            break;
        }
        case 0x8000: {
            uint8_t x = (opcode & 0x0f00) >> 8;
            uint8_t y = (opcode & 0x00f0) >> 4;
            switch(opcode & 0x000f) {
                // 0x8xy0: Vx = Vy
                case 0x0000: {
                    V[x] = V[y];
                    break;
                }
                // 0x8xy1: Vx |= Vy
                case 0x0001: {
                    V[x] |= V[y];
                    break;
                }
                // 0x8xy2: Vx &= Vy
                case 0x0002: {
                    V[x] &= V[y];
                    break;
                }
                // 0x8xy3: Vx ^= Vy
                case 0x0003: {
                    V[x] ^= V[y];
                    break;
                }
                // 0x8xy4: Vx += Vy, VF = carry
                case 0x0004: {
                    V[0xf] = (V[x] + V[y] > 255) ? 1 : 0;
                    V[x] += V[y];
                    break;
                }
                // 0x8xy5: Vx -= Vy, VF = !borrow
                case 0x0005: {
                    V[0xf] = (V[x] > V[y]) ? 1 : 0;
                    V[x] -= V[y];
                    break;
                }
                // 0x8xy6: Vx >>= 1, VF = lsb of Vx
                case 0x0006: {
                    V[0xf] = V[x] & 0x1;
                    V[x] >>= 1;
                    break;
                }
                // 0x8xy7: Vx = Vy - Vx, VF = !borrow
                case 0x0007: {
                    V[0xf] = (V[y] > V[x]) ? 1 : 0;
                    V[x] = V[y] - V[x];
                    break;
                }
                // 0x8xye: Vx <<= 1, VF = msb of Vx
                case 0x000e: {
                    V[0xf] = V[x] >> 7;
                    V[x] <<= 1;
                    break;
                }
                default: {
                    std::cerr << "Unknown opcode: " << opcode << std::endl;
                    exit(1);
                }
            }
            break;
        }
        case 0x9000: {
            // 9xy0: skip next instruction if Vx != Vy
            if (V[(opcode & 0x0f00) >> 8] != V[(opcode & 0x00f0) >> 4]) {
                pc += 2;
            }
            break;
        }
        case 0xa000: {
            // annn: set I = nnn
            I = opcode & 0x0fff;
            break;
        }
        case 0xb000: {
            // bnnn: jump to location nnn + V0
            pc = (opcode & 0x0fff) + V[0];
            break;
        }
        case 0xc000: {
            // cxkk: Vx = random byte & kk
            V[(opcode & 0x0f00) >> 8] = (rand() % 256) & (opcode & 0x00ff);
            break;
        }
        case 0xd000: {
            // dxyn: draw sprite at (Vx, Vy) with width 8 and height n
            uint8_t x = V[(opcode & 0x0f00) >> 8];
            uint8_t y = V[(opcode & 0x00f0) >> 4];
            uint8_t height = opcode & 0x000f;
            uint8_t pixel;

            // set VF = 0 --- collision detection
            V[0xf] = 0;

            // set each row of 8 pixels
            for (int yline = 0; yline < height; ++yline) {
                pixel = memory[I + yline];  // each row is byte starting at I
                for (int xline = 0; xline < 8; xline++) {
                    // check if pixel is set by anding with 0x80 (1000 0000) shifted right by xline to get the current pixel bit
                    if ((pixel & (0x80 >> xline))) {
                        uint16_t index = x + xline + ((y + yline) * 64);    // location of pixel on display
                        // pixel already set => collision detected
                        if (display[index]) {
                            V[0xf] = 1;
                        }
                        // xor pixel with display to set pixel if it was unset or unset if it was set
                        display[index] ^= 1;
                    }
                }
            }
            drawFlag = true;    // set draw flag to update screen
            break;
        }
        case 0xe000: {
            switch(opcode & 0x00ff) {
                // ex9e: skip next instruction if key with value Vx is pressed
                case 0x009e: {
                    if (keypad[V[(opcode & 0x0f00) >> 8]]) {
                        pc += 2;
                    }
                    break;
                }
                // exa1: skip next instruction if key with value Vx is not pressed
                case 0x00a1: {
                    if (!keypad[V[(opcode & 0x0f00) >> 8]]) {
                        pc += 2;
                    }
                    break;
                }
                default: {
                    std::cerr << "Unknown opcode: " << opcode << std::endl;
                    exit(1);
                }
            }
            break;
        }
        case 0xf000: {
            switch(opcode & 0x00ff) {
                // fx07: Vx = delay timer
                case 0x0007: {
                    V[(opcode & 0x0f00) >> 8] = delay_timer;
                    break;
                }
                // fx0a: wait for key press and store value in Vx
                case 0x000a: {
                    bool keyPress = false;
                    for (int i = 0; i < 16; ++i) {
                        if (keypad[i]) {
                            V[(opcode & 0x0f00) >> 8] = i;
                            keyPress = true;
                        }
                    }
                    if (!keyPress) {
                        return;
                    }
                    break;
                }
                // fx15: delay timer = Vx
                case 0x0015: {
                    delay_timer = V[(opcode & 0x0f00) >> 8];
                    break;
                }
                // fx18: sound timer = Vx
                case 0x0018: {
                    sound_timer = V[(opcode & 0x0f00) >> 8];
                    break;
                }
                // fx1e: I += Vx
                case 0x001e: {
                    I += V[(opcode & 0x0f00) >> 8];
                    break;
                }
                // fx29: set I to location of sprite for digit Vx
                case 0x0029: {
                    I = V[(opcode & 0x0f00) >> 8] * 5;    // multiply by 5 since each sprite is 5 bytes long
                    break;
                }
                // fx33: store decimal representation of Vx with hundredths, tenths, ones in memory locations I, I+1, I+2
                case 0x0033: {
                    uint8_t Vx = V[(opcode & 0x0f00) >> 8];
                    memory[I] = Vx / 100;
                    memory[I + 1] = (Vx / 10) % 10;
                    memory[I + 2] = Vx % 10;
                    break;
                }
                // fx55: store V0 to Vx in memory starting at location I
                case 0x0055: {
                    for (int i = 0; i <= ((opcode & 0x0f00) >> 8); ++i) {
                        memory[I + i] = V[i];
                    }
                    break;
                }
                // fx65: fill V0 to Vx with values from memory starting at location I
                case 0x0065: {
                    for (int i = 0; i <= ((opcode & 0x0f00) >> 8); ++i) {
                        V[i] = memory[I + i];
                    }
                    break;
                }
                default: {
                    std::cerr << "Unknown opcode: " << opcode << std::endl;
                    exit(1);
                }
            }
            break;
        }
        default: {
            std::cerr << "Unknown opcode: " << opcode << std::endl;
            exit(1);
        }
    }

    // update timers
    if (delay_timer > 0) {
        --delay_timer;
    }
    if (sound_timer > 0) {
        if (sound_timer == 1) {
            // make sound
        }
        --sound_timer;
    }
}