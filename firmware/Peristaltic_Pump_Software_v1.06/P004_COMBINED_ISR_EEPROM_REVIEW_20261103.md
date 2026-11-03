# PerPompa P0-04 — EEPROM / UART RX / Timer1 gerçek kesme çakışması QA

> **Sonraki QA ölçümü (2026-11-03):** [Yoğun marker doğrulaması](P004_DENSE_MARKER_ISR_REVIEW_20261103.md), 8/16 MHz pozitif ve Remote LOW koşularının dördünü de yürüttü. Remote LOW 2/2 PASS; pozitif birleşik UART ISR testleri 2/2 REVIEW. Daha sık EEPROM örneklemesine rağmen simüle marker→commit penceresinde UART ISR 0 kaldı. Tarihsel bu belgenin FAIL/NOT_RUN statüleri olduğu gibi korunur.

**Tarih:** 2026-11-03 · **Karar:** `COMBINED_ISR_EEPROM_REVIEW` · `ALL_PATHS_STACK_REVIEW` · `PHYSICAL_NOT_RUN` · `RELEASE_BLOCKED`.

## Kaynak kapsamı ve korumalar

- SHA256 kilitli geçici Flash-LCD birleşik v1.06 QA `.ino`: `5c7009477ba56cb70850f22ece9860fcb222bd4c0a0c3a10622c21b66c79bfa5`. Bu, üretim firmware'i değildir.
- Üretim v1.06 `.ino` Git blob `957f963c1d4f59a04f18a896e9f6c92890061a49` korunmuştur.
- Yeni QA kaynağı `qa/p004/stack_integrated_irq_probe.c`; yürütme betiği `qa/p004/run_integrated_irq_avr.sh`. Tam özgün AVR `setup()/loop()`, EEPROM HAL, Timer1 `__vector_13` ve UART RX `__vector_18` yürütülür; simülatör girişleri sentetiktir. Fiziksel cihaz/buton/elektriksel test **değildir**.
- Başlangıçta UART'ı yalnız enjekte edilmiş bayt sayısıyla değerlendiren eski testte gerçek RX kesmesi **kanıtlanmamıştı**. Yeni QA, kesme vektörüne gerçek CPU girişlerini sayar. SPH/SPL yarım güncellemesi hariç **tamamlanmış AVR SP** örnekleri kullanılır.

## Yeni host pozitif/negatif önkoşul

Geçerli EEPROM slot A + çift tanık ile `Gate::boot == kNewReady`, ikinci slota kayıt; CRC'si bozuk slot A ile `Gate::boot == kLocked` ve yeni slot yazımı reddi: **2/2 host UBSan PASS**. Bu, fiziksel veya AVR birleşik koşu PASS değildir.

## Birleşik AVR sonuçları — HOLD

[Actions #41671014892](https://github.com/uysalayhan23/PerPompa/actions/runs/41671014892) ve [Actions #41671498650](https://github.com/uysalayhan23/PerPompa/actions/runs/41671498650) her biri salt-okunur, 15 dakika sınırında tek işti; iki koşu da birleşik ISR eşzamanlılık kapısından **FAIL** oldu. İkinci koşu yalnız QA UART giriş zamanını EEPROM tanığının ilk yazımına çekti; firmware aynı SHA'da kaldı. Tekrar çalıştırma yapılmayacak.

| 16 MHz, ikinci koşuda gerçek AVR simülasyonu | Kanıt |
|---|---:|
| AVR derleme / statik SRAM | PASS / 1.326 B |
| EEPROM ilk kayıt çift tanık ve commit | Gözlendi |
| UART RX vektörüne gerçek giriş, toplam | 7 |
| Timer1 vektörüne gerçek giriş, toplam | 13.268 |
| Timer1 kesmesi EEPROM yazım penceresinde | **2** |
| **UART RX kesmesi EEPROM yazım penceresinde** | **0 — kabul sağlanmadı** |
| SPH / SPL güncelleme sayısı | 120 / 120 |
| Tamamlanmış SP'ye göre gözlenen stack derinliği | 422 B |
| Tamamlanmış SP'ye göre gözlenen heap boşluğu | 275 B |
| MOTOR_STEP HIGH örnekleri | 0 |
| Çalıştırma kabul sonucu | **QA_ACCEPT=0 / FAIL** |

**Kritik yorum:** Verinin UART alıcısına gönderilmesi veya ISR'nin kayıt **dışında** çalışması, EEPROM yazımı sırasında işlediğini kanıtlamaz. Bu iki simülasyonda aynı yazım penceresinde UART RX ISR çalışması **gösterilemedi**. Bu, gerçek donanımda kesmenin imkânsız olduğunu ya da firmware'de kesin bir hata bulunduğunu kanıtlamaz; test zamanlaması, simülatör davranışı ve kapsanan çalışma yolu ayrıştırılmalıdır.

**8 MHz birleşik yol, Remote LOW negatif senaryo:** Her iki Actions koşusu da ilk 16 MHz pozitif birleşik kapıda durduğundan **NOT_RUN**. Geçmiş AVR 8 MHz derleme PASS'ları bu yeni birleşik yol için PASS sayılmaz.

## Teslim / açık kapılar

- Tek kullanımlık `.github/workflows/p004-integrated-irq-once.yml` testlerden sonra aktif `main` üzerinden kaldırılır.
- İki başarısız çalışma ve somut eksik olay GitHub'da kalır; üretim firmware'i, v1.04 arşivi, v1.05 politika kilitleri ve saatlik P0-04 otomasyonu değişmez.
- Sıradaki kabul için **aynı EEPROM yazma penceresinde gerçek Timer1 + UART RX vektör yürütümü**, kayıt bütünlüğü, Remote LOW engeli, motor STOP ve tamamlanmış SP sınırı için ayrı gerçek AVR kanıtı gerekir. Yalnız sonrasında fiziksel cihazda güç kesintisi/seri veri/ISR testi değerlendirilebilir.
- `NOT_RUN` sonuçlarını `PASS` veya `RELEASE_READY` sayma. `PHYSICAL_NOT_RUN` ve `RELEASE_BLOCKED` değişmez.
