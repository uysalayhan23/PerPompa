#ifndef PERPOMPA_V106_MODBUS_RTU_FRAME_H
#define PERPOMPA_V106_MODBUS_RTU_FRAME_H
// v1.06 isolated Modbus RTU frame parser. No GPIO, EEPROM, motor actions or Arduino dependency.
// The caller MUST provide actual per-byte arrival timestamps for correct RTU gap handling.
// Polling a blocked serial RX queue does NOT supply trustworthy arrival timestamps.
#include <stdint.h>
#include <stddef.h>
namespace perpompa_v106 {
static const uint8_t kMaxAdu = 64;
struct Frame { uint8_t len; uint8_t adu[kMaxAdu]; };
inline uint16_t crc16(const uint8_t *src, uint8_t count) {
  uint16_t crc=0xffff;
  for(uint8_t i=0;i<count;++i){
    crc^=src[i];
    for(uint8_t j=0;j<8;++j)crc=(crc&1)?(uint16_t)((crc>>1)^0xa001):(uint16_t)(crc>>1);
  }
  return crc;
}
inline bool validCrc(const uint8_t* src, uint8_t len){
  if(!src||len<4)return false;
  const uint16_t c=crc16(src,(uint8_t)(len-2));
  return src[len-2]==(uint8_t)c && src[len-1]==(uint8_t)(c>>8);
}
inline uint32_t charGap1_5(uint32_t baud){
  return baud==0?0UL:baud>19200?750UL:(uint32_t)((16500000ULL+baud-1)/baud);
}
inline uint32_t frameGap3_5(uint32_t baud){
  return baud==0?0UL:baud>19200?1750UL:(uint32_t)((38500000ULL+baud-1)/baud);
}
class Receiver {
  uint8_t addr_, size_;
  uint32_t last_us_, t1_5_, t3_5_;
  bool active_, discard_;
  uint8_t buf_[kMaxAdu];
  bool finish(Frame &out) {
    const bool ok=configurationValid() && !discard_ && size_>=4 && buf_[0]==addr_ && validCrc(buf_,size_);
    if(ok){out.len=size_;for(uint8_t i=0;i<size_;++i)out.adu[i]=buf_[i];}
    active_=discard_=false; size_=0;
    return ok;
  }
public:
  Receiver(uint8_t slaveAddress,uint32_t baud):addr_(slaveAddress),size_(0),last_us_(0),
    t1_5_(charGap1_5(baud)),t3_5_(frameGap3_5(baud)),active_(false),discard_(false){}
  bool configurationValid()const {return addr_>=1&&addr_<=247&&t3_5_>t1_5_;}
  void reset(){active_=discard_=false;size_=0;last_us_=0;}
  bool feed(uint8_t data,uint32_t received_us,Frame &out) {
    bool completed=false;
    if(active_){
      const uint32_t delta=received_us-last_us_; // modulo-2^32, valid for reasonable gaps
      if(delta>=t3_5_)completed=finish(out);
      else if(delta>t1_5_)discard_=true; // invalid inter-character gap; ignore until quiet
    }
    // A bad byte sequence is dropped until a complete frame gap, not resynchronized
    // opportunistically by an in-stream address byte.
    if(!active_){active_=true;discard_=false;size_=0;}
    last_us_=received_us;
    if(size_>=kMaxAdu)discard_=true;
    if(!discard_)buf_[size_++]=data;
    return completed;
  }
  bool idle(uint32_t now_us,Frame &out) {
    return active_ && (uint32_t)(now_us-last_us_)>=t3_5_ && finish(out);
  }
};
} // namespace perpompa_v106
#endif
