#ifndef CHIP8_DEF
#define CHIP8_DEF

#define SDL_main main

//#define SDL_MAIN_HANDLED
#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <iostream>

#include "SDL.h"
#ifdef WIN32
#include <Windows.h>
#endif
#include <chrono>
#include <thread>

#define LOADEDCHIP 0x200
#define FONTSTART 0x050
#define FONTEND 0x09F
#define FONTMEMORY 0x1FF
#define NEWLINE "\n";

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef unsigned short us;

const u8 keymap[16] = {
    SDLK_x, SDLK_1, SDLK_2, SDLK_3, SDLK_q, SDLK_w, SDLK_e, SDLK_a,
    SDLK_s, SDLK_d, SDLK_z, SDLK_c, SDLK_4, SDLK_r, SDLK_f, SDLK_v,
};

class CHIP8 {
 public:
  CHIP8();
  ~CHIP8();

  void cycle() noexcept;
  bool loadROM(const char* rom);
  bool drawFlag;
  u8 graphicsBuffer[64 * 32];
  u8 key[16];

 private:
  template <typename U = u16>
  void logError(const char* error, const U msg = NULL) noexcept;
  void init() noexcept;
  void setupFont() noexcept;

  // All the memory is RAM and should be considered to be writable. CHIP-8 games
  // can, and do, modify themselves.
  u8 memory[4906];

  u16 stack[16];
  u16 sp;
  u16 pc;
  u16 opcode;
  u16 index;
  u8 Vx[16];  // registers Vx - VF

  u8 delay;
  u8 sound;

  const unsigned char font[FONTMEMORY]{

      0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
      0x20, 0x60, 0x20, 0x20, 0x70,  // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
      0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
      0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
      0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
      0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
      0xF0, 0x80, 0xF0, 0x80, 0x80   // F

  };
};

#endif
