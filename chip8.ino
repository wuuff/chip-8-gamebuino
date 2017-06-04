#include <avr/pgmspace.h>
#include <EEPROM.h>
#include "src\Gamebuino\Gamebuino.h"

Gamebuino gb;

extern uint8_t loaded_index;

extern "C" {
    #include "chip8.h"
    #include "trip8.h"
    void chip8_initialize();
    void chip8_execute();
    //void chip8_draw(C8* CH8);
    void display_clear(){
      //gb.display.clear();
      memset(CH8_display, 0, 256);
    }
    uint8_t display_getPixel(uint8_t x, uint8_t y){      
      return (CH8_display[(y >> 3) * 64 + x] >> (y & 7)) & 1;
    }
    void display_drawPixel(uint8_t x, uint8_t y){
      if ((x < 64) && (y<32))
        CH8_display[(y >> 3) * 64 + x] ^= (1 << (y & 7));
    }
    uint8_t buttons_held(uint8_t button){
      return gb.buttons.repeat(button,0) ? 1 : 0;
    }
    uint8_t get_random(uint8_t val){
      val = random(val);
      return val;
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
    void display_print(){
      gb.display.setColor(WHITE,BLACK);
      gb.display.cursorX = 0;
      gb.display.cursorY = 36;
      gb.display.print(F("EX 0x"));
      gb.display.print(CH8_oph,HEX);
      gb.display.print(F(",0x"));
      gb.display.print(CH8_opl,HEX);
      gb.display.print(F(" @ 0x"));
      gb.display.println(CH8_pc,HEX);
      gb.display.print(F("I:0x"));
      gb.display.print(CH8_I,HEX);
      gb.display.print(F(" SP:0x"));
      gb.display.print(CH8_sp,HEX);
      gb.display.setColor(INVERT);
    }

    uint8_t CH8_oph;//Opcode high byte
    uint8_t CH8_opl;//Opcode low byte    
    unsigned char CH8_V[0x10];
    unsigned short CH8_I;
    unsigned short CH8_pc;
    //unsigned char graphics[64 * 32];
    unsigned char CH8_delay_timer;
    unsigned char CH8_sound_timer;
    unsigned short CH8_stack[0x10];
    unsigned short CH8_sp;
    uint8_t CH8_key_buf[6];
    unsigned char CH8_workspace[512+rom_size];
}

//C8 CH8;

//Just do drawing here so we don't have to add pokitto drawing
//functions to the chip8 file
/*void draw(C8* CH8){
    int i, j;
    for (i = 0; i < 32; i++)
        for (j = 0; j < 64; j++){
            if( CH8_graphics[(j)+(i)*64] && j < 110 && i < 88){
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
  gb.display.setBuffer(CH8_display);
  
  gb.begin();  
  gb.titleScreen(F("Chip8"));
  gb.battery.show = false;
  gb.setFrameRate(120);
  gb.pickRandomSeed();
  //initialize key bindings from EEPROM
  for(uint8_t i = 0; i < 6; i++){
    CH8_key_buf[i] = EEPROM.read(i);
  }
  memory_init();
  //chip8_initialize(&CH8);
}

#define MODE_MENU 0
#define MODE_KEYS 1
#define MODE_GAME 2
uint8_t mode = 0;
bool unloaded = true;
//uint16_t mem_addr = -1;//0x200;

void flip() {
  //gb.display.update();
  gb.display.frameCount ++;
  uint8_t col, maxcol, p;
  uint8_t * src = CH8_display;

  for (p = 0; p < 6; p++) {
    gb.display.command(PCD8544_SETYADDR | p);
    
    // start at the beginning of the row
    col = 0;
    maxcol = LCDWIDTH_NOROT - 1;
    gb.display.command(PCD8544_SETXADDR | col);
    digitalWrite(SCR_DC, HIGH);
    digitalWrite(SCR_CS, LOW);
    
    if ((p==0) || (p==5))
      for (; col <= maxcol; col++)
        SPI.transfer(0x00);
    else {
      for (; col < 10; col++)
        SPI.transfer(0x00);
      for (; col < 74; col++)
        SPI.transfer(*src++);
      for (; col <= maxcol; col++)
        SPI.transfer(0x00);
    }
    digitalWrite(SCR_CS, HIGH);
  }
  gb.display.command(PCD8544_SETYADDR); // no idea why this is necessary but it is to finish the last byte?
}

void loop() {

  if(gb.update(flip)){

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
            chip8_initialize();
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
      //gb.display.println(CH8_memory[0x200%156]);
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
        CH8_key_buf[i] = EEPROM.read(i);
      }
      while(1){
        if( gb.update() ){
          gb.display.cursorX = 0;
          gb.display.cursorY = 0;
          gb.display.println(F("Bindings: "));
          gb.display.println(F("\33 \30 \32 \31 \25 \26"));//This would be wrong if the screen was rotated
          for(i = 0; i < 6; i++){
              gb.display.print(CH8_key_buf[i],HEX);
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
      mem_addr = (uint16_t)(((uint16_t)memory_get(&CH8,CH8_pc)) << 8) | ((uint16_t)memory_get(&CH8,CH8_pc + 1));
      gb.display.println(mem_addr);
      gb.display.println(speshul);
      display_print(&CH8);*/
      //gb.display.println(CH8_memory[0x200%156]);
      //if( gb.buttons.pressed(BTN_A) ){
        //counter++;
      for (int i=0; i<16; i++)
        chip8_execute();
      //}
      if( gb.buttons.pressed(BTN_C) ){
        mode = MODE_MENU;
      }
    }
    //chip8_execute(&CH8);
    //draw(&CH8);
  }
}
