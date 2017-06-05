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

  void memory_swap(uint8_t index){
    //unsigned count;
    //Save current page
    filename[7] = 'A'+loaded_index;
    memFile = SD.open(filename,FILE_WRITE);
    memFile.seek(0);
    memFile.write(CH8_memory, pagesize);
    memFile.close();
    /*pf_open(filename);
    pf_lseek(0);
    pf_write(CH8_memory,pagesize,&count);
    pf_write(0,0,&count);*/
    

    //Load next page
    filename[7] = 'A'+index;
    memFile = SD.open(filename,FILE_READ);
    memFile.read(CH8_memory, pagesize);
    memFile.close();
    /*pf_open(filename);
    pf_read(CH8_memory,pagesize,&count);*/

    loaded_index = index;
  }

  void memory_set(uint16_t addr, uint8_t val){
    if( !loaded(addr) ){
      memory_swap(addr/pagesize);
    }
    CH8_memory[addr%pagesize] = val;
  }

  uint8_t memory_get(uint16_t addr){
    if( !loaded(addr) ){
      memory_swap(addr/pagesize);
    }
    return CH8_memory[addr%pagesize];
  }

  /*uint16_t memory_load(C8* CH8, uint16_t addr, const void* val, size_t count){
    if( !loaded(addr) ){
      memory_swap(CH8, (addr)/pagesize);
    }
    for( uint16_t i = 0; i < count; i++ ){
      if( !loaded(addr+i) ){
        //memory_swap(CH8, (addr+i)/pagesize);
        return addr+i;
      }
      CH8_memory[(addr+i)%pagesize] = pgm_read_byte(val+i);
      //return addr+i+1;//Just do one and return so we can get granular
    }
    return 0;
  }*/

  void memory_load(char* fname){
    uint16_t addr = 0x200;
    unsigned long filesize = 0;
    loaded_index = addr/pagesize;//Switch to starting page (we don't care about saving current mem contents)
    memFile = SD.open(fname,FILE_READ);
    filesize = memFile.size();
    while( (addr-0x200) < filesize ){
      if( gb.update() ){
        gb.display.cursorX = 0;
        gb.display.cursorY = 0;
        gb.display.print(F("Loading "));
        gb.display.println(fname);
        gb.display.print(addr-0x200);
        gb.display.print(F("/"));
        gb.display.println(filesize);
        //if( gb.buttons.pressed(BTN_A) ){
        if( !loaded(addr) ){
          memFile.close();
          memory_swap(addr/pagesize);//Swap pages out, which trashes the file state
          memFile = SD.open(fname,FILE_READ);//Re-open the file
          memFile.seek(addr-0x200);//Return to previous spot
        }
        CH8_memory[(addr)%pagesize] = (uint8_t)memFile.read();
        addr++;
        //}
      }
    }
    
    
  }
  
}

void memory_init(){
  SD.begin(10);
  //PFFS.begin(10, rx, tx);
  //Create all possible pages on the SD card
  /*for(uint8_t i = 0; i < memsize/pagesize; i++){
    memFile = SD.open(filename,FILE_WRITE);
    memFile.write(CH8_memory, pagesize);//Just zeros at startup I hope
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

#define MENU_PTRS 130 //Room for 10 roms
bool rom_menu(char* buf){
  uint16_t offset = 0;
  uint16_t ptroffset = MENU_PTRS;
  uint8_t count = 0;//Please no more than 255 roms (although we actually have space for much less)
  File root = SD.open("/");
  do{
    memFile = root.openNextFile();
    memFile.getName(buf,13);
    /*while (1) {
      if (gb.update()) {
        gb.display.cursorX = 0;
        gb.display.cursorY = 0;
        gb.display.println(F("The file:"));
        gb.display.println(buf);
        gb.display.println((char*)(CH8_memory));
        gb.display.println(*(char**)(CH8_memory+130));
        gb.display.println(F("--"));
        gb.display.println((char*)(CH8_memory+14));
        gb.display.println(*(char**)(CH8_memory+132));
        if (gb.buttons.pressed(BTN_A)){
          memFile = root.openNextFile();
          memFile.getName(buf,13);
          strcpy((char*)(CH8_memory+offset),buf);//Reuse memory buffer
      *((uint8_t**)(CH8_memory+ptroffset)) = (uint8_t*)(CH8_memory+offset);
      offset+=strlen(buf)+1;
      count++;
      ptroffset+=2; 
        }
        if (gb.buttons.pressed(BTN_B)){
          break;
        }
      }
    }*/
          
    if( strlen(buf) > 3 && buf[strlen(buf)-1] == '8' ){// && buf[strlen(buf)-2] == 'H' && buf[strlen(buf)-3] == 'C' ){
      strcpy((char*)(CH8_memory+offset),buf);//Reuse memory buffer
      *((uint8_t**)(CH8_memory+ptroffset)) = (uint8_t*)(CH8_memory+offset);
      offset+=strlen(buf)+1;
      count++;
      ptroffset+=2; 
    }
    //memFile.close();
  }while(memFile);

  //custommenu((char**)&(buf),1);
  //custommenu((char**)&(CH8_memory),1);
  count = custommenu((char**)(CH8_memory+MENU_PTRS),count);
  if( count == -1 ){
    return false;
  }
  strcpy(buf,((char**)(CH8_memory+MENU_PTRS))[count]);//Load filename into buffer
  return true;
}

bool rom_load(){
  char buf[13]; 
  if( rom_menu(buf) ){
    memory_load(buf);
    return true;
  }
  return false;
}

