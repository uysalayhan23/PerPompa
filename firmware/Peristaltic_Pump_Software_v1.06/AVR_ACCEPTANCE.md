# PerPompa v1.06 I²C LCD — Windows AVR kabul kanıtı

**Kanıt kaynağı:** 2026-11-03 tarihinde Windows PowerShell'de çalıştırılan `Build-And-Compile-I2C.ps1` çıktısı kullanıcı tarafından paylaşıldı. Bu metin, gerçek cihaz üzerinde yapılan test değildir.

- `PINNED_V105_SHA=b737292fdf33eec5aa62dac662d698f0dc705c01`
- `V106_I2C_ONLY_SHA256=4FD0D926FBF622DDA9DA26A3FB3AD4306C4D2DF51474A0441927F185990D65BB`
- `V106_REVIEW_GIT_BLOB_SHA=7933e4fe697a8778603ba553eb620e7788dc5ede`
- `NON_LCD_DIFF=ZERO; FORBIDDEN_COMMUNICATION=NONE`
- `16MHz AVR PASS`: Sketch 27486 / 30720 bayt (Flash), global 1367 / 2048 bayt (SRAM), 681 bayt kalan
- `8MHz AVR PASS`: Sketch 27482 / 30720 bayt (Flash), global 1367 / 2048 bayt (SRAM), 681 bayt kalan
- `I2C_ONLY_AVR=2/2 PASS`, `AVR TEST CIKIS KODU: 0`

**Kaynak sürekliliği:** GitHub'a eklenen .ino blob'u kullanıcı Windows AVR çıktısındaki `7933e4fe...` değeriyle tam eşleşir; altı header v1.05'in değişmeyen Git bloblarıyla kopyalanmıştır.

**Bağımlılıklar:** Arduino CLI 1.5.2-rc.1; arduino:avr 1.8.8; hd44780 1.3.2; TimerOne 1.1.1; ClickEncoder. Derlemede LCD için `Wire` kullanılır.

**HOLD:** Gerçek I²C LCD pin eşlemesi, kontrast, açılış ve menü, USB/Remote/STEP ve EEPROM davranışı fiziksel cihazda doğrulanmadı. AVR derlemesi başarıyla tamamlandı, ancak bu güvenlik/ürün kabulü değildir. `PHYSICAL_NOT_RUN` / `RELEASE_BLOCKED`.

## 2026-11-03: LCD başlatma hatasında fail-closed düzeltmesi

**Kanıt kaynağı:** Kullanıcının Windows PowerShell üzerinden paylaştığı `Run-AVR-Preflight.ps1` gerçek Arduino CLI çıktısı.

- Önceki v1.06 kaynak Git blob: `7933e4fe697a8778603ba553eb620e7788dc5ede`
- Yeni AVR ölçümüyle birebir eşleşen kaynak blob: `55b19a8b1017bf1924dc568439f920ec9c90064f`
- `PATCH=THREE_LCD_ONLY_EDITS; INVERSE_DIFF=ZERO; NO_MODBUS_RS485=PASS`
- 16MHz AVR: Flash **27540 / 30720** bayt; statik SRAM **1368 / 2048**, boş **680** bayt — **PASS**
- 8MHz AVR: Flash **27536 / 30720** bayt; statik SRAM **1368 / 2048**, boş **680** bayt — **PASS**
- `NEW_LCD_FAILCLOSED_AVR=2/2`, çıkış kodu `0`

LCD'nin `begin()` çağrısı hata döndürürse açılış kodu motor çıkışlarını pasifleştirir; ana döngüde START/USB/Remote kontrolüne girilmeden motor pasif tutulur. Bu davranışın kaynak testi AVR derleme PASS değildir; motorun fiziksel olarak emniyetli duruma geçtiği **henüz ölçülmedi**. Fiziksel I²C veri yolu takılması, besleme ve gerçek EEPROM geçişi ayrıca inceleme gerektirir.

`PHYSICAL_NOT_RUN`, `SRAM_STACK_NOT_RUN`, `EEPROM_P004_REVIEW`, `RELEASE_BLOCKED` korunur.
