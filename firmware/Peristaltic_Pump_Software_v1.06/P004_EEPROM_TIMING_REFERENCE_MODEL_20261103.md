# P0-04 — Bağımsız EEPROM zamanlama referans modeli (2026-11-03)

**Kabul: `REFERENCE_MODEL_QA_9/9_PASS`; `FIRMWARE_ISR_OVERLAP_REVIEW`; `STACK_ALL_PATHS_REVIEW`; `PHYSICAL_NOT_RUN`; `RELEASE_BLOCKED`.**

## Dayanak ve hedef

Microchip ATmega328P datasheet, EEPROM *atomic erase and write* için **tipik 3,4 ms/bayt**, `EEMPE` etkinleştirme sonrası **4 CPU çevrimlik** yazma penceresi ve `EEPE` meşgul bitinin işlem bitene kadar aktif kalması davranışını açıklar:
https://ww1.microchip.com/downloads/en/devicedoc/atmel-7810-automotive-microcontrollers-atmega328p_datasheet.pdf

SimAVR v1.6 `avr_eeprom.c` önceki kaynak incelemesinde veri ve `EEPE` bitini EECR callback sırasında *hemen* güncelliyor, 3400 µs süreyi yalnız EEPROM-ready IRQ için planlıyor. Bu nedenle simAVR'nin önceki marker→commit 3335/3235 CPU çevrimleri gerçek EEPROM yazma süresi kanıtı olamaz. [Kaynak bazlı inceleme](P004_SIMAVR_EEPROM_TIMING_MODEL_REVIEW_20261103.md).

## Yalnız QA kaynakları

- `qa/p004/p004_eeprom_timing_reference.py`: bağımsız, saf Python durum makinesi. EEPROM yazma isteği, 4 çevrim `EEMPE` kapısı, `EEPE` meşgul durumu, tipik 3400 µs gecikmiş commit; UART_RX_ISR ve TIMER1_ISR bağımsız olay kaydı; kesilmiş yazma `RECOVERY_REVIEW` olarak kapatılır.
- `qa/p004/test_p004_eeprom_timing_reference.py`: zaman çizelgesi eşdeğerlik testleri ve kasıtlı olumsuz durumlar; canlı firmware veya fiziksel UART/EEPROM ile bağlantı **yok**.
- Üretilen QA modülü Git blob `c2b47c176c815f1e6c506d345007f1bda4724edf`, SHA256 `e87a50610835dc4ba433dc842a363ab44fa5ea8cef7a9dedbcb997eceba4b891`.
- Test kaynağı Git blob `529a151b0ad54c44463e40c3ef149aa9e41c9823`, SHA256 `7dae7d16c9966d112eae1a7e9bc9d7d580d94dfbed83eb7eaa517bac9a3ca2ff`.

## Yeni QA pozitif-negatif doğrulaması

Yerel Python 3.13.5, `python -m unittest -q test_p004_eeprom_timing_reference.py` ve `py_compile`: **9/9 test PASS**. 8 MHz ve 16 MHz birlikte çalıştırılmış pozitif durum (iki frekans); ayrı negatif durumlar: erken EEPROM veri görünürlüğünün reddi, paralel yazmanın engellenmesi, `EEMPE` 4-çevrim aşımının reddi, UART yokken birleşik çakışma ilan edilmeyişi, UART commit sonrası geldiğinde sayılmayışı, güç kesintisi sonucu kayıt kabul edilmemesi, geçersiz IRQ/adres ve desteklenmeyen frekans. Model hesaplaması tipik 3,4 ms için **27.200 çevrim (8 MHz)** / **54.400 çevrim (16 MHz)**.

**Önemli kabul ayrımı:** Modelde iki IRQ olayı programlanıp `MODEL_EVENT_OVERLAP` görülmesi **özgün v1.06 `setup()/loop()` veya gerçek ATmega328P ISR overlap PASS değildir**. Test gerçek zaman jitter/toleransını, endüstriyel güç kaybında kısmi EEPROM içeriğini, watchdog/reset, motor ISR yükünü, gerçek çağrı yığınını, EEPROM write-mode farklılıklarını ya da tüm yolların SPH/SPL ile tamamlanmış stack minimumunu simüle etmez. Test fixture'ında `motor_step_high=False` sabit tutulur; gerçek firmware'in STEP çıkışını ölçmez. Her durumda `physical_acceptance=REVIEW`, `full_stack_acceptance=REVIEW`.

## Sıradaki **tek** donanım/AVR kabul planı — henüz NOT_RUN

1. Önce simAVR EEPROM sürücüsünün EEPE bitini yazım boyunca koruduğu ve veri commit'ini geciktirdiği bağımsız model/driver doğrulaması; nominal 3,4 ms parametre, tolerans aralığı ve farklı EEPROM mode'ları açıkça yazılmalı.
2. **Aynı özgün QA `.ino` SHA** ve 16/8 MHz AVR ELF üzerinde, doğrulanmış driver ile UART RX ve Timer1 kesmelerini EEPROM meşgul penceresi içinde PC/vektör geçişleriyle doğrula; gönderilen UART baytı tek başına ISR kabulü değildir.
3. CRC, çift-slot atomik commit, Remote LOW engeli, STEP güvenliği, kesinti sırasında kayıt, watchdog ve tamamlanmış-SP/heap en düşük açıklığını gerçek kodla çapraz test et; bozuk CRC, kesilmiş commit, UART gecikmesi ve kayıp Timer ISR mutantları olmadan PASS verme.
4. Donanım üzerinde uzman gözetimli ölçümler ayrı `PHYSICAL_NOT_RUN` kapısından geçmeli; model ve AVR simülasyonu endüstriyel `RELEASE_BLOCKED` kilidini kaldırmaz.

**Teslim sınırı:** Yeni GitHub Actions koşusu yok; saatlik P0-04 otomasyonunun ayarlarına dokunulmadı (en son okumada **paused/disabled** olduğu görüldü). Üretim v1.06 blob `957f963c1d4f59a04f18a896e9f6c92890061a49`, v1.05 politika ve v1.04 arşivi korunur.
