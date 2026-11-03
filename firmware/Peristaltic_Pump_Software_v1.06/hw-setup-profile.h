#ifndef PERPOMPA_HW_SETUP_PROFILE_H
#define PERPOMPA_HW_SETUP_PROFILE_H
// Pure, AVR-friendly configuration validation component. Not wired into v1.05 .ino.
// Physical board, microstep wiring, and driver identity cannot be sensed here.
#include <stdint.h>

namespace perpompa_hw {
static const uint8_t kUnknown = 255;
static const uint8_t kA4988 = 0;
static const uint8_t kDRV8825 = 1;
static const uint8_t kProMini5V16MHz = 0;
static const uint8_t kProMini3V3_8MHz = 1;
static const uint8_t kRequiredMicrosteps = 4;
static const uint8_t kRecordLength = 12;
static const uint8_t kRecordVersion = 1;
static const uint8_t kCommitMarker = 0xA5;
// Legacy EEPROM uses addresses 0..21. Reserve independent two-slot region.
static const uint16_t kSlot0Address = 32;
static const uint16_t kSlot1Address = 48;

struct Profile {
  uint8_t board;
  uint8_t driver;
  uint8_t microsteps;
  bool confirmed;
};

inline Profile suggestedProfile() {
  Profile p = {kUnknown, kA4988, kRequiredMicrosteps, false};
  return p;
}
inline bool validSelection(const Profile& p) {
  return (p.board == kProMini5V16MHz || p.board == kProMini3V3_8MHz) &&
         (p.driver == kA4988 || p.driver == kDRV8825) &&
         p.microsteps == kRequiredMicrosteps;
}
inline bool cpuMatches(const Profile& p, uint32_t compiledFcpu) {
  return validSelection(p) &&
    ((p.board == kProMini5V16MHz && compiledFcpu == 16000000UL) ||
     (p.board == kProMini3V3_8MHz && compiledFcpu == 8000000UL));
}
inline bool mayEdit(bool motorIdle, bool remoteIdle, bool usbIdle) {
  return motorIdle && remoteIdle && usbIdle;
}
inline bool mayConfirm(const Profile& p, uint32_t compiledFcpu,
                       bool userExplicitlyConfirmedPhysicalSetup,
                       bool motorIdle, bool remoteIdle, bool usbIdle) {
  return mayEdit(motorIdle, remoteIdle, usbIdle) &&
         userExplicitlyConfirmedPhysicalSetup && cpuMatches(p, compiledFcpu);
}
inline Profile changedSelection(const Profile& previous, uint8_t board, uint8_t driver) {
  Profile result = previous;
  if (board != previous.board || driver != previous.driver) {
    result.board = board;
    result.driver = driver;
    result.confirmed = false;
  }
  return result;
}
inline uint8_t crc8(const uint8_t* b, uint8_t count) {
  uint8_t crc = 0;
  for (uint8_t i = 0; i < count; ++i) {
    crc ^= b[i];
    for (uint8_t bit = 0; bit < 8; ++bit) {
      crc = (crc & 0x80U) ? static_cast<uint8_t>((crc << 1) ^ 0x07U)
                          : static_cast<uint8_t>(crc << 1);
    }
  }
  return crc;
}
// Produces a *pending* record: caller must invalidate destination commit byte
// first, write bytes 0..10, then write commit byte 0xA5 as the final operation.
// This function never writes EEPROM or enables a motor.
inline bool encodePending(const Profile& p, uint8_t generation,
                          uint8_t out[kRecordLength]) {
  if (!out || !validSelection(p)) return false;
  out[0] = 'P'; out[1] = 'H'; out[2] = kRecordVersion; out[3] = kRecordLength;
  out[4] = p.board; out[5] = p.driver; out[6] = p.microsteps;
  out[7] = p.confirmed ? 1 : 0; out[8] = generation;
  out[9] = crc8(out, 9); out[10] = static_cast<uint8_t>(out[9] ^ 0xFFU);
  out[11] = 0; // uncommitted by design
  return true;
}
inline bool decodeCommitted(const uint8_t raw[kRecordLength],
                            Profile* result, uint8_t* generation) {
  if (!raw || !result || !generation) return false;
  if (raw[11] != kCommitMarker || raw[0] != 'P' || raw[1] != 'H' ||
      raw[2] != kRecordVersion || raw[3] != kRecordLength ||
      raw[7] > 1 || raw[9] != crc8(raw, 9) ||
      raw[10] != static_cast<uint8_t>(raw[9] ^ 0xFFU)) return false;
  Profile p = {raw[4], raw[5], raw[6], raw[7] == 1};
  if (!validSelection(p)) return false;
  *result = p; *generation = raw[8];
  return true;
}
inline bool sameProfile(const Profile& a, const Profile& b) {
  return a.board == b.board && a.driver == b.driver &&
         a.microsteps == b.microsteps && a.confirmed == b.confirmed;
}
// Dual-slot recovery. Ambiguous generation distance (128), or disagreeing
// profiles with identical generations, are rejected rather than guessed.
inline bool loadNewest(const uint8_t slot0[kRecordLength],
                       const uint8_t slot1[kRecordLength],
                       Profile* out, uint8_t* generation, uint8_t* slotIndex) {
  if (!out || !generation || !slotIndex) return false;
  Profile a = {}, b = {}; uint8_t seqA = 0, seqB = 0;
  const bool validA = decodeCommitted(slot0, &a, &seqA);
  const bool validB = decodeCommitted(slot1, &b, &seqB);
  if (!validA && !validB) return false;
  if (validA && validB) {
    const uint8_t distance = static_cast<uint8_t>(seqB - seqA);
    if (distance == 0 && !sameProfile(a, b)) return false;
    if (distance == 128) return false;
    if (distance != 0 && distance < 128) {
      *out = b; *generation = seqB; *slotIndex = 1;
      return true;
    }
  }
  if (validA) { *out = a; *generation = seqA; *slotIndex = 0; }
  else { *out = b; *generation = seqB; *slotIndex = 1; }
  return true;
}
inline bool mayStart(const uint8_t slot0[kRecordLength],
                     const uint8_t slot1[kRecordLength],
                     uint32_t compiledFcpu, bool stopRequested, bool setupOpen) {
  Profile p = {}; uint8_t generation = 0, slot = 0;
  return !stopRequested && !setupOpen &&
    loadNewest(slot0, slot1, &p, &generation, &slot) &&
    p.confirmed && cpuMatches(p, compiledFcpu);
}
} // namespace perpompa_hw
#endif
