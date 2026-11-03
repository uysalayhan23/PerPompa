# P0-04 — Flash LCD QA adayı: gerçek v1.06 setup()/loop() yeni native regresyonu

**Tarih:** 2026-11-03  
**Sonuç:** `HOST_FULL_SOURCE_TARGETED_PASS` · `AVR_PRIOR_SAME_SHA_2/2_PASS` · `ALL_PATHS_STACK_PEAK_REVIEW` · `PHYSICAL_NOT_RUN` · `RELEASE_BLOCKED`.

## Kaynak, kapsam ve yeniden üretim

- Geçici MergedGates kurtarma kaynağı .ino SHA256: `e0d852757f74fedc4c00dd9da05b9cf02bf660c72bf7dcaa4afc52c8a7710e21`.
- Beş satırlık LCD `F()` Flash metin onarımından sonra **test edilen tam .ino SHA256**: `5c7009477ba56cb70850f22ece9860fcb222bd4c0a0c3a10622c21b66c79bfa5`. Kaynakta yalnız LCD sabit metinleri taşındı.
- QA arşivi: `qa/p004/flash_fullsource_native_qa.tar.gz`; SHA256 `2326f2bff168a6a165f93bafcb393ab91d333d38fb8dff7d2affaf36ec2cd539`. İçerik: `host_prelude.cpp`, `new_flash_fullsource_cases.inc`, GCC/Clang UBSan ve ASan+UBSan koşu betikleri, README.
- **Tek komut:** depo kökünden `bash qa/p004/run_flash_fullsource_native_review.sh`. SHA256 doğrular; geçici klasörde gerçek `setup()/loop()` gövdeli C++ adayını oluşturur. Yerel GCC/Clang gerekir. Üretim dosyalarını değiştirmez; cihaz programlamaz.

## Önceki AVR / SRAM ölçümlerini tekrarlamadan yeni regresyon

| Kapı | Bu çalışma |
|---|---|
| Yeni gerçek .ino kaynaklı host senaryoları, GCC/Clang × 8/16 MHz | **48/48 PASS, UBSan** |
| Aynı yeni senaryolar, ayrı ASan+UBSan derleme ve yürütüm | **48/48 PASS** |
| Bozuk LCD metni / çıkış temizleme negatif mutantları | **20/20 yakalandı, UBSan** |
| Aynı kasıtlı bozulmalar, ASan+UBSan | **20/20 yakalandı** |
| Eski 110 B SRAM AVR A/B ve 95→205 B simAVR karşılaştırması | **Önceki Actions PASS korunuyor; yeniden çalıştırılmadı** |
| Bütün motor / EEPROM / ISR / USB eşzamanlı en kötü-yol gerçek stack güvenliği | **REVIEW / NOT_RUN** |

İncelenen 12 yeni uçtan-uca host durumu: legacy Save Sett. uyarısı; recovery notice; reboot notice; service hold; zorunlu reboot sırasında seri/Remote/STEP kilidi; menüden çıkarken overlay sıfırlama; LCD init fail-closed; Remote'un legacy review sırasında reddi; fiziksel onayla eski kalibrasyon kaydı; bekleyen USB verisiyle onaysız kaydın reddi; çıkış menüsünün on boşlukla temizlenmesi; 11 menü türünde dinamik satır biçimlendirme. Negatifler beş farklı literal bozulmasını tespit eder. Mock ile gerçek `F()` Flash adres uzayı tam taklit edilmez; aynı SHA'lı adayın AVR derlemesi daha önce `41645418905` koşusunda PASS olmuştur.

## Stack / heap güvenlik kapısı — ölçülmüş olan ve eksik olan

Önceki [gerçek AVR/simAVR A/B kabulünde](P004_SRAM_FLASH_LCD_QA_20261103.md) geçici aday için 8/16 MHz statik SRAM **1.436→1.326 bayt**, simüle en dar stack–heap boşluğu **95→205 bayt**, Flash +78 bayt. A/B simülasyonu **tam worst-case değildir**.

Önceki ayrı `-fno-lto -fstack-usage` AVR tanı derlemesinde **368 çerçeve kaydı** vardı. En büyük tekil kayıtlar: EEPROM `Store::save` 101 B, `format_menu_line` 76 B (dynamic,bounded), `update_lcd` 66 B, `loop` 54 B, `SafeSetup::confirmAndSave` 44 B. Bu veriler **önceki bazın optimize edilmemiş** tanı derlemesindendir; güvenli çağrı-yığını sınırına toplanamaz. Özellikle EEPROM kayıt/kurtarma, karmaşık LCD çizimi, UART girişleri ve timer kesmeleri aynı anda gerçekleşen fiziksel kombinasyon için **AVR stack high-water kanıtı yoktur**. 205 baytlık gözlenen boşluk kabul edilmiş bir endüstriyel güvenlik marjı değildir.

**Politika kararı:** Test paketi ve kaynak yalnız `qa/p004` altında tutulur; ayrı onay, tam stack/heap ve fiziksel kabul olmadan üretim .ino'suna taşınmaz. v1.06 production blob `957f963c1d4f59a04f18a896e9f6c92890061a49` korunur. `PHYSICAL_NOT_RUN`, `RELEASE_BLOCKED`, v1.04/v1.05 kilitleri, saatlik otomasyon değişmez.
