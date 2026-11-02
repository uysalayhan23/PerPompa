#ifndef HD44780_I2CEXP_STUB_H
#define HD44780_I2CEXP_STUB_H
#include <stdint.h>
#include <string.h>
class hd44780_I2Cexp {
public:
  char row[2][17];
  int initCode, cursorCode;
  unsigned int writes;
  int shortWritesAfter;
  uint8_t col, line;
  hd44780_I2Cexp():initCode(0),cursorCode(0),writes(0),shortWritesAfter(-1),col(0),line(0){memset(row,0,sizeof(row));}
  int begin(int columns,int rows){return(columns==16 && rows==2)?initCode:-1;}
  int setCursor(uint8_t x,uint8_t y){col=x;line=y;return (x<16 && y<2)?cursorCode:-1;}
  size_t print(const char* text){
    if(!text||line>1||col>15)return 0;
    const size_t n=strlen(text);
    if(shortWritesAfter>=0 && (int)writes++>=shortWritesAfter)return n? n-1 :0;
    for(size_t i=0;i<n&&i+col<16;++i)row[line][i+col]=text[i];
    row[line][16]=0;
    return n;
  }
};
#endif
