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

/*typedef struct chip8{
    //FILE * game;

    uint8_t oph;//Opcode high byte
    uint8_t opl;//Opcode low byte
    unsigned char memory[pagesize];
    unsigned char V[0x10];
    unsigned short I;
    unsigned short pc;
    //unsigned char graphics[64 * 32];
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned short stack[0x10];
    unsigned short sp;
    uint8_t key_buf[6];
    //unsigned char key[0x10];
} C8;*/
extern uint8_t CH8_oph;//Opcode high byte
extern uint8_t CH8_opl;//Opcode low byte
extern unsigned char CH8_memory[pagesize];
extern unsigned char CH8_V[0x10];
extern unsigned short CH8_I;
extern unsigned short CH8_pc;
extern unsigned char CH8_delay_timer;
extern unsigned char CH8_sound_timer;
extern unsigned short CH8_stack[0x10];
extern unsigned short CH8_sp;
extern uint8_t CH8_key_buf[6];
#endif // CHIP8
