#ifndef PERPOMPA_P004_SETTINGS_STORE_H
#define PERPOMPA_P004_SETTINGS_STORE_H
// P0-04: crash-consistent settings storage. No motor control and no Arduino dependencies.
// Legacy 0..21 untouched. HW Setup uses 32..59 and 64; records use 80..109 and 112..141.
#include <stdint.h>
namespace perpompa_p004 {
static const uint8_t kItems=11;
static const uint8_t kBytes=30;
static const uint16_t kSlotA=80;
static const uint16_t kSlotB=112;
static const uint8_t kCommitted=0xa5;
enum ReadResult : uint8_t { kMissing=0, kLoaded=1, kConflict=2 };
static inline uint16_t crc16(const uint8_t *buf, uint8_t n) {
  uint16_t crc=0xffff;
  for(uint8_t i=0;i<n;i++) {
    crc^=(uint16_t)buf[i]<<8;
    for(uint8_t bit=0;bit<8;bit++)
      crc=(crc&0x8000U)?(uint16_t)((crc<<1)^0x1021U):(uint16_t)(crc<<1);
  }
  return crc;
}
template<class Eeprom> class Store {
  Eeprom &ee_;
  struct Record { uint8_t gen; int16_t values[kItems]; };
  void readSlot(uint16_t base, uint8_t bytes[kBytes]) const {
    for(uint8_t i=0;i<kBytes;i++)bytes[i]=ee_.read(base+i);
  }
  bool decode(uint16_t base,Record &out) const {
    uint8_t b[kBytes];readSlot(base,b);
    if(b[29]!=kCommitted||b[0]!='P'||b[1]!='S'||b[2]!=1||b[3]!=kItems)return false;
    const uint16_t crc=crc16(b,27);
    if(b[27]!=(uint8_t)crc||b[28]!=(uint8_t)(crc>>8))return false;
    out.gen=b[4];
    for(uint8_t i=0;i<kItems;i++) {
      uint16_t raw=(uint16_t)b[5+2*i]|((uint16_t)b[6+2*i]<<8);
      if(raw>32767)return false;
      out.values[i]=(int16_t)raw;
    }
    return true;
  }
  static bool same(const Record &a,const Record &b) {
    for(uint8_t i=0;i<kItems;i++)if(a.values[i]!=b.values[i])return false;
    return true;
  }
  ReadResult newest(Record &selected,uint8_t &index) const {
    Record a={},b={}; const bool va=decode(kSlotA,a),vb=decode(kSlotB,b);
    if(!va&&!vb)return kMissing;
    if(va&&vb) {
      const uint8_t d=(uint8_t)(b.gen-a.gen);
      if(d==128||(d==0&&!same(a,b)))return kConflict;
      if(d!=0&&d<128) {selected=b;index=1;return kLoaded;}
    }
    if(va){selected=a;index=0;}else{selected=b;index=1;}
    return kLoaded;
  }
  static void encode(const int vals[kItems],uint8_t gen,uint8_t out[kBytes]) {
    out[0]='P';out[1]='S';out[2]=1;out[3]=kItems;out[4]=gen;
    for(uint8_t i=0;i<kItems;i++) {
      const uint16_t v=(uint16_t)vals[i];
      out[5+2*i]=(uint8_t)v;out[6+2*i]=(uint8_t)(v>>8);
    }
    const uint16_t check=crc16(out,27);
    out[27]=(uint8_t)check;out[28]=(uint8_t)(check>>8);out[29]=kCommitted;
  }
public:
  explicit Store(Eeprom &ee):ee_(ee){}
  ReadResult load(int result[kItems]) const {
    Record best={};uint8_t index=0;
    const ReadResult state=newest(best,index);
    if(state==kLoaded&&result)for(uint8_t i=0;i<kItems;i++)result[i]=best.values[i];
    return state;
  }
  bool save(const int vals[kItems]) {
    if(!vals)return false;
    for(uint8_t i=0;i<kItems;i++)if(vals[i]<0||vals[i]>32767)return false;
    Record last={};uint8_t latestIndex=0;
    const ReadResult state=newest(last,latestIndex);
    if(state==kConflict)return false;
    if(state==kLoaded) {
      bool unchanged=true;
      for(uint8_t i=0;i<kItems;i++)if(last.values[i]!=vals[i]){unchanged=false;break;}
      if(unchanged)return true;
    }
    const uint16_t target=state==kLoaded?(latestIndex==0?kSlotB:kSlotA):kSlotA;
    const uint8_t gen=state==kLoaded?(uint8_t)(last.gen+1U):0;
    uint8_t record[kBytes];encode(vals,gen,record);
    // Invalidate destination first; previously committed slot is not touched.
    ee_.update(target+29,0);
    if(ee_.read(target+29)!=0)return false;
    for(uint8_t i=0;i<29;i++) {
      ee_.update(target+i,record[i]);
      if(ee_.read(target+i)!=record[i])return false;
    }
    // A *single* byte is committed only after all payload/checksum bytes verify.
    ee_.update(target+29,kCommitted);
    if(ee_.read(target+29)!=kCommitted)return false;
    Record verify={};uint8_t index=0;
    if(newest(verify,index)!=kLoaded||index!=(target==kSlotA?0:1)||
       verify.gen!=gen)return false;
    for(uint8_t i=0;i<kItems;i++)if(verify.values[i]!=vals[i])return false;
    return true;
  }
};
}
#endif
