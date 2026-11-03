# PerPompa v1.06 — Kurtarma anında kesinti regresyonu (2026-11-03)

**Task:** İlk kaydın güvenli kurtarılması için ayrı EEPROM kesinti kabulü. Bu yalnız **kaynak-başlığı native C++ simülasyonudur**; gerçek Arduino/EEPROM testi veya tüm `.ino setup()/loop()` entegrasyon testi değildir.

## Sabit kaynaklar ve test doğrulaması

- Başlangıç canlı main HEAD: `baacc74e5384b06b829bbb605fc49d04466620b8`.
- Korunan aktif v1.06 `.ino`: Git blob `55b19a8b1017bf1924dc568439f920ec9c90064f`.
- Korumacı REVIEW-only başlıkları: `p004-migration-guard-candidate.h` Git blob `9962766dfa150888d77dbf23857216f52d0a48b5`; `p004-recovery-ui.h` Git blob `036978c0f7bf5631180dcf96dac329a4dbd9d3c9`.
- Önceden raporlanan güvensiz 48/48 kurtarma adayı **yeniden kabul edilmedi / geçersiz**.

Yeni `recovery_cut_integration_native.cpp` SHA256: `555cad67c9b387abc2df22d794396f3b7512327871b8fdd41901da8cbcfa910a`. GCC/Clang C++11, `-Wall -Wextra -Werror -pedantic -fsanitize=undefined -fno-sanitize-recover=all`.

| Kontrol | g++ | clang++ |
|---|---:|---:|
| İlk kayıt 50 kesinti | PASS | PASS |
| 48 LOCKED içinden kurtarılabilen | 17 | 17 |
| Kurtarılmaması gereken servis kilidi | 31 | 31 |
| Kurtarma sırasında yeni kesinti konumu | 161 | 161 |
| Kesinti sonrası yanlış `NEW_READY` | 0 | 0 |
| Korumalı EEPROM alanları bozuldu | 0 | 0 |
| Test doğrulama sayısı | **315371 PASS** | **315371 PASS** |

Kurtarma sırasında 161 noktadan 144'ü `LOCKED`, 17'si `LEGACY_REVIEW` kaldı; kullanıcı onayı ve yeniden kalibrasyon olmadan motor izni doğmadı. Hasarlı eski commit işaretine sahip kayıtlar korunarak reddedildi.

Tekrarlanabilir QA ZIP SHA256: `D8E859B85AA9AA3B07BDE47710DE5921258524A96CD4E4C65616009798806F09`; 10 dosya / ZIP içi SHA256 manifesti kontrol edildi. Paket ayrıca RAM-only AVR derleme betiği içerir.

## Açık kapılar

- Tutucu kurtarma **aktif firmware'e eklenmedi**; menü `setup/loop` tam kaynak/native testleri `NOT_RUN`.
- Operatör onayı, USB/Remote/HW Setup idle koşulları için yalnız izole review başlığı testi PASS; gerçek LCD/encoder kart kabulü `NOT_RUN`.
- Kullanılmayan `String inputString` ve `reserve(200)` satırlarını kaldıran RAM-only aday `957f963c1d4f59a04f18a896e9f6c92890061a49`: **AVR 8/16MHz = NOT_RUN**.
- AVR compiler bu native konteynerde mevcut değil; geçmiş v1.06 AVR 2/2 PASS, farklı blob için yeni AVR PASS değildir.
- Dinamik heap/stack üst kullanım, gerçek brownout, gerçek motor/stop gecikmesi = `NOT_RUN`. **RELEASE_BLOCKED**.
- v1.05, v1.04 arşivi, v1.06 firmware ve P0-04 saatlik otomasyonu değiştirilmedi. Modbus/RS485 eklenmedi. Actions 0/50 bu tur.
