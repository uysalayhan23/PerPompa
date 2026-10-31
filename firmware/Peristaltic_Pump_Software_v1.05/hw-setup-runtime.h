#ifndef PERPOMPA_HW_SETUP_RUNTIME_H
#define PERPOMPA_HW_SETUP_RUNTIME_H
// HW-SETUP-01B: firmware-ready but NOT integrated into active .ino.
// Depends only on the accepted pure profile validator. EEPROM must support read/update.
#include "hw-setup-profile.h"
#include <stdint.h>
#include <stddef.h>

namespace perpompa_hw {
static const uint16_t kSetupArmAddress = 64;  // Outside v1.04 EEPROM 0..21 and profile 32..59.
static const uint8_t kSetupArmed = 0x6c;
static const uint8_t kSetupLocked = 0x00;

enum StartPath : uint8_t {
  kLocalDose, kLocalPump, kLocalCal, kRemoteDose, kRemotePump, kRemoteCal,
  kUsbDose, kUsbPump, kUsbCal, kStartPathCount
};
enum Page : uint8_t { kBoardPage, kDriverPage, kConfirmPage, kPageCount };

template<class Eeprom>
class SafeSetup {
public:
  SafeSetup(Eeprom& storage, uint32_t fcpu) : eeprom_(storage), fcpu_(fcpu),
      draft_(suggestedProfile()), editing_(false), blocked_(false),
      page_(kBoardPage) {}

  void boot() {
    editing_=false; blocked_=false; page_=kBoardPage;
    draft_=suggestedProfile();
    Profile saved; uint8_t gen=0, slot=0;
    if(readLatest(saved,gen,slot)) draft_=saved;
    draft_.confirmed=false; // Display selection never grants authorization.
  }

  // Enter configuration only with all controls idle. Write the independent inhibit
  // byte *before* allowing any selection change. A partially written byte during
  // brownout has no physical atomicity guarantee: hardware brownout gate is REVIEW.
  bool enter(bool motorIdle,bool remoteIdle,bool usbIdle) {
    if(editing_ || !mayEdit(motorIdle,remoteIdle,usbIdle)) return false;
    blocked_=true;
    eeprom_.update(kSetupArmAddress,kSetupLocked);
    if(eeprom_.read(kSetupArmAddress)!=kSetupLocked) return false;
    Profile saved; uint8_t gen=0, slot=0;
    draft_=readLatest(saved,gen,slot) ? saved : suggestedProfile();
    draft_.confirmed=false; editing_=true; page_=kBoardPage;
    return true;
  }

  bool chooseBoard(uint8_t board) {
    if(!editing_ || (board!=kProMini5V16MHz && board!=kProMini3V3_8MHz))return false;
    draft_=changedSelection(draft_,board,draft_.driver);draft_.confirmed=false;
    return true;
  }
  bool chooseDriver(uint8_t driver) {
    if(!editing_ || (driver!=kA4988 && driver!=kDRV8825))return false;
    draft_=changedSelection(draft_,draft_.board,driver);draft_.confirmed=false;
    return true;
  }
  bool nextPage(){if(!editing_)return false;page_=static_cast<Page>((page_+1)%kPageCount);return true;}
  void cancel(){editing_=false;blocked_=true;draft_.confirmed=false;}

