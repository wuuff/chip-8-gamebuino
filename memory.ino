#include "SdFat.h"
SdFat SD;
//#include <petit_fatfs.h>

#include "chip8.h"

char filename[] = "CHIPMEMA.DAT";//# in filename may change

File memFile;
uint8_t loaded_index = -1;

extern "C" {

  /*uint8_t loaded(uint16_t addr){
    //return (addr >= loaded_index * 512 && addr < loaded_index * 512 + 512);
    return loaded_index == (addr/512);
  }*/
  #define loaded(addr) (loaded_index == ((addr)/pagesize))

  void memory_swap(C8* CH8, uint8_t index){
    //unsigned count;
    //Save current page
    filename[7] = 'A'+loaded_index;
    memFile = SD.open(filename,FILE_WRITE);
    memFile.seek(0);
    memFile.write(CH8->memory, pagesize);
    memFile.close();
    /*pf_open(filename);
    pf_lseek(0);
    pf_write(CH8->memory,pagesize,&count);
    pf_write(0,0,&count);*/
    

    //Load next page
    filename[7] = 'A'+index;
    memFile = SD.open(filename,FILE_READ);
    memFile.read(CH8->memory, pagesize);
    memFile.close();
    /*pf_open(filename);
    pf_read(CH8->memory,pagesize,&count);*/

    loaded_index = index;
  }

  void memory_set(C8* CH8, uint16_t addr, uint8_t val){
    if( !loaded(addr) ){
      memory_swap(CH8, addr/pagesize);
    }
    CH8->memory[addr%pagesize] = val;
  }

  uint8_t memory_get(C8* CH8, uint16_t addr){
    if( !loaded(addr) ){
      memory_swap(CH8, addr/pagesize);
    }
    return CH8->memory[addr%pagesize];
  }

  uint16_t memory_load(C8* CH8, uint16_t addr, const void* val, size_t count){
    if( !loaded(addr) ){
      memory_swap(CH8, (addr)/pagesize);
    }
    for( uint16_t i = 0; i < count; i++ ){
      if( !loaded(addr+i) ){
        //memory_swap(CH8, (addr+i)/pagesize);
        return addr+i;
      }
      CH8->memory[(addr+i)%pagesize] = pgm_read_byte(val+i);
      //return addr+i+1;//Just do one and return so we can get granular
    }
    return 0;
  }
  
}

void memory_init(C8* CH8){
  SD.begin(10);
  //PFFS.begin(10, rx, tx);
  //Create all possible pages on the SD card
  /*for(uint8_t i = 0; i < memsize/pagesize; i++){
    memFile = SD.open(filename,FILE_WRITE);
    memFile.write(CH8->memory, pagesize);//Just zeros at startup I hope
    memFile.close();
  }*/
  loaded_index = 0;//First page of memory
}

/*byte rx() { // needed by petit_fatfs
  SPDR = 0xFF;
  loop_until_bit_is_set(SPSR, SPIF);
  return SPDR;
}

void tx(byte d) { // needed by petit_fatfs
  SPDR = d;
  loop_until_bit_is_set(SPSR, SPIF);
}*/

int8_t custommenu(const char* const* items, uint8_t length) {
  gb.display.persistence = false;
  int8_t activeItem = 0;
  int8_t currentY = LCDHEIGHT;
  int8_t targetY = 0;
  boolean exit = false;
  int8_t answer = -1;
  while (1) {
    if (gb.update()) {
      if (gb.buttons.pressed(BTN_A) || gb.buttons.pressed(BTN_B) || gb.buttons.pressed(BTN_C)) {
        exit = true; //time to exit menu !
        targetY = - gb.display.fontHeight * length - 2; //send the menu out of the screen
        if (gb.buttons.pressed(BTN_A)) {
          answer = activeItem;
          gb.sound.playOK();
        } else {
          gb.sound.playCancel();
        }
      }
      if (exit == false) {
        if (gb.buttons.repeat(BTN_DOWN,4)) {
          activeItem++;
          gb.sound.playTick();
        }
        if (gb.buttons.repeat(BTN_UP,4)) {
          activeItem--;
          gb.sound.playTick();
        }
        //don't go out of the menu
        if (activeItem == length) activeItem = 0;
        if (activeItem < 0) activeItem = length - 1;

        targetY = -gb.display.fontHeight * activeItem + (gb.display.fontHeight+4); //center the menu on the active item
      } else { //exit :
        if ((currentY - targetY) <= 1)
        return (answer);
      }
      //draw a fancy menu
      currentY = (currentY + targetY) / 2;
      gb.display.cursorX = 0;
      gb.display.cursorY = currentY;
      gb.display.fontSize = 1;
      gb.display.textWrap = false;
      for (byte i = 0; i < length; i++) {
        if (i == activeItem){
          gb.display.cursorX = 3;
          gb.display.cursorY = currentY + gb.display.fontHeight * activeItem;
        }
        gb.display.println(*(items+i));
      }

      //display.fillRect(0, currentY + 3 + 8 * activeItem, 2, 2, BLACK);
      gb.display.setColor(WHITE);
      gb.display.drawFastHLine(0, currentY + gb.display.fontHeight * activeItem - 1, LCDWIDTH);
      gb.display.setColor(BLACK);
      gb.display.drawRoundRect(0, currentY + gb.display.fontHeight * activeItem - 2, LCDWIDTH, (gb.display.fontHeight+3), 3);
    }
  }
}

bool rom_load(C8* CH8){
  uint16_t offset = 0;
  uint16_t ptroffset = 130;
  uint8_t count = 0;//Please no more than 255 roms (although we actually have space for much less)
  char buf[13]; 
  File root = SD.open("/");
  //do{
    //memFile = root.openNextFile();
    //memFile.getName(buf,13);
    while (1) {
      if (gb.update()) {
        gb.display.cursorX = 0;
        gb.display.cursorY = 0;
        gb.display.println(F("The file:"));
        gb.display.println(buf);
        gb.display.println((char*)(CH8->memory));
        gb.display.println(*(char**)(CH8->memory+130));
        gb.display.println(F("--"));
        gb.display.println((char*)(CH8->memory+14));
        gb.display.println(*(char**)(CH8->memory+132));
        if (gb.buttons.pressed(BTN_A)){
          memFile = root.openNextFile();
          memFile.getName(buf,13);
          strcpy((char*)(CH8->memory+offset),buf);//Reuse memory buffer
      *((uint8_t**)(CH8->memory+ptroffset)) = (uint8_t*)(CH8->memory+offset);
      offset+=strlen(buf)+1;
      count++;
      ptroffset+=2; 
        }
        if (gb.buttons.pressed(BTN_B)){
          break;
        }
      }
    }
          
    //if( strlen(buf) > 3 && buf[strlen(buf)-1] == '8' && buf[strlen(buf)-2] == 'H' && buf[strlen(buf)-3] == 'C' ){
      
    //}
    //memFile.close();
  //}while(count <= 5);//memFile);

  custommenu((char**)&(buf),1);
  custommenu((char**)&(CH8->memory),1);
  custommenu((char**)(CH8->memory+130),count);
  return false;
}

