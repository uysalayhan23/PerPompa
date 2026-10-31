#include <cassert>
#include <cstring>
#include <cstdio>
enum Button{Open,Pressed,Held,Released,Clicked,DoubleClicked};
int actions=0;
void handle(Button b){
  switch(b){
    case Clicked:++actions;break;
    case Held:actions+=2;break;
    case Released:break;
    case DoubleClicked:break;
    case Open:
    case Pressed:break;
  }
}
int main(){
  int count=0;
  char customUL[]={'u','L',0};
  char customRate[]={'u','L','/','m','i','n',0};
  const char *vol=customUL,*rate=customRate;
  assert(std::strcmp(vol,"uL")==0);++count;
  assert(std::strcmp(rate,"uL/min")==0);++count;
  assert(vol!=(const char*)"uL");++count; // same content, distinct storage
  assert(rate!=(const char*)"uL/min");++count;
  assert(std::strcmp("mL","uL")!=0);++count;
  handle(Open);handle(Pressed);assert(actions==0);++count;
  handle(Clicked);handle(Held);assert(actions==3);++count;
  std::printf("P002_STRING_SWITCH_TARGET_CPP=%d/%d PASS; ORIGINAL_INO_UNCHANGED; AVR=NOT_RUN\n",count,count);
}
