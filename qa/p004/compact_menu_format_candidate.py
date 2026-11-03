#!/usr/bin/env python3
"""P0-04 QA only: compact one 16-character LCD line without a 48-byte temporary buffer."""
from pathlib import Path
import hashlib
import sys

BASE_SHA = "5c7009477ba56cb70850f22ece9860fcb222bd4c0a0c3a10622c21b66c79bfa5"
NEW_SHA = "87fb708ca17e33cea2d13606344605d85768b200c5d8d182e4c30c05a4d3bd71"

OLD = r"""void format_menu_line(int index, char *line){
  char full[48];
  snprintf(full, sizeof(full), "%d|%s", index, menu[index].name_);
  if (menu[index].type == VALUE){
    value_dbl = menu[index].value / pow(10, menu[index].decimals);
    dtostrf(value_dbl, VALUE_MAX_DIGITS, menu[index].decimals, value_str);
    strncat(full, " ", sizeof(full) - strlen(full) - 1);
    strncat(full, value_str, sizeof(full) - strlen(full) - 1);
    strncat(full, menu[index].suffix, sizeof(full) - strlen(full) - 1);
  } else if (menu[index].type == OPTION){
    strncat(full, " ", sizeof(full) - strlen(full) - 1);
    strncat(full, menu[index].options[menu[index].value], sizeof(full) - strlen(full) - 1);
  }
  memset(line, ' ', LCD_COLUMNS);
  line[LCD_COLUMNS] = '\0';
  size_t count = strlen(full);
  if (count > LCD_COLUMNS) count = LCD_COLUMNS;
  memcpy(line, full, count);
}"""

NEW = r"""void format_menu_line(int index, char *line){
  // The LCD displays only 16 characters. Avoid a separate 48-byte stack buffer.
  snprintf(line, LCD_COLUMNS + 1, "%d|%s", index, menu[index].name_);
  if (menu[index].type == VALUE){
    value_dbl = menu[index].value / pow(10, menu[index].decimals);
    dtostrf(value_dbl, VALUE_MAX_DIGITS, menu[index].decimals, value_str);
    strncat(line, " ", LCD_COLUMNS - strlen(line));
    strncat(line, value_str, LCD_COLUMNS - strlen(line));
    strncat(line, menu[index].suffix, LCD_COLUMNS - strlen(line));
  } else if (menu[index].type == OPTION){
    strncat(line, " ", LCD_COLUMNS - strlen(line));
    strncat(line, menu[index].options[menu[index].value], LCD_COLUMNS - strlen(line));
  }
  const size_t used = strlen(line);
  memset(line + used, ' ', LCD_COLUMNS - used);
  line[LCD_COLUMNS] = '\0';
}"""

if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit("Usage: compact_menu_format_candidate.py /path/to/COPIED_QA.ino")
    target = Path(sys.argv[1])
    data = target.read_bytes()
    assert hashlib.sha256(data).hexdigest() == BASE_SHA, "Unexpected input source; no write"
    assert data.count(OLD.encode()) == 1, "Source function anchor missing or ambiguous"
    result = data.replace(OLD.encode(), NEW.encode())
    assert hashlib.sha256(result).hexdigest() == NEW_SHA, "QA candidate SHA mismatch"
    target.write_bytes(result)
    print("QA_COMPACT_FORMAT_SHA_PASS", NEW_SHA)
