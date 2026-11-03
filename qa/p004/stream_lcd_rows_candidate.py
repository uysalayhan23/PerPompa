#!/usr/bin/env python3
"""P0-04 QA only: make update_lcd use one 17-byte row buffer. No firmware upload."""
from pathlib import Path
import hashlib,sys
BASE_SHA="5c7009477ba56cb70850f22ece9860fcb222bd4c0a0c3a10622c21b66c79bfa5"
RESULT_SHA="fb02070a91a6420a53382ca73e2406898f0f6a45cb817e1c9969bce0e809a8fb"
OLD="""  char desired[2][LCD_COLUMNS+1];
  format_menu_line(menu_number_1, desired[0]);
  format_menu_line(menu_number_2, desired[1]);
  for (int row=0; row<LCD_ROWS; row++){
    for (int col=0; col<LCD_COLUMNS; col++){
      if (!lcd_cache_valid || desired[row][col] != lcd_shown[row][col]){
        lcd.setCursor(col, row);
        lcd.write(desired[row][col]);
        lcd_shown[row][col] = desired[row][col];
      }
    }
  }
"""
NEW="""  // A single row buffer avoids holding two full LCD lines on the AVR stack.
  char desired[LCD_COLUMNS+1];
  for (int row=0; row<LCD_ROWS; row++){
    format_menu_line(row == 0 ? menu_number_1 : menu_number_2, desired);
    for (int col=0; col<LCD_COLUMNS; col++){
      if (!lcd_cache_valid || desired[col] != lcd_shown[row][col]){
        lcd.setCursor(col, row);
        lcd.write(desired[col]);
        lcd_shown[row][col] = desired[col];
      }
    }
  }
"""
if __name__=="__main__":
    if len(sys.argv)!=2:raise SystemExit("Usage: stream_lcd_rows_candidate.py /path/to/COPIED_QA.ino")
    p=Path(sys.argv[1]);data=p.read_bytes()
    assert hashlib.sha256(data).hexdigest()==BASE_SHA,"Wrong base source; no write"
    assert data.count(OLD.encode())==1,"Missing/ambiguous source anchor"
    candidate=data.replace(OLD.encode(),NEW.encode())
    assert hashlib.sha256(candidate).hexdigest()==RESULT_SHA,"Patched SHA mismatch; no write"
    p.write_bytes(candidate)
    print("P004_ROW_STREAM_QA_SHA256_PASS",RESULT_SHA)
