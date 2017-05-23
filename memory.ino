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

