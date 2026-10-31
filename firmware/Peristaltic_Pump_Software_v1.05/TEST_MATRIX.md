# PerPompa v1.05 — Regresyon Test Matrisi

**Statü sözlüğü:** `SOURCE_CHECK` kaynak incelemesi; `MODEL_CHECK` kaynak davranışına dayalı model; `HOST_CPP` gerçek fonksiyon derleme/çalıştırma; `AVR_COMPILE` hedef Arduino derlemesi; `HW_BENCH` cihaz giriş/çıkış; `HW_PUMP` gerçek sıvı/motor doğrulaması. `NOT_RUN`, `REVIEW`, `FAIL` ve `PASS` birbirine dönüştürülmez.

**İlk temel:** v1.04 ve v1.05 `.ino` Git blob `0d965c08947ba0b3170d22fbff806fc2e6f1e83a` (31.10.2026). Önceki oturumda 32/32 kaynak desen denetimi ve model kontrolleri raporlandı; **AVR veya gerçek kart PASS değildir**. Ayrıntılı native test kanıtı henüz eklenmedi.

| ID | Pozitif durum | Negatif/sınır durum | Gereken kanıt | Başlangıç durumu |
|---|---|---|---|---|
| M01 Dose | Ayarlı hacim adım sayısı ve bitiş | sıfır cal, aşırı steps, negatif veya sıfır hacim | Gerçek hesap fonksiyonu + step çıkışı | REVIEW |
| M02 Pump | Geçerli hızda kararlı periyot | sıfır hız, çok düşük rpm/mL/min, overflow | AVR 8/16 MHz timing, stop | REVIEW |
| M03 Cal | 30 tur 24.000 mikro-adım hedefi | bitiş/reset, sapmış süre | Native + fiziksel ölçüm | REVIEW |
| M04 USB | p/d/c/w/x geçerli komut | boş, eksi, fazla, yarım, yabancı char, stop | Seri emülasyon + RNG | REVIEW |
| M05 Remote Hold | D5 LOW start / HIGH stop | bounce, ilk açılış LOW, menüde LOW | Tam döngü modeli + cihaz | REVIEW |
| M06 Remote Pulse | yeni LOW toggle | tekrarlı LOW, bounce, açılışta LOW | Tam döngü modeli + cihaz | REVIEW |
| M07 RNG | normal step aktif; stop/bitince idle | USB yanlış durum, hata sonrası idle | A0 dijital sinyal, PLC rölesi | REVIEW |
| M08 EEPROM | geçerli ayarlar kaydı/geri yükleme | boş ve kısmi kayıt, eski v1.03 bellek | Simülasyon + güç çevrimi | REVIEW |
| M09 DIR | CW/CCW, menü değişimi | kayıtlı CCW açılışı | D6 sinyal gözlemi | REVIEW |
| M10 Encoder | 1 tık/1 adım, buton | hızlı/ters/basılı adım, stop kuyruğu | Mock/native ve fiziksel encoder | REVIEW |
| M11 LCD | 16×2 değişen karakter önbelleği | taşma, yeniden çizim, uzun sayı | Mock/native ve fiziksel LCD | REVIEW |
| M12 Release | v1.04 arşivi değişmez, v1.05 bağımsız | yanlış klasör, arşiv yazma, HEAD yarışması | Blob SHA ve expected_sha | SOURCE_CHECK |

İlk test altyapısı [tests/reference-gate.cjs](tests/reference-gate.cjs) yalnız statik ve sayısal model kapısıdır. `node` çıktısındaki PASS, `AVR_COMPILE` veya `HW_PUMP` anlamına gelmez. Firmware düzeltme kabulü için ek gerçek kaynak/derleme testleri şarttır.

## Endüstriyel kabul uzantısı (2026-10-31)

G01–G10 ürün release kapıları [INDUSTRIAL_RELEASE_GATE.md](INDUSTRIAL_RELEASE_GATE.md) içinde takip edilir; bu test matrisi bunların yerine geçmez. [RISK_REGISTER.md](RISK_REGISTER.md) R-001–R-012 her test alanının neden gerekli olduğunu gösterir. HOST_CPP test hazırlığı [tests/native-math-harness.cjs](tests/native-math-harness.cjs) dosyasındadır: **henüz burada çalıştırılmış test yok, AVR_COMPILE ve HW_* tümü NOT_RUN.** Bir test aracının varlığı PASS değildir.
