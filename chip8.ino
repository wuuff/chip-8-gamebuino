#include <avr/pgmspace.h>
#include <Gamebuino.h>

Gamebuino gb;

extern uint8_t loaded_index;

extern "C" {
    #include "chip8.h"
    #include "trip8.h"
    void chip8_initialize(C8* CH8);
    void chip8_execute(C8* CH8);
    //void chip8_draw(C8* CH8);
    void display_clear(){
      gb.display.clear();
    }
    uint8_t display_getPixel(uint8_t x, uint8_t y){
      return gb.display.getPixel(x,y);
    }
    void display_drawPixel(uint8_t x, uint8_t y){
      gb.display.drawPixel(x,y);
    }
    /*void display_clearPixel(uint8_t x, uint8_t y){
      gb.display.setColor(
      gb.display.drawPixel(x,y);
    }*/
    void display_print(C8* CH8){
      gb.display.cursorX = 0;
      gb.display.cursorY = 36;
      gb.display.print(F("EX "));
      gb.display.print(CH8->opcode);
      gb.display.print(F(" @ "));
      gb.display.println(CH8->pc);
      gb.display.print(F("I:"));
      gb.display.print(CH8->I);
      gb.display.print(F(" SP:"));
      gb.display.print(CH8->sp);
    }
}

C8 CH8;

//Just do drawing here so we don't have to add pokitto drawing
//functions to the chip8 file
/*void draw(C8* CH8){
    int i, j;
    for (i = 0; i < 32; i++)
        for (j = 0; j < 64; j++){
            if( CH8->graphics[(j)+(i)*64] && j < 110 && i < 88){
                gb.display.drawPixel(j,i);
            }
        }
}*/

void setup() {
  gb.begin();
  gb.titleScreen(F("Chip8"));
  gb.battery.show = false;
  gb.setFrameRate(120);
  gb.pickRandomSeed();
  memory_init(&CH8);
  //chip8_initialize(&CH8);
}

uint8_t mode = 0;
uint8_t counter = 0;
uint16_t mem_addr = 0x200;


void loop() {
  if(gb.update()){
    //gb.display.clear();
    if( mode == 0 ){
      mem_addr = memory_load(&CH8, mem_addr, rom+(mem_addr-0x200), sizeof(rom)-(mem_addr-0x200));
      gb.display.cursorX = 0;
      gb.display.cursorY = 0;
      gb.display.print(F("Loading: "));
      gb.display.print(mem_addr-0x200);
      gb.display.print(F("/"));
      gb.display.println(sizeof(rom));
      //gb.display.print(((mem_addr-0x200)*100)/sizeof(rom));
      //gb.display.println(F("%"));
      //gb.display.println(loaded_index);
      //gb.display.println(CH8.memory[0x200%156]);
      //gb.display.println(mem_addr-0x200);
      //gb.display.println(sizeof(rom));
      //gb.display.print(counter);
      counter++;
      if( mem_addr == 0 ){
        chip8_initialize(&CH8);
        counter = 0;
        mode = 1; 
        gb.display.persistence = true;
        gb.display.clear();
        //gb.display.setColor(WHITE,BLACK);
        //gb.display.fillScreen(BLACK);
      }
    }else{
      gb.display.setColor(WHITE,BLACK);
      gb.display.cursorX = 0;
      gb.display.cursorY = 42;
      gb.display.print(F("c "));
      gb.display.print(counter);
      gb.display.print(F(" i "));
      gb.display.println(loaded_index);
      gb.display.setColor(INVERT);
      /*gb.display.print(memory_get(&CH8,0x200));
      gb.display.print(F(" "));
      gb.display.print(memory_get(&CH8,0x201));
      gb.display.print(F(" "));
      gb.display.print(memory_get(&CH8,0x202));
      gb.display.print(F(" "));
      gb.display.println(memory_get(&CH8,0x203));
      mem_addr = (uint16_t)(((uint16_t)memory_get(&CH8,CH8.pc)) << 8) | ((uint16_t)memory_get(&CH8,CH8.pc + 1));
      gb.display.println(mem_addr);
      gb.display.println(speshul);
      display_print(&CH8);*/
      //gb.display.println(CH8.memory[0x200%156]);
      //if( gb.buttons.pressed(BTN_A) ){
        counter++;
        chip8_execute(&CH8);
      //}
    }
    //chip8_execute(&CH8);
    //draw(&CH8);
  }
}
