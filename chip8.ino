#include <avr/pgmspace.h>
#include <EEPROM.h>
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
    uint8_t buttons_held(uint8_t button){
      return gb.buttons.repeat(button,0) ? 1 : 0;
    }
    /*void display_clearPixel(uint8_t x, uint8_t y){
      gb.display.setColor(
      gb.display.drawPixel(x,y);
    }*/
    void display_pbh(uint8_t a){
      gb.display.setColor(WHITE,BLACK);
      gb.display.cursorX = 0;
      gb.display.cursorY = 36;
      gb.display.print((char)a);
      gb.display.setColor(INVERT);
    }
    void display_print(C8* CH8){
      gb.display.setColor(WHITE,BLACK);
      gb.display.cursorX = 0;
      gb.display.cursorY = 36;
      gb.display.print(F("EX 0x"));
      gb.display.print(CH8->oph,HEX);
      gb.display.print(F(",0x"));
      gb.display.print(CH8->opl,HEX);
      gb.display.print(F(" @ 0x"));
      gb.display.println(CH8->pc,HEX);
      gb.display.print(F("I:0x"));
      gb.display.print(CH8->I,HEX);
      gb.display.print(F(" SP:0x"));
      gb.display.print(CH8->sp,HEX);
      gb.display.setColor(INVERT);
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

#define MAIN_MENU_LEN 3
const char loadrom[] PROGMEM = "Load ROM";
const char configure[] PROGMEM = "Config Keys";
const char title[] PROGMEM = "Title Screen";
const char* const menu[3] PROGMEM = {
  loadrom,
  configure,
  title
};

void setup() {
  gb.begin();
  gb.titleScreen(F("Chip8"));
  gb.battery.show = false;
  gb.setFrameRate(120);
  gb.pickRandomSeed();
  //initialize key bindings from EEPROM
  for(uint8_t i = 0; i < 6; i++){
    CH8.key_buf[i] = EEPROM.read(i);
  }
  memory_init(&CH8);
  //chip8_initialize(&CH8);
}

#define MODE_MENU 0
#define MODE_KEYS 1
#define MODE_GAME 2
uint8_t mode = 0;
bool unloaded = true;
//uint16_t mem_addr = -1;//0x200;


void loop() {
  if(gb.update()){
    //gb.display.clear();
    if( mode == MODE_MENU ){
      /*mem_addr = memory_load(&CH8, mem_addr, rom+(mem_addr-0x200), sizeof(rom)-(mem_addr-0x200));
      gb.display.cursorX = 0;
      gb.display.cursorY = 0;
      gb.display.print(F("Loading: "));
      gb.display.print(mem_addr-0x200);
      gb.display.print(F("/"));
      gb.display.println(sizeof(rom));*/

      switch( gb.menu(menu, MAIN_MENU_LEN) ){
        case -1:
          if(unloaded){
            gb.titleScreen(F("Chip8"));
            gb.battery.show = false;
          }else{
            mode = MODE_GAME;
            gb.display.persistence = true;
            gb.display.clear();
          }
          break;
        case 0:
          //Go to rom selection screen
          //mem_addr = -1;//Signal that we can't go back to game because we will corrupt its ram
          unloaded = true;//Signal that we can't go back to game because we will corrupt its ram
          if( rom_load() ){
            chip8_initialize(&CH8);
            mode = MODE_GAME; 
            gb.display.persistence = true;
            gb.display.clear();
            unloaded = false;
          }
          break;
        case 1:
          mode = MODE_KEYS;
          break;
        case 2: 
          gb.titleScreen(F("Chip8"));
          gb.battery.show = false;
          break;
      }
      
      //gb.display.print(((mem_addr-0x200)*100)/sizeof(rom));
      //gb.display.println(F("%"));
      //gb.display.println(loaded_index);
      //gb.display.println(CH8.memory[0x200%156]);
      //gb.display.println(mem_addr-0x200);
      //gb.display.println(sizeof(rom));
      //gb.display.print(counter);
      /*counter++;
      if( mem_addr == 0 ){
        chip8_initialize(&CH8);
        counter = 0;
        mode = MODE_GAME; 
        gb.display.persistence = true;
        gb.display.clear();
        //gb.display.setColor(WHITE,BLACK);
        //gb.display.fillScreen(BLACK);
      }*/
    }else if( mode == MODE_KEYS ){
      uint8_t i;
      for(i = 0; i < 16; i++){
        while(1){
          if( gb.update() ){
            //Let player choose a binding that they want for each game
            gb.display.cursorX = 0;
            gb.display.cursorY = 0;
            gb.display.print(F("Press button for "));
            gb.display.println(i,HEX);
            gb.display.println(F("\27 to skip key\nKeypad:"));
            gb.display.println(F("1 2 3 C"));
            gb.display.println(F("4 5 6 D"));
            gb.display.println(F("7 8 9 E"));
            gb.display.println(F("A 0 B F"));
            if( gb.buttons.pressed(BTN_C) ){
              break;//Do nothing
            }
            if( gb.buttons.pressed(BTN_A) ){
              EEPROM.update(BTN_A,i);
              break;
            }
            if( gb.buttons.pressed(BTN_B) ){
              EEPROM.update(BTN_B,i);
              break;
            }
            if( gb.buttons.pressed(BTN_UP) ){
              EEPROM.update(BTN_UP,i);
              break;
            }
            if( gb.buttons.pressed(BTN_RIGHT) ){
              EEPROM.update(BTN_RIGHT,i);
              break;
            }
            if( gb.buttons.pressed(BTN_DOWN) ){
              EEPROM.update(BTN_DOWN,i);
              break;
            }
            if( gb.buttons.pressed(BTN_LEFT) ){
              EEPROM.update(BTN_LEFT,i);
              break;
            }
          }
        }
      }
      //load into the key buf
      for(i = 0; i < 6; i++){
        CH8.key_buf[i] = EEPROM.read(i);
      }
      while(1){
        if( gb.update() ){
          gb.display.cursorX = 0;
          gb.display.cursorY = 0;
          gb.display.println(F("Bindings: "));
          gb.display.println(F("\33 \30 \32 \31 \25 \26"));//This would be wrong if the screen was rotated
          for(i = 0; i < 6; i++){
              gb.display.print(CH8.key_buf[i],HEX);
              gb.display.print(F(" "));
          }
          gb.display.println();
          gb.display.print(F("Press \25"));
          if( gb.buttons.pressed(BTN_A) ){
            break;
          }
        }
      }
      mode = MODE_MENU;
    }else{
      /*gb.display.setColor(WHITE,BLACK);
      gb.display.cursorX = 0;
      gb.display.cursorY = 42;
      gb.display.print( (char)('0' + buttons_held(0)) );*/
      /*gb.display.print(F("c "));
      gb.display.print(counter);
      gb.display.print(F(" i "));
      gb.display.println(loaded_index);*/
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
        //counter++;
        chip8_execute(&CH8);
        chip8_execute(&CH8);
      //}
      if( gb.buttons.pressed(BTN_C) ){
        mode = MODE_MENU;
      }
    }
    //chip8_execute(&CH8);
    //draw(&CH8);
  }
}
