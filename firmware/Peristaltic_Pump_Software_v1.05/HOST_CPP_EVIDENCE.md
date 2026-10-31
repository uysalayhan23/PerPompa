# PerPompa v1.05 — P0-01 İlk HOST_CPP Kanıtı
**Tarih:** 2026-10-31 • **Kaynak main HEAD:** `75f414385183e60638a4efdfc14fa44a6e848496` • **Firmware blob:** `0d965c08947ba0b3170d22fbff806fc2e6f1e83a`
**Tür:** İki gerçek kaynak C++ fonksiyon gövdesinin izole masaüstü çalışması; tüm `.ino`, hedef kart veya fiziksel pompa testi değildir.

## Kaynak ve yöntem
GitHub'daki değişmemiş `steps_calc` ve `delay_us_calc` fonksiyon gövdeleri ve gerekli `#define` sabitleri alındı. Geçici C++ sürücüsüyle `g++ 14.2.0 -std=c++17 -O0 -g -fsanitize=undefined,float-cast-overflow -fno-sanitize-recover=all` altında koşuldu. Aynı yaklaşımı otomatikleştiren [tests/native-math-harness.cjs](tests/native-math-harness.cjs) geliştirildi ve yerel geçici çalışma kopyasında Node sözdizimi/koşu doğrulaması yapıldı. Testin tamamının doğrudan uzak GitHub dosya sistemi üzerinde çalıştırıldığı iddia edilmez; otomatik araç ileride depoyu yerel klonlayıp aynı kodu ayıklayarak tekrar çalıştırmalıdır.

## Gözlenen sonuçlar
| ID | Test | Gözlenen sonuç | Statü |
|---|---|---|---|
| H01 | 1.0 mL; 30 dönüşte 5.000 mL kalibrasyon | 4800 step | HOST_CPP PASS |
| H02 | 10.0 mL/dakika; 5.000 mL kalibrasyon | 625 µs yarım periyot | HOST_CPP PASS |
| H03 | rotasyon birimi 3 dönüş | 2400 step | HOST_CPP PASS |
| H04 | 10 rpm | 3750 µs yarım periyot | HOST_CPP PASS |
| H05 | Cal=0, pozitif hacim | `runtime error: inf is outside the range of representable values of type 'long int'` | **KNOWN DEFECT REPRODUCED** |
| H06 | Hız=0, pozitif Cal | Aynı sanitizer `inf`→`long` hatası | **KNOWN DEFECT REPRODUCED** |
| H07 | Çok küçük Cal + maksimum hacim | `2.39976e+10` hesaplanan step > `2147483647` signed 32-bit sınırı | MODEL_BOUNDARY REVIEW |

Bu pozitif sonuçlar yalnızca hesap fonksiyonu girdileri için geçerlidir; pompada doğruluk, stop, encoder, Remote, RNG veya tüm firmware üzerinde PASS değildir. Geliştirme kodu değiştirilmedi. P0-01'in çözümüne geçmeden geçerli ve sınır girişler için daha geniş host/AVR testleri, 8/16 MHz timing ve bağımsız QA gerekiyor.

## Mevcut blokaj
`AVR_COMPILE=NOT_RUN; HW_BENCH=NOT_RUN; HW_PUMP=NOT_RUN; USER_ACCEPTANCE=NOT_RUN; RELEASE_BLOCKED`.

## Sonraki adım
Sıfır ve aralık dışı girişin tüm yollarda reddedilmesi için aday minimal yama ve test kabul kriterini hazırlamak; üretim kaynak koduna gönderim ancak manifestodaki gerçek C++/AVR derleme ve regresyon kapısı geçilirse.
