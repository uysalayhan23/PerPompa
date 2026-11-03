# PerPompa P0-04 — Yoğun EEPROM işaretçisi / UART RX / Timer1 birleşik AVR QA

> **2026-11-03 kaynak doğrulaması:** [simAVR v1.6 EEPROM zamanlama incelemesi](P004_SIMAVR_EEPROM_TIMING_MODEL_REVIEW_20261103.md) simAVR'nin EEPROM verisini ve EEPE bitini yazma sırasında **hemen** değiştirdiğini doğruladı; 3,4 ms gecikme yalnız ready IRQ'ya uygulanıyor. Bu testin 3.335/3.235 çevrimlik marker→commit süresi **fiziksel EEPROM karşılığı değildir**. Pozitif birleşik ISR `REVIEW`; Remote LOW 2/2 simülasyon PASS ve `RELEASE_BLOCKED` korunur.

**2026-11-03 · Karar: `2/2_REMOTE_LOW_PASS` · `2/2_POSITIVE_REVIEW` · `ALL_PATHS_ISR_STACK_REVIEW` · `PHYSICAL_NOT_RUN` · `RELEASE_BLOCKED`.**

## Gerekçe ve yöntem

Önceki [birleşik ISR raporu](P004_COMBINED_ISR_EEPROM_REVIEW_20261103.md) 8.192 komutta bir yapılan EEPROM gözlemi ve geç recovery witness uyaranı ile yazım sırasında UART RX ISR gözlemleyemedi. Yeni QA aracı `qa/p004/stack_integrated_marker_probe.c`, sentetik menü işlemi sırasında commit işaretçisi (slot A `109`) `0` olduğunda UART RX veri uyaranını başlatır. EEPROM örnekleme **yalnız açılış kayıt geçişinde 64 komutta bir** yapıldı. `qa/p004/run_integrated_marker_avr.sh`, her iki FQBN için pozitif ve Remote LOW negatif çıktıları *fail-fast yapmadan* toplar; CPU'nun `__vector_18` UART RX ve `__vector_13` Timer1 vektörlerine gerçek girişini ve SPH/SPL tamamlanmış stack değerlerini inceler. **Üretim firmware'i veya kayıt algoritması değiştirilmedi.**

Kaynak SHA256: Flash LCD QA `.ino` `5c7009477ba56cb70850f22ece9860fcb222bd4c0a0c3a10622c21b66c79bfa5`. Üretim `.ino` Git blob SHA `957f963c1d4f59a04f18a896e9f6c92890061a49` değişmedi.

**Tek, salt-okunur ve sahibiyle sınırlı Actions koşusu:** [#41673802315](https://github.com/uysalayhan23/PerPompa/actions/runs/41673802315). 15 dakika sınırı, bir iş, `contents:read`, fiziksel cihaz/yükleme/dağıtım yok. **Genel koşu sonucu FAIL**, çünkü gerekli pozitif UART ISR eşzamanlılığı görülmedi.

## Yeni gerçek 8/16 MHz AVR sonucu

| İncelenen ölçüt | 16 MHz pozitif | 8 MHz pozitif | 16 MHz Remote LOW | 8 MHz Remote LOW |
|---|---:|---:|---:|---:|
| Gerçek AVR derleme | PASS | PASS | Aynı ELF | Aynı ELF |
| EEPROM commit | **1** | **1** | **0** | **0** |
| İptal edilen commit marker `0` gözlendi | 1 | 1 | 0 | 0 |
| Marker `0` → commit süresi (sim AVR döngüsü) | **3.335** | **3.235** | uygulanmaz | uygulanmaz |
| Gerçek UART RX ISR, toplam | 7 | 7 | 7 | 7 |
| **UART ISR, marker→commit penceresinde** | **0** | **0** | uygulanmaz | uygulanmaz |
| Timer1 ISR, marker→commit penceresinde | **1** | **1** | uygulanmaz | uygulanmaz |
| Motor STEP HIGH örnekleri | 0 | 0 | 0 | 0 |
| Tamamlanmış-SP stack derinliği (B) | 422 | 422 | 314 | 320 |
| Tamamlanmış-SP en dar gözlenen heap açıklığı (B) | 275 | 275 | 383 | 377 |
| Test kabulü | **REVIEW** | **REVIEW** | **PASS** | **PASS** |

AVR kaynakları 16/8 MHz için sırasıyla **28.010 / 28.006 B Flash** ve **1.326 B statik SRAM** ile derlendi. 16/8 MHz'nin pozitif kayıt örneklerinde timer ISR bir kez yazım penceresinde gerçekleşti, ancak UART ISR aynı pencerede **gösterilemedi**. UART verisi fiziksel UART IRQ girişinde işlenmiş olsa da bu kayıt penceresinde gerçekleşmedi. Bu veri **cihazda eşzamanlı çalışmanın güvenli ya da imkânsız olduğunu kanıtlamaz**.

Bu testteki geçersiz marker→commit döngü aralığı özellikle kısaydı (3.335 / 3.235 CPU döngüsü); bu, simülatördeki EEPROM modelinin zamanlama doğruluğunun ayrıca değerlendirilmesini gerektirir. Yazımın gerçek elektriksel/EEPROM zamanlamasına eşdeğerliği kanıtlanmadı. Sadece UART ISR toplam sayısıyla birleşik yol PASS verilmesi yasaktır.

Negatif Remote LOW denemelerinde 2/2 **motor STEP LOW**, EEPROM commit/marker 0 hiç yok, UART RX ISR mevcut, tamamlanmış SP açıklığı pozitif. Test betiğinin kontrolleri pozitif birleşik yolları doğru şekilde **REVIEW** saydı; önceki host EEPROM/CRC 2/2 PASS kapısını tekrar çalıştırmadı.

## Teslim ve açık kabul

- Geçici `.github/workflows/p004-integrated-marker-once.yml` aktif `main` üzerinden kaldırılır. Kaynak QA aracı, test çıktıları ve zamanlama sorunu bu rapor ve tarihsel Actions koşusunda korunur.
- Yeni tam cihaz kabulü için yazım işleminin gerçek/kalibre edilmiş EEPROM zamanlamasında UART RX ve Timer1 kesmeleri, EEPROM veri bütünlüğü, motor STOP ve tamamlanmış-SP tepe kullanımının birlikte doğrulanması gerekir. Özellikle **simülatörün EEPROM yazma çevrimi fiziksel cihazın tepe stack kabulü yerine geçmez**.
- `ALL_PATHS_ISR_STACK_REVIEW` · `PHYSICAL_NOT_RUN` · `RELEASE_BLOCKED`; firmware promotion, üretim main yazımı, motor kontrol onayı yok.
