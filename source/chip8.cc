#include "chip8.hpp"

CHIP8::CHIP8() { init(); }

CHIP8::~CHIP8(){};

void CHIP8::init() noexcept {
  pc = 0x200;
  opcode = 0;
  index = 0;
  sp = 0;

  for (int x = 0; x < 16; x++) {
    stack[x] = 0;
    Vx[x] = 0;
    key[x] = 0;
  }

  for (int x = 0; x < 2048; x++) {
    graphicsBuffer[x] = 0;
  }

  for (int x = 0; x < 4096; x++) {
    memory[x] = 0;
  }

  setupFont();

  delay = 0;
  sound = 0;

  srand(time(NULL));
}

void CHIP8::setupFont() noexcept {
  for (int i = 0; i < 80; i++) {
    memory[FONTSTART + i] = font[i];
  }
}

template <typename U>
void CHIP8::logError(const char* error, const U msg) noexcept {
  if (msg != NULL) {
    std::cerr << error << " " << msg << NEWLINE;
  } else {
    std::cerr << error << NEWLINE;
  }
}

bool CHIP8::loadROM(const char* rom) {
  init();

  FILE* ROMfile = fopen(rom, "rb");

  if (ROMfile == NULL) {
    logError("[ERROR]: could not load ROM file!");
    return false;
  }

  fseek(ROMfile, 0, SEEK_END);
  long size = ftell(ROMfile);
  rewind(ROMfile);

  char* buffer = (char*)malloc(sizeof(char) * size);
  if (buffer == NULL) {
    logError("[ERROR]: Could not allocate memory for ROM file!");

    return false;
  }

  size_t result = fread(buffer, sizeof(char), (size_t)size, ROMfile);
  if (result != size) {
    logError("[ERROR]: Could not read ROM!");
    return false;
  }

  if ((4096 - LOADEDCHIP) > size) {
    for (int x = 0; x < size; x++) {
      memory[x + LOADEDCHIP] = (u8)buffer[x];
    }
  } else {
    logError("[ERROR]: ROM file is too large for memory!");

    return false;
  }
  fclose(ROMfile);
  free(buffer);

  return true;
}

