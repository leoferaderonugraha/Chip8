#include "Chip8.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

bool Chip8::load(const char *path)
{
    FILE *rom = fopen(path, "rb");

    if (!rom) {
        fprintf(stderr, "Fail to load the file\n");
        return false;
    }

    printf("\nLoad: %s\n", path);

    fseek(rom, 0, SEEK_END);
    fileSize = ftell(rom);
    rewind(rom);

    printf("File size: %.2lfKiB\n\n", (float)fileSize / 1024);

    fread(&mem[0x200], fileSize, sizeof(uint8_t), rom);
    fclose(rom);

    return true;
}

void Chip8::emulateCycle()
{
    uint16_t opcode = (mem[pc] << 8) | mem[pc + 1]; // Instruction is 2 bytes each

    //! Temporary
    // if (pc >= fileSize) {
    //     exit(0);
    // }
    x   = (opcode & 0x0F00) >> 8;   // Shift 8 bit
    y   = (opcode & 0x00F0) >> 4;   // Shift 4 bit
    kk  =  opcode & 0x00FF;
    nnn =  opcode & 0x0FFF;

    srand(time(0));

    switch (opcode & 0xF000) {
        //* 00E-
        case 0x0000:
            switch(opcode) {
                case 0x00E0:
                    //* Clear the display.
                    memset(gfx, 0, 64 * 32);
                    updateScreen = true;
                    pc += 2;
                    printf("CLS\n");
                    break;
                case 0x00EE:
                    //* Return from a subroutine.
                    // The interpreter sets the program counter to the address at the top of the stack,
                    // then subtracts 1 from the stack pointer.
                    pc = stack[--sp];
                    pc += 2;
                    printf("RET\n");
                    break;
                //* 0nnn
                default:
                    //* Jump to a machine code routine at nnn.
                    //! Ignored by modern interpreter
                    printf("SYS \t 0x%.4X (ignored)\n", nnn);
                    // exit(0);
                    pc += 2;
                    break;
            }
            break;
        //* 1nnn
        case 0x1000:
            //* Jump to location nnn.
            //  The interpreter sets the program counter to nnn.
            pc = nnn;
            printf("JP \t 0x%X\n", nnn);
            break;
        //* 2nnn
        case 0x2000:
            //* Call subroutine at nnn.
            //  The interpreter increments the stack pointer,
            //  then puts the current PC on the top of the stack.
            //  The PC is then set to nnn.
            stack[sp++] = pc;
            pc = nnn;
            printf("CALL \t 0x%.4X\n", nnn);
            break;
        //* 3xkk
        case 0x3000:
            //* Skip next instruction if Vx = kk.
            //  The interpreter compares register Vx to kk,
            //  and if they are equal, increments the program counter by 2.
            if (V[x] == kk) {
                pc += 2;
            }
            pc += 2;
            printf("SE \t V%d, 0x%X\n", x, kk);
            break;
        //* 4xkk
        case 0x4000:
            //* Skip next instruction if Vx != kk.
            //  The interpreter compares register Vx to kk,
            //  and if they are not equal, increments the program counter by 2.
            if (V[x] != kk) {
                pc += 2;
            }
            pc += 2;
            printf("SNE \t V%d, 0x%X\n", x, kk);
            break;
        //* 5xy0
        case 0x5000:
            //* Skip next instruction if Vx = Vy.
            //  The interpreter compares register Vx to register Vy,
            //  and if they are equal, increments the program counter by 2.
            if (V[x] == V[y]) {
                pc += 2;
            }
            pc += 2;
            printf("SE \t V%d, V%d\n", x, y);
            break;
        //* 6xkk
        case 0x6000:
            //* Set Vx = kk.
            //  The interpreter puts the value kk into register Vx.
            V[x] = kk;
            pc += 2;
            printf("LD \t V%d, 0x%X\n", x, kk);
            break;
        //* 7xkk
        case 0x7000:
            //* Set Vx = Vx + kk.
            //  Adds the value kk to the value of register Vx, then stores the result in Vx. 
            V[x] += kk;
            pc += 2;
            printf("ADD \t V%d, 0x%X\n", x, kk);
            break;
        //* 8xy-
        case 0x8000:
            switch (opcode & 0x000F) {
                case 0:
                    //* Set Vx = Vy.
                    //  Stores the value of register Vy in register Vx.
                    V[x] = V[y];
                    pc += 2;
                    printf("LD \t V%d, V%d\n", x, y);
                    break;
                case 1:
                    //* Set Vx = Vx OR Vy.
                    //  Performs a bitwise OR on the values of Vx and Vy,
                    //  then stores the result in Vx.
                    V[x] |= V[y];
                    pc += 2;
                    printf("OR \t V%d, V%d\n", x, y);
                    break;
                case 2:
                    //* Set Vx = Vx AND Vy.
                    //  Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx.
                    V[x] &= V[y];
                    pc += 2;
                    printf("AND \t V%d, V%d\n", x, y);
                    break;
                case 3:
                    //* Set Vx = Vx XOR Vy.
                    //  Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx.
                    V[x] ^= V[y];
                    pc += 2;
                    printf("XOR \t V%d, V%d\n", x, y);
                    break;
                case 4:
                    //* Set Vx = Vx + Vy, set VF = carry.
                    //  The values of Vx and Vy are added together.
                    //  If the result is greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise 0.
                    //  Only the lowest 8 bits of the result are kept, and stored in Vx.
                    V[x] += V[y];
                    V[0xF] = V[y] > (0xFF - V[x]);
                    pc += 2;
                    printf("ADD \t V%d, V%d\n", x, y);
                    break;
                case 5:
                    //* Set Vx = Vx - Vy, set VF = NOT borrow.
                    //  If Vx > Vy, then VF is set to 1, otherwise 0.
                    //  Then Vy is subtracted from Vx, and the results stored in Vx.
                    V[0xF] = V[x] > V[y];
                    V[x] -= V[y];
                    pc += 2;
                    printf("SUB \t V%d, V%d\n", x, y);
                    break;
                case 6:
                    //* Set Vx = Vx SHR 1.
                    //  If the least-significant bit of Vx is 1,
                    //  then VF is set to 1, otherwise 0. Then Vx is divided by 2.
                    V[0xF] = V[x] & 0x0001;
                    V[x] >>= 1;
                    pc += 2;
                    printf("SHR \t V%d {, V%d}\n", x, y);
                    break;
                case 7:
                    //* Set Vx = Vy - Vx, set VF = NOT borrow.
                    //  If Vy > Vx, then VF is set to 1, otherwise 0.
                    //  Then Vx is subtracted from Vy, and the results stored in Vx.
                    V[0xF] = V[y] > V[x];
                    V[x] = V[y] - V[x];
                    pc += 2;
                    printf("SUBN \t V%d, V%d\n", x, y);
                    break;
                case 0xE:
                    //* Set Vx = Vx SHL 1.
                    //  If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0.
                    //  Then Vx is multiplied by 2.
                    V[0xF] = V[x] >> 7;
                    V[x] <<= 1;
                    pc += 2;
                    printf("SHL \t V%d {, V%d}\n", x, y);
                    break;
            }
            break;
        case 0x9000:
            //* Skip next instruction if Vx != Vy.
            //  The values of Vx and Vy are compared, and if they are not equal,
            //  the program counter is increased by 2.
            if (V[x] != V[y]) {
                pc += 2;
            }
            pc += 2;
            printf("SNE \t V%d, V%d\n", x, y);
            break;
        case 0xA000:
            //* Set I = nnn.
            //  The value of register I is set to nnn.
            I = nnn;
            pc += 2;
            printf("LD \t I, 0x%.4X\n", nnn);
            break;
        case 0xB000:
            //* Jump to location nnn + V0.
            //  The program counter is set to nnn plus the value of V0.
            pc = nnn + V[0];
            printf("JP \t V0, 0x%.4X\n", nnn);
            break;
        case 0xC000:
            //* Set Vx = random byte AND kk.
            //  The interpreter generates a random number from 0 to 255, which is then ANDed with the value kk.
            //  The results are stored in Vx.
            V[x] = (rand() % 255) & kk;
            pc += 2;
            printf("RND \t V%d, 0x%X\n", x, kk);
            break;
        case 0xD000:
        {
            //* Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
            //  The interpreter reads n bytes from memory, starting at the address stored in I.
            //  These bytes are then displayed as sprites on screen at coordinates (Vx, Vy).
            //  Sprites are XORed onto the existing screen.
            //  If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0.
            //  If the sprite is positioned so part of it is outside the coordinates of the display,
            //  it wraps around to the opposite side of the screen.
            unsigned short x = V[(opcode & 0x0F00) >> 8];
            unsigned short y = V[(opcode & 0x00F0) >> 4];
            unsigned short height = opcode & 0x000F;
            unsigned short pixel;

            V[0xF] = 0;
            for (int yline = 0; yline < height; yline++)
            {
                pixel = mem[I + yline];
                for(int xline = 0; xline < 8; xline++)
                {
                    if((pixel & (0x80 >> xline)) != 0)
                    {
                        if(gfx[(x + xline + ((y + yline) * 64))] == 1)
                        {
                            V[0xF] = 1;
                        }
                        gfx[x + xline + ((y + yline) * 64)] ^= 1;
                    }
                }
            }

            updateScreen = true;
            pc += 2;
            printf("DRW \t V%d, V%d, 0x%X\n", x, y, opcode & 0x000F);
            break;
        }
        //* Ex--
        case 0xE000:
            switch (opcode & 0x00FF) {
                case 0x9E:
                    //* Skip next instruction if key with the value of Vx is pressed.
                    //  Checks the keyboard, and if the key corresponding to the value of
                    //  Vx is currently in the down position, PC is increased by 2.
                    if (key[V[x]]) {
                        pc += 2;
                    }
                    pc += 2;
                    printf("SKP \t V%d\n", x);
                    break;
                case 0xA1:
                    //* Skip next instruction if key with the value of Vx is not pressed.
                    //  Checks the keyboard, and if the key corresponding to the value of
                    //  Vx is currently in the up position, PC is increased by 2.
                    if (!key[V[x]]) {
                        pc += 2;
                    }
                    pc += 2;
                    printf("SKNP \t V%d\n", x);
                    break;
            }
            break;
        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x07:
                    //* Set Vx = delay timer value.
                    V[x] = delayTimer;
                    pc += 2;
                    printf("LD \t V%d, DT\n", x);
                    break;
                case 0x0A:
                {
                    //* Wait for a key press, store the value of the key in Vx.
                    //  All execution stops until a key is pressed,
                    //  then the value of that key is stored in Vx.
                    bool key_pressed = false;

                    //* Wait for all key to be released
                    for (int i = 0; i < 16; i++) {
                        //* Key still pressed
                        if (!key[i]) {
                            V[x] = i;
                            key_pressed = true;
                        }
                    }
                    
                    //* If no key pressed, try again
                    if (!key_pressed) {
                        return;
                    }

                    pc += 2;
                    printf("LD \t V%d, K\n", x);
                    break;
                }
                case 0x15:
                    //* Set delay timer = Vx.
                    //  DT is set equal to the value of Vx.
                    delayTimer = V[x];
                    pc += 2;
                    printf("LD \t DT, V%d\n", x);
                    break;
                case 0x18:
                    //* Set sound timer = Vx.
                    //  ST is set equal to the value of Vx.
                    soundTimer = V[x];
                    pc += 2;
                    printf("LD \t ST, V%d\n", x);
                    break;
                case 0x1E:
                    //* Set I = I + Vx.
                    //  The values of I and Vx are added, and the results are stored in I.
                    if(I + V[x] > 0xFFF)
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;
                    I += V[x];
                    pc += 2;
                    printf("ADD \t I, V%d\n", x);
                    break;
                case 0x29:
                    //* Set I = location of sprite for digit Vx.
                    //  The value of I is set to the location for
                    //  the hexadecimal sprite corresponding to the value of Vx.
                    I = V[x] * 0x5;
                    pc += 2;
                    printf("LD \t F, V%d\n", x);
                    break;
                case 0x33:
                    //* Store BCD representation of Vx in memory locations I, I+1, and I+2.
                    //  The interpreter takes the decimal value of Vx,
                    //  and places the hundreds digit in memory at location in I,
                    //  the tens digit at location I+1, and the ones digit at location I+2.
                    mem[I]      = (V[x] / 100) % 10;
                    mem[I + 1]  = (V[x] / 10) % 10;
                    mem[I + 2]  = (V[x] / 1) % 10;
                    pc += 2;
                    printf("LD \t B, V%d\n", x);
                    break;
                case 0x55:
                    //* Store registers V0 through Vx in memory starting at location I.
                    //  The interpreter copies the values of
                    //  registers V0 through Vx into memory, starting at the address in I.
                    for (size_t i = 0; i <= x; i++) {
                        mem[I + i] = V[i];
                    }
                    pc += 2;
                    printf("LD \t [I], V%d\n", x);
                    break;
                case 0x65:
                    //* Read registers V0 through Vx from memory starting at location I.
                    //  The interpreter reads values from memory starting at location I into registers V0 through Vx.
                    for (size_t i = 0; i <= x; i++) {
                        V[i] = mem[I + i];
                    }
                    pc += 2;
                    printf("LD \t V%d, [I]\n", x);
                    break;
            }
            break;
        default:
            printf("Invalid\n");
            break;
    }

    printf("Opcode: 0x%.4X\n", opcode);
    printf("Stack : ");
    for (auto addr : stack) {
        printf("0x%.4X ", addr);
    }
    putchar('\n');

    printf("V : ");
    for (auto v : V) {
        printf("%d ", v);
    }
    putchar('\n');
    printf("I : 0x%.4X\n", I);

    // printf("GFX :\n");
    // for (auto pixel : gfx) {
    //     printf("%d ", pixel);
    // }
    putchar('\n');
}
