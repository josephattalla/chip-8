#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>

class Chip8 {
    private:
        uint8_t memory[4096];
        uint8_t V[16];
        uint16_t I;
        uint16_t pc;
        uint16_t stack[16];
        uint8_t sp;
        uint8_t delay_timer;
        uint8_t sound_timer;
        uint16_t opcode;
    
    public:
        uint8_t keypad[16];
        uint8_t display[64 * 32];
        bool drawFlag;
        Chip8();
        void initialize();
        void loadGame(const char* filename);
        void emulateCycle();
};

#endif