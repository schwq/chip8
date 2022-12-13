#include "chip8.hpp"

static SDL_Window* window = NULL;
static int width = 1080;
static int height = 540;

const char* rom = "roms/tetris.ch8";

int main(int argc, char* argv) {
       
    std::cout << "[START]" << NEWLINE;

	CHIP8 chip = CHIP8();

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cout << "[ERROR]: SDL could not initialize: " << SDL_GetError() << NEWLINE;
        exit(1);
    }

    window = SDL_CreateWindow("Emulator CHIP8", SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);

    if (window == NULL) {
        std::cout << "[ERROR]: Window could not be created: " << SDL_GetError() << NEWLINE;
        exit(2);
    }

    SDL_Renderer* render = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetLogicalSize(render, width, height);

    SDL_Texture* tex = SDL_CreateTexture(render, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, 64, 32);

    u32 pixels[64 * 32];

load:
    if (!chip.loadROM(rom)) {
        return 2;
    }

    while (true) {
        chip.cycle();
        
        SDL_Event event;
        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_QUIT) exit(0);

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) exit(0);
                if (event.key.keysym.sym == SDLK_F1) goto load;

                for (int x = 0; x < 16; x++) {
                    if (event.key.keysym.sym == keymap[x]) {
                        chip.key[x] = 1;
                    }
                }
            }

            if (event.type == SDL_KEYUP) {
                for (int x = 0; x < 16; x++) {
                    if (event.key.keysym.sym == keymap[x]) {
                        chip.key[x] = 0;
                    }
                }
            }

        }

        if (chip.drawFlag) {
            chip.drawFlag = false;

            for (int x = 0; x < 2048; x++) {
                u8 pixel = chip.graphicsBuffer[x];
                pixels[x] = (0x00FFFFFF * pixel) | 0xFF000000;
            }

            SDL_UpdateTexture(tex, NULL, pixels, 64 * sizeof(Uint32));
            SDL_RenderClear(render);
            SDL_RenderCopy(render, tex, NULL, NULL);
            SDL_RenderPresent(render);
        }
        std::this_thread::sleep_for(std::chrono::microseconds(1500));
    }

    

}