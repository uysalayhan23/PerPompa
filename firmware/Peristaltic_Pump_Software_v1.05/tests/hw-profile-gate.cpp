// PerPompa v1.05 - HOST C++ configuration commissioning model, not firmware code.
// Built/tested with g++ -std=c++17 -Wall -Wextra -Werror -pedantic -fsanitize=address,undefined
#include <cassert>
#include <cstdio>
#include <cstdint>
enum class Board : uint8_t { ProMini5V16MHz=0, ProMini3V3_8MHz=1, Invalid=255 };
enum class Driver : uint8_t { A4988=0, DRV8825=1, Invalid=255 };
struct Profile {
 Board board; Driver driver; uint8_t microsteps;
 bool storedWithValidMarker; bool commissioningConfirmed;
};
constexpr bool validDriver(Driver d){return d==Driver::A4988 || d==Driver::DRV8825;}
constexpr bool validBoard(Board b){return b==Board::ProMini5V16MHz || b==Board::ProMini3V3_8MHz;}
constexpr unsigned pulseWidthMicroseconds(Driver d){return validDriver(d) ? 2u : 0u;}
constexpr bool safeToStart(const Profile& p, unsigned long compileTimeFcpu, bool motorIdle, bool remoteIdle){
 return validBoard(p.board) && validDriver(p.driver) && p.storedWithValidMarker &&
 p.commissioningConfirmed && p.microsteps==4 && motorIdle && remoteIdle &&
 ((p.board==Board::ProMini5V16MHz && compileTimeFcpu==16000000ul) ||
  (p.board==Board::ProMini3V3_8MHz && compileTimeFcpu==8000000ul));
}
int main(){
 int pass=0;
 auto check=[&](bool good, const char* description){ if(!good){std::fprintf(stderr,"FAIL: %s\n",description);return false;}pass++;return true;};
 Profile a{Board::ProMini5V16MHz, Driver::A4988, 4, true, true};
 Profile b{Board::ProMini5V16MHz, Driver::DRV8825, 4, true, true};
 Profile c{Board::ProMini3V3_8MHz, Driver::A4988, 4, true, true};
 Profile d{Board::ProMini3V3_8MHz, Driver::DRV8825, 4, true, true};
 assert(check(safeToStart(a,16000000ul,true,true),"16MHz A4988 valid"));
 assert(check(safeToStart(b,16000000ul,true,true),"16MHz DRV8825 valid"));
 assert(check(safeToStart(c,8000000ul,true,true),"8MHz A4988 valid"));
 assert(check(safeToStart(d,8000000ul,true,true),"8MHz DRV8825 valid"));
 assert(check(!safeToStart(a,8000000ul,true,true),"16MHz profile vs 8MHz firmware"));
 assert(check(!safeToStart(b,8000000ul,true,true),"16MHz DRV vs 8MHz firmware"));
 assert(check(!safeToStart(c,16000000ul,true,true),"8MHz profile vs 16MHz firmware"));
 assert(check(!safeToStart(d,16000000ul,true,true),"8MHz DRV vs 16MHz firmware"));
 assert(check(!safeToStart(a,12000000ul,true,true),"unsupported F_CPU rejected"));
 a.storedWithValidMarker=false;
 assert(check(!safeToStart(a,16000000ul,true,true),"uninitialized EEPROM rejected"));
 a.storedWithValidMarker=true;a.commissioningConfirmed=false;
 assert(check(!safeToStart(a,16000000ul,true,true),"not commissioned rejected"));
 a.commissioningConfirmed=true;a.microsteps=8;
 assert(check(!safeToStart(a,16000000ul,true,true),"microstep != 1/4 rejected"));
 a.microsteps=4;a.driver=Driver::Invalid;
 assert(check(!safeToStart(a,16000000ul,true,true),"invalid driver rejected"));
 a.driver=Driver::A4988;a.board=Board::Invalid;
 assert(check(!safeToStart(a,16000000ul,true,true),"invalid board rejected"));
 a.board=Board::ProMini5V16MHz;
 assert(check(!safeToStart(a,16000000ul,false,true),"running configuration change rejected"));
 assert(check(!safeToStart(a,16000000ul,true,false),"remote-active configuration change rejected"));
 assert(check(pulseWidthMicroseconds(Driver::A4988)>=1u,"A4988 1us STEP min"));
 assert(check(pulseWidthMicroseconds(Driver::DRV8825)>=2u,"DRV8825 1.9us STEP min rounded up"));
 assert(check(pulseWidthMicroseconds(Driver::Invalid)==0u,"invalid timing rejected"));
 std::printf("HOST_CPP_PROFILE_GATE=%d/%d PASS; not hardware validation; not firmware compiled\n",pass,pass);
}
