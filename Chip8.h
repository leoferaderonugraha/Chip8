#ifndef _CHIP_8_H
#define _CHIP_8_H

#include <cstdint>
#include <cstddef>
#include "SDL2/SDL.h"

#define START_LOCATION 0x200

class Chip8 {
private:
    uint8_t     mem[4096];      // 4 KB of RAM
    uint8_t     V[16];         // 16 general purpose 8-bit registers
    uint16_t    I;              // Register for store memory address

    uint8_t     delayTimer;     //* The delay timer is active whenever the delay timer register (DT) is non-zero.
                                //* This timer does nothing more than subtract 1 from the value of DT at a rate of 60Hz.
                                //* When DT reaches 0, it deactivates.

    uint8_t     soundTimer;     //* The sound timer is active whenever the sound timer register (ST) is non-zero.
                                //* This timer also decrements at a rate of 60Hz, however, as long as ST's value
                                //* is greater than zero, the Chip-8 buzzer will sound.
                                //* When ST reaches zero, the sound timer deactivates.

    uint16_t    pc = 0x200;     // Program counter
    uint8_t     sp;             // Stack pointer
    uint16_t    stack[16];      // Stack

    uint8_t     sprites[8 * 15] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0,   // 0
        0x20, 0x60, 0x20, 0x20, 0x70,   // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0,   // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0,   // 3
        0x90, 0x90, 0xF0, 0x10, 0x10,   // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0,   // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0,   // 6
        0xF0, 0x10, 0x20, 0x40, 0x40,   // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0,   // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0,   // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90,   // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0,   // B
        0xF0, 0x80, 0x80, 0x80, 0xF0,   // C
        0xE0, 0x90, 0x90, 0x90, 0xE0,   // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0,   // E
        0xF0, 0x80, 0xF0, 0x80, 0x80    // F
    };

    //* For debugging purpose
    uint8_t x;
    uint8_t y;
    uint8_t kk;
    uint16_t nnn;
public:
    uint8_t gfx[64 * 32];       // Graphics buffer
    bool updateScreen = false;  // Indicator for update the screen
    uint8_t key[16];            //* Keymap

    //* For debugging purpose
    size_t fileSize;

    bool load(const char *path);
    void emulateCycle();
};

#endif // _CHIP_8_H
