# P0-04 — LCD iki satırdan tek satır tamponuna geçiş: AVR stack incelemesi


> **ÖLÇÜM DÜZELTMESİ — 2026-11-03 (sonraki doğrulama):** 205→189 B ham SP farkı, tamamlanmış SP'ye göre gerçek 16 B stack regresyonu olarak kanıtlanmış değildir. SPH/SPL ara örnekleme kusuru doğrulandı. `DO_NOT_PROMOTE` kararı, optimize tüm-yol faydası kanıtlanmadığı için korunur; kodun gerçekten 16 B kötüleştiği söylenemez. Kaynak ve kanıt: [P004_SP_ATOMIC_MEASUREMENT_CORRECTION_20261103.md](P004_SP_ATOMIC_MEASUREMENT_CORRECTION_20261103.md). Önceki ham sonuçlar tarihsel kanıt olarak korunur; `PHYSICAL_NOT_RUN`, `RELEASE_BLOCKED` değişmez.

**Tarih:** 2026-11-03 · **Karar:** `QA_REVIEW_FAIL` / `DO_NOT_PROMOTE` / `STACK_ALL_PATHS_REVIEW` / `PHYSICAL_NOT_RUN` / `RELEASE_BLOCKED`.

## Kapsam ve değişmeyen kaynaklar

- Geçici MergedGates + Flash LCD QA `.ino` SHA256: `5c7009477ba56cb70850f22ece9860fcb222bd4c0a0c3a10622c21b66c79bfa5`.
- Yeni QA `.ino` SHA256: `fb02070a91a6420a53382ca73e2406898f0f6a45cb817e1c9969bce0e809a8fb`.
- Yalnız `update_lcd()` fonksiyonu değişti: `char desired[2][LCD_COLUMNS+1]` yerine tek `char desired[LCD_COLUMNS+1]` ve iki satırın sırayla işlenmesi. Önceki `format_menu_line()` tampon optimizasyonu **kullanılmadı**; o aday da `DO_NOT_PROMOTE`.
- Yama üretimi: `qa/p004/stream_lcd_rows_candidate.py`. GitHub-hosted AVR ve simAVR karşılaştırması: `qa/p004/run_row_avr_ab.sh`.
- **Üretim v1.06** `.ino` Git blob `957f963c1d4f59a04f18a896e9f6c92890061a49` değişmedi. v1.05 aktif politika ve v1.04 arşivi korunur.

## Bu aday için yeni native pozitif ve negatif kanıt

Gerçek SHA-kilitli `.ino` kaynaklarındaki `setup()`, `update_lcd()` ve menü biçimlendirme işlevleriyle; GCC/Clang × F_CPU 8/16 MHz, UBSan altında her yapılandırmada 2.048 deterministik/rastgele ekran durumunun yazdırma/caching izi karşılaştırıldı: **8.192/8.192 çıktı paritesi PASS**. Kasıtlı ikinci satır seçimi, cache yazma eksikliği ve yanlış cache satırı mutantları: **12/12 yakalandı**. Motor STEP ve EEPROM yazma sayıları sıfır kaldı. Host prototip donanım mock'udur; gerçek fiziksel LCD/AVR kabulü değildir. Kaynak QA kanıt arşivi SHA256 `bec4580d133de3dfad50ff0dae842ca47fd4a7c2aa2b1d3c3a2f4afe69cf6b50` (ayrı indirilebilir QA dosyası); üretim koduna karıştırılmamıştır.

## Tek seferlik AVR karşılaştırması — FAIL

[GitHub Actions #41662491076](https://github.com/uysalayhan23/PerPompa/actions/runs/41662491076) salt-okunur (owner-gated, `contents:read`, tek iş, 15 dakika, upload/deploy yok) koşuda **4/4 gerçek AVR A/B derleme başarılı**, ancak otomatik stack açıklığı kapısı gerçek regresyon saptadı ve workflow **FAIL** verdi. Ayrı `-fno-lto -fstack-usage` çıktısı optimize firmware'in toplam worst-case'i değildir.

| Ölçüm | Baz 16 MHz | QA yeni 16 MHz | Baz 8 MHz | QA yeni 8 MHz |
|---|---:|---:|---:|---:|
| Derleme | PASS | PASS | PASS | PASS |
| Flash (B) | 28.010 | 27.958 | 28.006 | 27.954 |
| Statik SRAM (B) | 1.326 | 1.326 | 1.326 | 1.326 |
| No-LTO `update_lcd` çerçevesi (B) | 66 | 48 | 66 | 48 |
| **Simüle stack derinliği (B)** | **492** | **508** | **492** | **508** |
| **Gözlenen en dar stack–heap boşluğu (B)** | **205** | **189** | **205** | **189** |
| LCD I²C işlem adedi (sınırlandırılmış iz) | 563 | 291 | 291 | 291 |
| Giriş uyaranı aşamaları | 6/6 | 6/6 | 6/6 | 6/6 |

**Kritik:** 18 B yerel çerçeve azalması, optimize AVR'de **16 B toplam stack kötüleşmesi** ile eşzamanlıdır. 16 MHz testinin sabit simülasyon sınırında I²C işlem sayısı da değişmiştir; bu test gerçek süre/ekran paritesi kanıtı değildir. Negatif kapı bu adayın reddini doğru şekilde tetikledi. 110 B başarıyla kabul edilmiş önceki Flash-LCD QA iyileştirmesi ve fiziksel kilitler değişmeden kalır.

## Teslim

`DO_NOT_PROMOTE`: yeni aday üretim firmware'ine alınmadı. Tek kullanımlık `.github/workflows/p004-row-stack-once.yml` aktif `main`den kaldırılır, test issue'u kapatılır. Tarihsel koşu ve QA kaynakları belgelenir. Aynı başarısız optimizasyonu yeniden çalıştırma; yalnız farklı ve ölçülebilir güvenlik yararı olan kusurları değerlendir. `STACK_ALL_PATHS_REVIEW`, `PHYSICAL_NOT_RUN`, `RELEASE_BLOCKED`.
