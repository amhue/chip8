/**
 * @file chip8.c
 * @author Aritra Mondal
 * @copyright 2024
 */

#include "config.h"
#include "timer.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// #define DEBUG_MODE

#ifdef DEBUG_MODE
#define DBG(...)                                                               \
    {                                                                          \
        printf(__VA_ARGS__);                                                   \
        printf("\n");                                                          \
    }
#else
#define DBG(...)
#endif

static struct chip8 {
    uint16_t pc; // program counter
    uint16_t i; // index register
    uint16_t stack[16];
    uint8_t sp; // stack pointer register
    uint8_t v[16]; // general purpose registers
    uint8_t carry; // carry flag
} cpu;

static struct opcode {
    uint16_t inst; // full opcode
    uint8_t cat; // top 4 bit (category)
    uint16_t nnn; // lowest 12 bit
    uint8_t n; // lowest 4 bit
    uint8_t x; // lower 4 bit of higher byte
    uint8_t y; // upper 4 bit of lower byte
    uint8_t kk; // lower byte
} op;

struct timer {
    uint8_t dt; // delay timer
    uint8_t st; // sound timer
};

extern struct timer timer;

uint8_t memory[4096];
bool draw_flag = false;

uint8_t fontset[80] = {
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
    0xF0, 0x80, 0xF0, 0x80, 0x80 // F
};

struct config config;

uint8_t display[64][32] = { 0 }; // 64 x 32 pixel display

bool key[16];

static inline uint8_t rnd() { return rand() % 256; }

void draw(uint8_t x, uint8_t y, uint8_t n)
{
    cpu.v[0xF] = 0;
    uint8_t byte = 0;
    uint8_t pixel = 0;

    uint8_t x_coord = x % config.scr_width;
    uint8_t y_coord = y % config.scr_height;

    for (uint8_t i = 0; i < n; i++) {
        if (y_coord + i >= config.scr_height)
            break;

        byte = memory[cpu.i + i];

        for (int j = 0; j < 8; j++) {
            if (x_coord + j >= config.scr_width)
                break;

            pixel = (byte & (0x80 >> j)) >> (7 - j);

            if (pixel && display[j + x_coord][i + y_coord])
                cpu.v[0xF] = 1;

            display[(j + x) % config.scr_width][(i + y) % config.scr_height]
                ^= pixel;
        }
    }
}

void init_sys()
{
    srand(time(NULL));
    cpu.pc = 0x200;
    timer.dt = 0;
    timer.st = 0;
    cpu.sp = 0;
    cpu.i = 0;
    memset(&(cpu.stack[0]), 0, 16);
    memcpy(&memory[0], fontset, 80);
}

void fetch()
{
    op.inst = (memory[cpu.pc] << 8) | memory[cpu.pc + 1];
    op.cat = op.inst >> 12;
    op.nnn = op.inst & 0xFFF;
    op.n = op.inst & 0xF;
    op.x = (op.inst & 0xF00) >> 8;
    op.y = (op.inst & 0xF0) >> 4;
    op.kk = op.inst & 0xFF;

    DBG("pc: 0x%04X, op: 0x%04X, cat: %X", cpu.pc, op.inst, op.cat);

    // Register states

    DBG("+--------+---------+---------+--------+");
    DBG("|V0 = %02X | V1 = %02X | V2 = %02X | V3 = %02X|", cpu.v[0x0],
        cpu.v[0x1], cpu.v[0x2], cpu.v[0x3]);
    DBG("|V4 = %02X | V5 = %02X | V6 = %02X | V7 = %02X|", cpu.v[0x4],
        cpu.v[0x5], cpu.v[0x6], cpu.v[0x7]);
    DBG("|V8 = %02X | V9 = %02X | VA = %02X | VB = %02X|", cpu.v[0x8],
        cpu.v[0x9], cpu.v[0xA], cpu.v[0xB]);
    DBG("|VC = %02X | VD = %02X | VE = %02X | VF = %02X|", cpu.v[0xC],
        cpu.v[0xD], cpu.v[0xE], cpu.v[0xF]);
    DBG("+--------+---------+---------+--------+");

    DBG("I = %03X, SP = %X", cpu.i, cpu.sp);
}

