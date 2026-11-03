#!/usr/bin/env python3
"""P0-04 review only: move LCD-only constant messages to AVR flash. No firmware upload."""
from pathlib import Path
import hashlib
import sys

BASE_SHA256 = "e0d852757f74fedc4c00dd9da05b9cf02bf660c72bf7dcaa4afc52c8a7710e21"
EXPECTED_SHA256 = "5c7009477ba56cb70850f22ece9860fcb222bd4c0a0c3a10622c21b66c79bfa5"
PATCHES = (
    (b'lcd.print("VERIFY CAL SETS ");', b'lcd.print(F("VERIFY CAL SETS "));'),
    (b'lcd.print("HOLD TO CONFIRM ");', b'lcd.print(F("HOLD TO CONFIRM "));'),
    (b'lcd.print("          ");', b'lcd.print(F("          "));'),
    (b'lcd.print(recoveryUiNotice==1 ? "RECOVERY REVIEW " : "SERVICE HOLD    ");',
     b'lcd.print(recoveryUiNotice==1 ? F("RECOVERY REVIEW ") : F("SERVICE HOLD    "));'),
    (b'lcd.print(recoveryUiNotice==1 ? (recoveryRebootRequired ? "REBOOT TO CHECK " : "VERIFY CAL SETS ") : "DO NOT ERASE    ");',
     b'lcd.print(recoveryUiNotice==1 ? (recoveryRebootRequired ? F("REBOOT TO CHECK ") : F("VERIFY CAL SETS ")) : F("DO NOT ERASE    "));'),
)

def sha(data):
    return hashlib.sha256(data).hexdigest()

def patch(data):
    assert sha(data) == BASE_SHA256, "Unexpected input source SHA256 (stop without changes)"
    output = data
    for old, new in PATCHES:
        assert output.count(old) == 1, "Missing or ambiguous source anchor: " + repr(old)
        output = output.replace(old, new)
    assert sha(output) == EXPECTED_SHA256, "Candidate SHA256 mismatch"
    assert len(output) - len(data) == 24
    return output

def main():
    assert len(sys.argv) == 2, "Usage: flash_lcd_literals_candidate.py /path/to/copied/QA.ino"
    target = Path(sys.argv[1])
    source = target.read_bytes()
    result = patch(source)
    target.write_bytes(result)
    print("BASELINE_SHA256=" + sha(source))
    print("CANDIDATE_SHA256=" + sha(result))
    print("PATCHES=5;LITERAL_CONTENTS_UNCHANGED;NO_FIRMWARE_UPLOAD")

if __name__ == "__main__":
    main()
