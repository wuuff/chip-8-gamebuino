#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <SDL/SDL.h>

#include <avr/pgmspace.h>
#include "trip8.h"
#include "chip8.h"

const unsigned char chip8_fontset[80] PROGMEM =
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

//#undef main

//0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
//0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
//0x200-0xFFF - Program ROM and work RAM

void chip8_initialize(C8 *);
void chip8_execute(C8 *);
//void chip8_draw(C8 *);
//void chip8_prec(char *, SDL_Event *);
//void chip8_prepare(char *);
//void chip8_start();

/*void chip8_start(){
    char name[100];

    printf("Enter the name of the game: ");
    scanf("%s", name);

    chip8_prepare(name);
}*/

/*void chip8_prepare(char * name){
    C8 CH8;

    chip8_initialize(&CH8, name);

    Uint8 * keys;
    //SDL_Event event;
    //SDL_Init(SDL_INIT_EVERYTHING);
    //SDL_SetVideoMode(SCREEN_W, SCREEN_H, SCREEN_BPP, SDL_HWSURFACE | SDL_DOUBLEBUF);


    for(;;){
        if(SDL_PollEvent (&event))
            continue;

        chip8_execute(&CH8);
        chip8_draw(&CH8);
        chip8_prec(name, &event);
    }
}*/

void chip8_initialize(C8 * CH8){

    int i;

    //memcpy_P(CH8->memory+0x200,rom, sizeof(rom)); // load game into memory
    //memory_load(CH8, 0x200, rom, sizeof(rom));
    //printf("size of rom %u\n", sizeof(rom));
    //printf("Start of rom: >%02x< >%02x< >%02x<\n", rom[0],rom[1],rom[2]);
    //printf("Start of mem: >%02x< >%02x< >%02x<\n", *(CH8->memory+0x200),*(CH8->memory+0x201),*(CH8->memory+0x202));
    //scanf((char*)&i);

    for(i = 0; i < 80; ++i){
            //CH8->memory[i] = pgm_read_byte(chip8_fontset[i]); // load fontset into memory
            memory_set(CH8,i,pgm_read_byte(chip8_fontset+i)); // load fontset into memory
    }

    //memset(CH8->graphics, 0, sizeof(CH8->graphics)); // clear graphics
    memset(CH8->stack, 0, sizeof(CH8->stack)); // clear stack
    memset(CH8->V, 0, sizeof(CH8->V)); // clear chip8 registers

    CH8->pc = 0x200;
    CH8->sp &= 0;
    //CH8->opcode = 0x200;
    CH8->oph = 0x02;
    CH8->opl = 0x00;
}

/*void chip8_draw(C8 * CH8){

    int i, j;
    SDL_Surface * surface = SDL_GetVideoSurface();
    SDL_LockSurface(surface);
    Uint32 * screen = (Uint32 *)surface->pixels;
    memset (screen,0,surface->w*surface->h*sizeof(Uint32));

    for (i = 0; i < SCREEN_H; i++)
        for (j = 0; j < SCREEN_W; j++){
            screen[j+i*surface->w] = CH8->graphics[(j/10)+(i/10)*64] ? 0xFFFFFFFF : 0;
        }

    SDL_UnlockSurface(surface);
    SDL_Flip(surface);
    SDL_Delay(15);
}*/

void chip8_timers(C8 * CH8){
    if(CH8->delay_timer > 0)
        CH8->delay_timer--;
    if(CH8->sound_timer > 0)
        CH8->sound_timer--;
    if(CH8->sound_timer != 0)
        ;//printf("%c", 7);
}

/*void chip8_prec(char * name, SDL_Event * event){
    Uint8 * keys = SDL_GetKeyState(NULL);
    if(keys[SDLK_ESCAPE])
        exit(1);
    if(keys[SDLK_r])
        chip8_prepare(name);
    if(keys[SDLK_c])
        chip8_start();
    if(keys[SDLK_p]){
        while(1){
            if(SDL_PollEvent(event)){
                keys = SDL_GetKeyState(NULL);
                if(keys[SDLK_ESCAPE])
                    exit(1);
                if(keys[SDLK_u])
                    break;
            }
        }
    }
}*/