void decode_exec()
{
    switch (op.cat) {
    case 0x0:
        switch (op.kk) {
        case 0xE0:
            DBG("CLS");
            memset(display, 0, config.scr_height * config.scr_width);
            draw_flag = true;
            cpu.pc += 2;
            break;

        case 0xEE:
            DBG("RET");
            cpu.pc = cpu.stack[--cpu.sp];
            break;

        default:
            fprintf(stderr, "Unknown opcode %04X!\n", op.inst);
            break;
        }

        break;

    case 0x1:
        DBG("JP %03X", op.nnn);
        cpu.pc = op.nnn;
        break;

    case 0x2:
        DBG("CALL %03X", op.nnn);
        cpu.stack[cpu.sp++] = cpu.pc + 2;
        cpu.pc = op.nnn;
        break;

    case 0x3:
        DBG("SE V%X, %02X", op.x, op.kk);
        cpu.pc += cpu.v[op.x] == op.kk ? 4 : 2;
        break;

    case 0x4:
        DBG("SNE V%X, %02X", op.x, op.kk);
        cpu.pc += cpu.v[op.x] != op.kk ? 4 : 2;
        break;

    case 0x5:
        DBG("SE V%X, V%0X", op.x, op.y);
        cpu.pc += cpu.v[op.x] == cpu.v[op.y] ? 4 : 2;
        break;

    case 0x6:
        DBG("LD V%X, %02X", op.x, op.kk);
        cpu.v[op.x] = op.kk;
        cpu.pc += 2;
        break;

    case 0x7:
        DBG("ADD V%X, %02X", op.x, op.kk);
        cpu.v[op.x] += op.kk;
        cpu.pc += 2;
        break;

    case 0x8:
        switch (op.n) {
        case 0x0:
            DBG("LD V%X, V%X", op.x, op.y);
            cpu.v[op.x] = cpu.v[op.y];
            cpu.pc += 2;
            break;

        case 0x1:
            DBG("OR V%X, V%X", op.x, op.y);
            cpu.v[op.x] |= cpu.v[op.y];
            cpu.v[0xF] = 0;
            cpu.pc += 2;
            break;

        case 0x2:
            DBG("AND V%X, V%X", op.x, op.y);
            cpu.v[op.x] &= cpu.v[op.y];
            cpu.v[0xF] = 0;
            cpu.pc += 2;
            break;

        case 0x3:
            DBG("XOR V%X, V%X", op.x, op.y);
            cpu.v[op.x] ^= cpu.v[op.y];
            cpu.v[0xF] = 0;
            cpu.pc += 2;
            break;

        case 0x4:
            DBG("ADD V%X, V%X", op.x, op.y);
            cpu.carry = (int)(cpu.v[op.x]) + (int)(cpu.v[op.y]) > 255;
            cpu.v[op.x] += cpu.v[op.y];
            cpu.v[0xF] = cpu.carry;
            cpu.pc += 2;
            break;

        case 0x5:
            DBG("SUB V%X, V%X", op.x, op.y);
            cpu.carry = cpu.v[op.x] >= cpu.v[op.y];
            cpu.v[op.x] -= cpu.v[op.y];
            cpu.v[0xF] = cpu.carry;
            cpu.pc += 2;
            break;

        case 0x6:
            DBG("SHR V%X, V%X", op.x, op.y);
            cpu.carry = cpu.v[op.y] & 1;
            cpu.v[op.y] >>= 1;
            cpu.v[op.x] = cpu.v[op.y];
            cpu.v[0xF] = cpu.carry;
            cpu.pc += 2;
            break;

        case 0x7:
            DBG("SUBN V%X, V%X", op.x, op.y);
            cpu.carry = cpu.v[op.x] <= cpu.v[op.y];
            cpu.v[op.x] = cpu.v[op.y] - cpu.v[op.x];
            cpu.v[0xF] = cpu.carry;
            cpu.pc += 2;
            break;

        case 0xE:
            DBG("SHL V%X, V%X", op.x, op.y);
            cpu.carry = (cpu.v[op.y] & 0x80) >> 7;
            cpu.v[op.y] <<= 1;
            cpu.v[op.x] = cpu.v[op.y];
            cpu.v[0xF] = cpu.carry;
            cpu.pc += 2;
            break;

        default:
            fprintf(stderr, "Unknown opcode %04X!\n", op.inst);
            break;
        }

        break;

    case 0x9:
        DBG("SNE V%X, V%X", op.x, op.y);
        cpu.pc += cpu.v[op.x] != cpu.v[op.y] ? 4 : 2;
        break;

    case 0xA:
        DBG("LD I, %03X", op.nnn);
        cpu.i = op.nnn;
        cpu.pc += 2;
        break;

    case 0xB:
        DBG("JP V0, %03X", op.nnn);
        cpu.pc = op.nnn + cpu.v[0x0];
        break;

    case 0xC:
        DBG("RND V%X, %02X", op.x, op.kk);
        cpu.v[op.x] = rnd() & op.kk;
        cpu.pc += 2;
        break;

    case 0xD:
        DBG("DRW V%X, V%X, %X", op.x, op.y, op.n);
        draw(cpu.v[op.x], cpu.v[op.y], op.n);
        draw_flag = true;
        cpu.pc += 2;
        break;

    case 0xE:
        switch (op.kk) {
        case 0x9E:
            DBG("SKP V%X", op.x);
            cpu.pc += key[cpu.v[op.x]] ? 4 : 2;
            break;

        case 0xA1:
            DBG("SKP V%X", op.x);
            cpu.pc += !key[cpu.v[op.x]] ? 4 : 2;
            break;

        default:
            fprintf(stderr, "Unknown opcode %04X!\n", op.inst);
            break;
        }

        break;

    case 0xF:
        switch (op.kk) {
        case 0x07:
            DBG("LD V%X, DT", op.x);
            cpu.v[op.x] = timer.dt;
            cpu.pc += 2;
            break;

        case 0x0A:
            DBG("LD V%X, K", op.x);
            static int key_pressed = 0xFF;
            static bool got_key = false;

            if (!got_key) {
                for (int i = 0; i < 16; i++) {
                    if (key[i]) {
                        got_key = true;
                        key_pressed = i;
                        break;
                    }
                }
            }

            if (got_key)
                if (!key[key_pressed]) {
                    cpu.v[op.x] = key_pressed;
                    key_pressed = 0xFF;
                    got_key = false;
                    cpu.pc += 2;
                }

            break;

        case 0x15:
            DBG("LD DT, V%X", op.x);
            timer.dt = cpu.v[op.x];
            cpu.pc += 2;
            break;

        case 0x18:
            DBG("LD ST, V%X", op.x);
            timer.st = cpu.v[op.x];
            cpu.pc += 2;
            break;

        case 0x1E:
            DBG("ADD I, V%X", op.x);
            cpu.i += cpu.v[op.x];
            cpu.pc += 2;
            break;

        case 0x29:
            DBG("LD F, V%X", op.x);
            cpu.i = cpu.v[op.x] * 5;
            cpu.pc += 2;
            break;

        case 0x33:
            DBG("LD B, V%X", op.x);

            memory[cpu.i + 2] = cpu.v[op.x] % 10;
            memory[cpu.i + 1] = (cpu.v[op.x] / 10) % 10;
            memory[cpu.i] = (cpu.v[op.x] / 100) % 10;

            cpu.pc += 2;
            break;

        case 0x55:
            DBG("LD [I], V%X", op.x);

            for (int i = 0; i <= op.x; i++) {
                memory[cpu.i++] = cpu.v[i];
            }

            cpu.pc += 2;
            break;

        case 0x65:
            DBG("LD V%X, [I]", op.x);

            for (int i = 0; i <= op.x; i++) {
                cpu.v[i] = memory[cpu.i++];
            }

            cpu.pc += 2;
            break;

        default:
            fprintf(stderr, "Unknown opcode %04X!\n", op.inst);
            break;
        }

        break;

    default:
        fprintf(stderr, "Unknown opcode %04X!\n", op.inst);
        break;
    }

    DBG("");
}