  // Explicit physical confirmation must come from a dedicated UI action, not
  // from selecting the A4988 suggestion or simply changing menu pages.
  bool confirmAndSave(bool physicalConfirmation,bool motorIdle,bool remoteIdle,bool usbIdle) {
    if(!editing_ || page_!=kConfirmPage ||
       !mayConfirm(draft_,fcpu_,physicalConfirmation,motorIdle,remoteIdle,usbIdle))return false;
    if(eeprom_.read(kSetupArmAddress)!=kSetupLocked)return false;
    Profile older;uint8_t gen=0,slot=0;
    const bool previous=readLatest(older,gen,slot);
    const uint16_t dest=(previous && slot==0)?kSlot1Address:kSlot0Address;
    Profile toSave=draft_;toSave.confirmed=true;
    uint8_t encoded[kRecordLength];
    if(!encodePending(toSave,previous?static_cast<uint8_t>(gen+1):0,encoded))return false;
    // Invalidate selected slot FIRST; data then written, marker as LAST operation.
    eeprom_.update(dest+11,kSetupLocked);
    if(eeprom_.read(dest+11)!=kSetupLocked)return false;
    for(uint8_t i=0;i<11;i++)eeprom_.update(dest+i,encoded[i]);
    for(uint8_t i=0;i<11;i++)if(eeprom_.read(dest+i)!=encoded[i])return false;
    eeprom_.update(dest+11,kCommitMarker);
    if(eeprom_.read(dest+11)!=kCommitMarker)return false;
    Profile verify;uint8_t verifyGen=0,verifySlot=0;
    if(!readLatest(verify,verifyGen,verifySlot)||!sameProfile(verify,toSave)||
       verifySlot!=(dest==kSlot0Address?0:1))return false;
    eeprom_.update(kSetupArmAddress,kSetupArmed);
    if(eeprom_.read(kSetupArmAddress)!=kSetupArmed)return false;
    editing_=false;blocked_=false;draft_=toSave;
    return true;
  }

  // Intended as a central guard at ALL local, remote and USB pulse paths.
  // Merely including this header has NO effect on firmware, hence integration NOT_RUN.
  bool permit(StartPath path, bool stopRequested) const {
    if(path>=kStartPathCount||stopRequested||editing_||blocked_||
       eeprom_.read(kSetupArmAddress)!=kSetupArmed)return false;
    Profile stored;uint8_t gen=0,slot=0;
    return readLatest(stored,gen,slot)&&stored.confirmed&&cpuMatches(stored,fcpu_);
  }

  // Two exactly 16-character LCD rows; these are strings for the existing LCD
  // adapter to display, never calls lcd or activates a motor.
  void lcdRows(char top[17],char bottom[17]) const {
    if(!top||!bottom)return;
    if(!editing_){
      Profile stored;uint8_t gen=0,slot=0;
      const bool valid=eeprom_.read(kSetupArmAddress)==kSetupArmed &&
        readLatest(stored,gen,slot)&&stored.confirmed&&cpuMatches(stored,fcpu_);
      put(top,valid?"HW SETUP READY":"SETUP REQUIRED");
      put(bottom,valid?"A4988/DRV CHECK":"MOTOR LOCKED");
    } else if(page_==kBoardPage){
      put(top,"HW Setup: Board");
      put(bottom,draft_.board==kProMini5V16MHz?"5V / 16MHz":
        draft_.board==kProMini3V3_8MHz?"3V3 / 8MHz":"Select board");
    } else if(page_==kDriverPage){
      put(top,"HW Setup: Driver");
      put(bottom,draft_.driver==kDRV8825?"DRV8825":"A4988 (suggest)");
    } else {
      put(top,"Confirm physical");
      put(bottom,cpuMatches(draft_,fcpu_)?"Press to confirm":"BOARD MISMATCH");
    }
  }
  bool editing() const {return editing_;}
  bool blocked() const {return blocked_;}
  const Profile& draft() const {return draft_;}
  Page page() const {return page_;}
private:
  Eeprom& eeprom_;uint32_t fcpu_;Profile draft_;bool editing_,blocked_;Page page_;
  bool readLatest(Profile& p,uint8_t& gen,uint8_t& slot) const {
    uint8_t a[kRecordLength],b[kRecordLength];
    for(uint8_t i=0;i<kRecordLength;i++){
      a[i]=eeprom_.read(kSlot0Address+i);b[i]=eeprom_.read(kSlot1Address+i);
    }
    return loadNewest(a,b,&p,&gen,&slot);
  }
  static void put(char* dst,const char* text) {
    uint8_t i=0;for(;i<16 && text[i];i++)dst[i]=text[i];
    for(;i<16;i++)dst[i]=' ';
    dst[16]='\0';
  }
};
} // namespace perpompa_hw
#endif