void chip8_execute(C8 * CH8){

    //Uint8 * keys;
    int y, x, vx, vy;
    //uint8_t times;
    uint8_t i;
    //uint8_t scratch;//storage for temporary calculations
    uint8_t height, pixel;

    //for(times = 0; times < 1; times++){
        //CH8->opcode = CH8->memory[CH8->pc] << 8 | CH8->memory[CH8->pc + 1];
        //CH8->opcode = (((uint16_t)memory_get(CH8,CH8->pc)) << 8);//(uint16_t)((((uint16_t)memory_get(CH8,CH8->pc)) << 8) | ((uint16_t)memory_get(CH8,CH8->pc + 1)));
        CH8->oph = memory_get(CH8,CH8->pc);
        CH8->opl = memory_get(CH8,CH8->pc + 1);
        //printf ("Executing %04X at %04X , I:%02X SP:%02X\n", CH8->opcode, CH8->pc, CH8->I, CH8->sp);
        //if( times == 0 )
        //  display_print(CH8);
        switch(CH8->oph & 0xF0){
            case 0x00:
                switch(CH8->opl & 0x0F){
                    case 0x00: // 00E0: Clears the screen
                        display_clear();
                        //memset(CH8->graphics, 0, sizeof(CH8->graphics));
                        CH8->pc += 2;
                    break;

                    case 0x0E: // 00EE: Returns from subroutine
                        CH8->pc = CH8->stack[(--CH8->sp)&0xF] + 2;
                    break;
                    default: ;//printf("Wrong opcode: %X\n", CH8->opcode); getchar();

                }
            break;

            case 0x10: // 1NNN: Jumps to address NNN
                CH8->pc = (CH8->oph & 0x0F);
                CH8->pc = (CH8->pc << 8) | (CH8->opl);
                //CH8->pc |= CH8->oph & 0x0F;//Hopefully cast as byte and then only set high bits
            break;

            case 0x20: // 2NNN: Calls subroutine at NNN
                CH8->stack[(CH8->sp++)&0xF] = CH8->pc;
                CH8->pc = (CH8->oph & 0x0F);
                CH8->pc = (CH8->pc << 8) | (CH8->opl);
                //CH8->pc = CH8->opl & 0xFF;
                //*((uint8_t*)&CH8->pc) = CH8->oph & 0x0F;//Hopefully cast as byte and then only set high bits
            break;

            case 0x30: // 3XNN: Skips the next instruction if VX equals NN
                if(CH8->V[(CH8->oph & 0x0F)] == (CH8->opl))
                    CH8->pc += 4;
                else
                    CH8->pc += 2;
            break;

            case 0x40: // 4XNN: Skips the next instruction if VX doesn't equal NN
                if(CH8->V[(CH8->oph & 0x0F)] != (CH8->opl))
                    CH8->pc += 4;
                else
                    CH8->pc += 2;
            break;

            case 0x50: // 5XY0: Skips the next instruction if VX equals VY
                if(CH8->V[(CH8->oph & 0x0F)] == CH8->V[(CH8->opl & 0xF0) >> 4])
                    CH8->pc += 4;
                else
                    CH8->pc += 2;
            break;

            case 0x60: // 6XNN: Sets VX to NN
                CH8->V[(CH8->oph & 0x0F)] = (CH8->opl);
                CH8->pc += 2;
            break;

            case 0x70: // 7XNN: Adds NN to VX
                CH8->V[(CH8->oph & 0x0F)] += (CH8->opl);
                CH8->pc += 2;
            break;

            case 0x80:
                switch(CH8->opl & 0x0F){
                    case 0x00: // 8XY0: Sets VX to the value of VY
                        CH8->V[(CH8->oph & 0x0F)] = CH8->V[(CH8->opl & 0xF0) >> 4];
                        CH8->pc += 2;
                    break;

                    case 0x01: // 8XY1: Sets VX to VX or VY
                        CH8->V[(CH8->oph & 0x0F)] = CH8->V[(CH8->oph & 0x0F)] | CH8->V[(CH8->opl & 0xF0) >> 4];
                        CH8->pc += 2;
                    break;

                    case 0x02: // 8XY2: Sets VX to VX and VY
                        CH8->V[(CH8->oph & 0x0F)] = CH8->V[(CH8->oph & 0x0F)] & CH8->V[(CH8->opl & 0xF0) >> 4];
                        CH8->pc += 2;
                    break;

                    case 0x03: // 8XY3: Sets VX to VX xor VY
                        CH8->V[(CH8->oph & 0x0F)] = CH8->V[(CH8->oph & 0x0F)] ^ CH8->V[(CH8->opl & 0xF0) >> 4];
                        CH8->pc += 2;
                    break;

                    case 0x04: // 8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
                        if(((int)CH8->V[(CH8->oph & 0x0F)] + (int)CH8->V[(CH8->opl & 0xF0) >> 4]) < 256)
                            CH8->V[0xF] &= 0;
                        else
                            CH8->V[0xF] = 1;

                        CH8->V[(CH8->oph & 0x0F)] += CH8->V[(CH8->opl & 0xF0) >> 4];
                        CH8->pc += 2;
                    break;

                    case 0x05: // 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
                        if(((int)CH8->V[(CH8->oph & 0x0F)] - (int)CH8->V[(CH8->opl & 0xF0) >> 4]) >= 0)
                            CH8->V[0xF] = 1;
                        else
                            CH8->V[0xF] &= 0;

                        CH8->V[(CH8->oph & 0x0F)] -= CH8->V[(CH8->opl & 0xF0) >> 4];
                        CH8->pc += 2;
                    break;

                    case 0x06: // 8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift
                        CH8->V[0xF] = CH8->V[(CH8->oph & 0x0F)] & 7;
                        CH8->V[(CH8->oph & 0x0F)] = CH8->V[(CH8->oph & 0x0F)] >> 1;
                        CH8->pc += 2;
                    break;

                    case 0x07: // 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
                        if(((int)CH8->V[(CH8->oph & 0x0F)] - (int)CH8->V[(CH8->opl & 0xF0) >> 4]) > 0)
                            CH8->V[0xF] = 1;
                        else
                            CH8->V[0xF] &= 0;

                        CH8->V[(CH8->oph & 0x0F)] = CH8->V[(CH8->opl & 0xF0) >> 4] - CH8->V[(CH8->oph & 0x0F)];
                        CH8->pc += 2;
                    break;

                    case 0x0E: // 8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift
                        CH8->V[0xF] = CH8->V[(CH8->oph & 0x0F)] >> 7;
                        CH8->V[(CH8->oph & 0x0F)] = CH8->V[(CH8->oph & 0x0F)] << 1;
                        CH8->pc += 2;
                    break;
                    default: ;//printf("Wrong opcode: %X\n", CH8->opcode); getchar();

                }
            break;

            case 0x90: // 9XY0: Skips the next instruction if VX doesn't equal VY
                if(CH8->V[(CH8->oph & 0x0F)] != CH8->V[(CH8->opl & 0xF0) >> 4])
                    CH8->pc += 4;
                else
                    CH8->pc += 2;
            break;

            case 0xA0: // ANNN: Sets I to the address NNN
                //CH8->I = CH8->opcode & 0x0FFF;
                CH8->I = (CH8->oph & 0x0F);
                CH8->I = (CH8->I << 8) | (CH8->opl);
                //CH8->I = CH8->opl & 0xFF;
                //*((uint8_t*)&CH8->I) = CH8->oph & 0x0F;//Hopefully cast as byte and then only set high bits
                CH8->pc += 2;
            break;

            case 0xB0: // BNNN: Jumps to the address NNN plus V0
                //CH8->pc = (CH8->opcode & 0x0FFF) + CH8->V[0];
                CH8->pc = (CH8->oph & 0x0F);
                CH8->pc = (CH8->pc << 8) | (CH8->opl);
                //CH8->pc = CH8->opl & 0xFF;
                //*((uint8_t*)&CH8->pc) = CH8->oph & 0x0F;//Hopefully cast as byte and then only set high bits
                CH8->pc += CH8->V[0];
            break;

            case 0xC0: // CXNN: Sets VX to a random number and NN
                pixel = get_random(255);
                CH8->V[(CH8->oph & 0x0F)] = pixel & (CH8->opl);//rand() & (CH8->opl & 0xFF);
                CH8->pc += 2;
            break;

            case 0xD0: // DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels
                vx = CH8->V[(CH8->oph & 0x0F)];
                vy = CH8->V[(CH8->opl & 0xF0) >> 4];
                height = CH8->opl & 0x0F;
                CH8->V[0xF] &= 0;

                for(y = 0; y < height; y++){
                    //pixel = CH8->memory[CH8->I + y];
                    pixel = memory_get(CH8,CH8->I + y);
                    for(x = 0; x < 8; x++){
                        if(pixel & (0x80 >> x)){
                            //TODO: is this correct?
                            //if(CH8->graphics[x+vx+(y+vy)*64])
                            if( display_getPixel(x+vx,y+vy) ){
                              CH8->V[0xF] = 1;
                              //display_clearPixel(x+vx,y+vy);
                            }/*{
                              display_drawPixel(x+vx,y+vy);
                            }*/
                            display_drawPixel(x+vx,y+vy);
                            //CH8->graphics[x+vx+(y+vy)*64] ^= 1;
                            
                        }
                    }
                }
                CH8->pc += 2;
            break;

            case 0xE0:
                switch(CH8->opl & 0x0F){
                    case 0x0E: // EX9E: Skips the next instruction if the key stored in VX is pressed
                        //keys = SDL_GetKeyState(NULL);
                        for(i = 0; i < 6; i++){
                          pixel = buttons_held(i);
                          if( CH8->key_buf[i] == CH8->V[(CH8->oph & 0x0F)] && pixel ){
                            CH8->pc += 2;//Add 2 and then add 2 after
                            break;
                          }
                        }
                        CH8->pc += 2;
                        //if(keys[keymap[CH8->V[(CH8->opcode & 0x0F00) >> 8]]])
                        //    CH8->pc += 4;
                        //else
                        //    CH8->pc += 2;
                    break;

                    case 0x01: // EXA1: Skips the next instruction if the key stored in VX isn't pressed
                        //keys = SDL_GetKeyState(NULL);
                        for(i = 0; i < 6; i++){
                          pixel = buttons_held(i);
                          if( CH8->key_buf[i] == CH8->V[(CH8->oph & 0x0F)] && !pixel ){
                            CH8->pc += 2;//Add 2 and then add 2 after
                            break;
                          }
                        }
                        CH8->pc += 2;
                        //if(!keys[keymap[CH8->V[(CH8->opcode & 0x0F00) >> 8]]])
                        //    CH8->pc += 4;
                        //else
                        //    CH8->pc += 2;
                    break;
                    default: ;//printf("Wrong opcode: %X\n", CH8->opcode); getchar();

                }
            break;

            case 0xF0:
                switch(CH8->opl){
                    case 0x07: // FX07: Sets VX to the value of the delay timer
                        CH8->V[(CH8->oph & 0x0F)] = CH8->delay_timer;
                        CH8->pc += 2;
                    break;

                    case 0x0A: // FX0A: A key press is awaited, and then stored in VX
                        //keys = SDL_GetKeyState(NULL);
                        for(i = 0; i < 6; i++){
                          pixel = buttons_held(i);//WHY IS THIS NEEDED?!  WHY DOES CALLING IT DIRECTLY RETURN A WRONG ANSWER
                          //display_pbh(pixel);
                          if( pixel == 1 ){
                            CH8->V[(CH8->oph & 0x0F)] = CH8->key_buf[i];
                            CH8->pc += 2;
                            break;
                          }
                        }
                        /*for(i = 0; i < 0x10; i++)
                            if(keys[keymap[i]]){
                                CH8->V[(CH8->opcode & 0x0F00) >> 8] = i;
                                CH8->pc += 2;
                            }*/
                    break;

                    case 0x15: // FX15: Sets the delay timer to VX
                        CH8->delay_timer = CH8->V[(CH8->oph & 0x0F)];
                        CH8->pc += 2;
                    break;

                    case 0x18: // FX18: Sets the sound timer to VX
                        CH8->sound_timer = CH8->V[(CH8->oph & 0x0F)];
                        CH8->pc += 2;
                    break;

                    case 0x1E: // FX1E: Adds VX to I
                        CH8->I += CH8->V[(CH8->oph & 0x0F)];
                        CH8->pc += 2;
                    break;

                    case 0x29: // FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
                        CH8->I = CH8->V[(CH8->oph & 0x0F)] * 5;
                        CH8->pc += 2;
                    break;

                    case 0x33: // FX33: Stores the Binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2
                        /*CH8->memory[CH8->I] = CH8->V[(CH8->opcode & 0x0F00) >> 8] / 100;
                        CH8->memory[CH8->I+1] = (CH8->V[(CH8->opcode & 0x0F00) >> 8] / 10) % 10;
                        CH8->memory[CH8->I+2] = CH8->V[(CH8->opcode & 0x0F00) >> 8] % 10;*/
                        memory_set(CH8,CH8->I,CH8->V[(CH8->oph & 0x0F)] / 100);
                        memory_set(CH8,CH8->I+1,(CH8->V[(CH8->oph & 0x0F)] / 10) % 10);
                        memory_set(CH8,CH8->I+2, CH8->V[(CH8->oph & 0x0F)] % 10);
                        CH8->pc += 2;
                    break;

                    case 0x55: // FX55: Stores V0 to VX in memory starting at address I
                        for(i = 0; i <= ((CH8->oph & 0x0F)); i++)
                            //CH8->memory[CH8->I+i] = CH8->V[i];
                            memory_set(CH8,CH8->I+i,CH8->V[i]);
                        CH8->pc += 2;
                    break;

                    case 0x65: //FX65: Fills V0 to VX with values from memory starting at address I
                        for(i = 0; i <= ((CH8->oph & 0x0F)); i++)
                            //CH8->V[i] = CH8->memory[CH8->I + i];
                            CH8->V[i] = memory_get(CH8,CH8->I + i);
                        CH8->pc += 2;
                    break;

                    default: ;//printf("Wrong opcode: %X\n", CH8->opcode); getchar();
                }
            break;
            default: ;//printf("Wrong opcode: %X\n", CH8->opcode); getchar();

        }
        chip8_timers(CH8);
    //}
}

/*int main(){
    chip8_start();
    return 0;
}*/
