#ifndef CHIP8
#define CHIP8

#define memsize 4096
#define pagesize 156
#define SCREEN_W 640
#define SCREEN_H 320
#define SCREEN_BPP 32
#define W 64
#define H 32

/*static int keymap[0x10] = {
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
};*/

typedef struct chip8{
    //FILE * game;

    unsigned short opcode;
    unsigned char memory[pagesize];
    unsigned char V[0x10];
    unsigned short I;
    unsigned short pc;
    //unsigned char graphics[64 * 32];
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned short stack[0x10];
    unsigned short sp;
    unsigned char key[0x10];
} C8;
#endif // CHIP8