void CHIP8::cycle() noexcept {
  // First we shifted left 8 bits, which adds 8 zeros.
  // Next we use the bitwise OR operation to merge them :
  opcode = memory[pc] << 8 | memory[pc + 1];

  // reads the first 4 bits to know which opcode to call
  switch (opcode & 0xF000) {
    case 0x0000:
      switch (opcode & 0x000F) {
        case 0x0000:  // 0x00E0  Clears the screen

          for (int x = 0; x < (64 * 32); x++) {
            graphicsBuffer[x] = 0;
          }

          drawFlag = true;
          pc += 2;
          break;

        case 0x000E:  // 0x00EE: Returns from subroutine
          --sp;
          pc = stack[sp];
          pc += 2;
          break;

        default:
          logError("[ERROR]: Unknown opcode [0x0000]: ", opcode);
          exit(1);
      }
      break;

    case 0x1000:
      pc = opcode & 0x0FFF;
      break;

    case 0x2000:
      stack[sp] = pc;
      ++sp;
      pc = opcode & 0x0FFF;
      break;

    case 0x3000:
      if (Vx[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
        pc += 4;
      } else {
        pc += 2;
      }
      break;

    case 0x4000:
      if (Vx[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
        pc += 4;
      } else {
        pc += 2;
      }
      break;

    case 0x5000:
      if (Vx[(opcode & 0x0F00) >> 8] == Vx[(opcode & 0x00F0) >> 4]) {
        pc += 4;
      } else {
        pc += 2;
      }
      break;

    case 0x6000:
      Vx[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
      pc += 2;
      break;

    case 0x7000:
      Vx[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
      pc += 2;
      break;

    case 0x8000:

      switch (opcode & 0x000F) {
        case 0x0000:
          Vx[(opcode & 0x0F00) >> 8] = Vx[(opcode & 0x00F0) >> 4];
          pc += 2;
          break;

        case 0x0001:
          Vx[(opcode & 0x0F00) >> 8] |= Vx[(opcode & 0x00F0) >> 4];
          pc += 2;
          break;

        case 0x0002:
          Vx[(opcode & 0x0F00) >> 8] &= Vx[(opcode & 0x00F0) >> 4];
          pc += 2;
          break;

        case 0x0003:
          Vx[(opcode & 0x0F00) >> 8] ^= Vx[(opcode & 0x00F0) >> 4];
          pc += 2;
          break;

        case 0x0004:
          Vx[(opcode & 0x0F00) >> 8] += Vx[(opcode & 0x00F0) >> 4];

          if (Vx[(opcode & 0x00F0) >> 4] >
              (0xFF - Vx[(opcode & 0x0F00) >> 8])) {
            Vx[0xF] = 1;
          } else {
            Vx[0xF] = 0;
          }

          pc += 2;
          break;

        case 0x0005:
          if (Vx[(opcode & 0x00F0) >> 4] > (Vx[(opcode & 0x0F00) >> 8])) {
            Vx[0xF] = 0;
          } else {
            Vx[0xF] = 1;
          }

          Vx[(opcode & 0x0F00) >> 8] -= Vx[(opcode & 0x00F0) >> 4];
          pc += 2;
          break;

        case 0x0006:
          Vx[0xF] = Vx[(opcode & 0x0F00) >> 8] & 0x1;
          Vx[(opcode & 0x0F00) >> 8] >>= 1;
          pc += 2;
          break;

        case 0x0007:
          if (Vx[(opcode & 0x0F00) >> 8] > Vx[(opcode & 0x00F0) >> 4]) {
            Vx[0xF] = 0;
          } else {
            Vx[0xF] = 1;
          }
          Vx[(opcode & 0x0F00) >> 8] =
              Vx[(opcode & 0x00F0) >> 4] - Vx[(opcode & 0x0F00) >> 8];
          pc += 2;
          break;

        case 0x000E:
          Vx[0xF] = Vx[(opcode & 0x0F00) >> 8] >> 7;
          Vx[(opcode & 0x0F00) >> 8] <<= 1;
          pc += 2;
          break;

        default:
          logError("[ERROR]: Unknown opcode [0x8000]: ", opcode);
          exit(1);
      }
      break;

    case 0x9000:
      if (Vx[(opcode & 0x0F00) >> 8] != Vx[(opcode & 0x00F0) >> 4]) {
        pc += 4;
      } else {
        pc += 2;
      }
      break;

    case 0xA000:
      index = opcode & 0x0FFF;
      pc += 2;
      break;

    case 0xB000:
      pc = Vx[0] + (opcode & 0x0FFF);
      break;

    case 0xC000:
      Vx[(opcode & 0x0F00) >> 8] = (rand() % (0xFF + 1)) & (opcode & 0x00FF);
      pc += 2;
      break;

    case 0xD000: {
      us x = Vx[(opcode & 0x0F00) >> 8];
      us y = Vx[(opcode & 0x00F0) >> 4];
      us height = opcode & 0x000F;
      us pixel;

      Vx[0xF] = 0;

      for (int yline = 0; yline < height; yline++) {
        pixel = memory[index + yline];
        for (int xline = 0; xline < 8; xline++) {
          if ((pixel & (0x80 >> xline)) != 0) {
            if (graphicsBuffer[(x + xline + ((y + yline) * 64))] == 1)
              Vx[0xF] = 1;
            graphicsBuffer[x + xline + ((y + yline) * 64)] ^= 1;
          }
        }
      }

      drawFlag = true;
      pc += 2;
    } break;

    case 0xE000:

      switch (opcode & 0x00FF) {
        case 0x009E:
          if (key[Vx[(opcode & 0x0F00) >> 8]] != 0) {
            pc += 4;
          } else {
            pc += 2;
          }
          break;

        case 0x00A1:
          if (key[Vx[(opcode & 0x0F00) >> 8]] == 0) {
            pc += 4;
          } else {
            pc += 2;
          }
          break;

        default:
          logError("[ERROR]: Unknown opcode [0xE000]: ", opcode);
          exit(1);
      }
      break;

    case 0xF000:

      switch (opcode & 0x00FF) {
        case 0x0007:
          Vx[(opcode & 0x0F00) >> 8] = delay;
          pc += 2;
          break;

        case 0x000A: {
          bool key_pressed = false;

          for (int x = 0; x < 16; x++) {
            if (key[x] != 0) {
              Vx[(opcode & 0x0F00) >> 8] = x;
              key_pressed = true;
            }
          }

          if (!key_pressed) return;

          pc += 2;
        } break;

        case 0x0015:
          delay = Vx[(opcode & 0x0F00) >> 8];
          pc += 2;
          break;

        case 0x0018:
          sound = Vx[(opcode & 0x0F00) >> 8];
          pc += 2;
          break;

        case 0x001E:
          if (index + Vx[(opcode & 0x0F00) >> 8] > 0xFFF) {
            Vx[0xF] = 1;
          } else {
            Vx[0xF] = 0;
          }
          index += Vx[(opcode & 0x0F00) >> 8];
          pc += 2;
          break;

        case 0x0029:
          index = Vx[(opcode & 0x0F00) >> 8] * 0x5;
          pc += 2;
          break;

        case 0x0033:
          memory[index] = Vx[(opcode & 0x0F00) >> 8] / 100;
          memory[index + 1] = (Vx[(opcode & 0x0F00) >> 8] / 10) % 10;
          memory[index + 2] = Vx[(opcode & 0x0F00) >> 8] % 10;
          pc += 2;
          break;

        case 0x0055:

          for (int x = 0; x <= ((opcode & 0x0F00) >> 8); x++) {
            memory[index + x] = Vx[x];
          }

          index += ((opcode & 0x0F00) >> 8) + 1;
          pc += 2;
          break;

        case 0x0065:

          for (int x = 0; x <= ((opcode & 0x0F00) >> 8); x++) {
            Vx[x] = memory[index + x];
          }

          index += ((opcode & 0x0F00) >> 8) + 1;
          pc += 2;
          break;

        default:
          logError("[ERROR]: Unknown opcode [0xF000]: ", opcode);
          exit(1);
      }
      break;

    default:
      logError("[ERROR]: Unknown opcode!", opcode);
      exit(1);
  }

  if (delay > 0) --delay;
  if (sound > 0) {
    if (sound == 1) {
#ifdef WIN32
      Beep(523, 500);
#endif
      std::cout << "Beep!\a" << std::endl;
      --sound;
    }
  }
}
