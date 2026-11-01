#ifndef PERPOMPA_P004_MIGRATION_GUARD_CANDIDATE_H
#define PERPOMPA_P004_MIGRATION_GUARD_CANDIDATE_H
// TEMPORARY PROPOSAL ONLY. NOT IN FIRMWARE. DO NOT FLASH.
// Double witness written BEFORE first new V2/P0-04 EEPROM save.
// Legacy 0..21, HW profile 32..59/64, current slots 80..109 and 112..141.
#include "p0-04-settings-store.h"
#include <stdint.h>
namespace p004_migration_candidate {
static const uint16_t kWitnessA=144, kWitnessB=160;
static const uint8_t kWitnessSize=8;
static const uint8_t kWitnessCommit=0xa5;
enum BootKind : uint8_t { kNewReady, kLegacyNeedsReview, kLocked };
enum WitnessKind : uint8_t { kVirgin, kValid, kDamaged };

template<class Eeprom> class Gate {
 Eeprom &ee_;
 perpompa_p004::Store<Eeprom> store_;
 static uint8_t expectedByte(uint8_t i) {
   // Magic, version, inverse signature, CRC8 of first 6 bytes, final commit.
   static const uint8_t b[6]={'P','M','2','G',1,0x5a};
   if(i<6)return b[i];
   if(i==6){uint8_t crc=0;for(uint8_t j=0;j<6;j++){
     crc^=b[j];for(uint8_t k=0;k<8;k++)crc=(crc&0x80)?(uint8_t)((crc<<1)^7):(uint8_t)(crc<<1);
   }return crc;}
   return kWitnessCommit;
 }
 WitnessKind inspect(uint16_t base) const {
   bool allVirgin=true,allValid=true;
   for(uint8_t i=0;i<kWitnessSize;i++) {
     const uint8_t v=ee_.read(base+i);
     if(v!=0xff)allVirgin=false;
     if(v!=expectedByte(i))allValid=false;
   }
   return allVirgin?kVirgin:(allValid?kValid:kDamaged);
 }
 bool slotsVirgin() const {
   for(uint8_t i=0;i<perpompa_p004::kBytes;i++) {
     if(ee_.read(perpompa_p004::kSlotA+i)!=0xff ||
        ee_.read(perpompa_p004::kSlotB+i)!=0xff)return false;
   }
   return true;
 }
 bool witnessValid() const {
   return inspect(kWitnessA)==kValid && inspect(kWitnessB)==kValid;
 }
 bool witnessVirgin() const {
   return inspect(kWitnessA)==kVirgin && inspect(kWitnessB)==kVirgin;
 }
 bool writeOne(uint16_t base){
   ee_.update(base+7,0);
   if(ee_.read(base+7)!=0)return false;
   for(uint8_t i=0;i<7;i++){
     ee_.update(base+i,expectedByte(i));
     if(ee_.read(base+i)!=expectedByte(i))return false;
   }
   ee_.update(base+7,kWitnessCommit);
   return ee_.read(base+7)==kWitnessCommit;
 }
 bool ensureWitness(){
   if(witnessValid())return true;
   if(!witnessVirgin())return false;
   if(!writeOne(kWitnessA))return false;
   if(!writeOne(kWitnessB))return false;
   return witnessValid();
 }
public:
 explicit Gate(Eeprom &ee):ee_(ee),store_(ee){}
 BootKind boot(int v[perpompa_p004::kItems]) const {
   // Existing P0-04 records with no witness are accepted for compatibility.
   // Any damaged/nonmatching witness denies automatic start.
   if(!witnessValid()&&!witnessVirgin())return kLocked;
   int tmp[perpompa_p004::kItems]={};
   const perpompa_p004::ReadResult r=store_.load(tmp);
   if(r==perpompa_p004::kConflict)return kLocked;
   if(r==perpompa_p004::kLoaded){
     if(v)for(uint8_t i=0;i<perpompa_p004::kItems;i++)v[i]=tmp[i];
     return kNewReady;
   }
   if(!witnessVirgin()||!slotsVirgin())return kLocked;
   // Caller MUST display legacy values, verify against physical calibration,
   // and require explicit operator acknowledgement BEFORE allowing motor start.
   return kLegacyNeedsReview;
 }
 bool save(const int v[perpompa_p004::kItems],bool explicitReview) {
   const BootKind state=boot(nullptr);
   if(state==kLocked||!v||(state==kLegacyNeedsReview&&!explicitReview))return false;
   // Reject invalid payloads BEFORE a durable migration witness is written.
   for(uint8_t i=0;i<perpompa_p004::kItems;i++)if(v[i]<0||v[i]>32767)return false;
   if(!ensureWitness())return false;
   return store_.save(v);
 }
 WitnessKind witnessA() const{return inspect(kWitnessA);}
 WitnessKind witnessB() const{return inspect(kWitnessB);}
};
} // namespace p004_migration_candidate
#endif
