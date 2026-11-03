# PerPompa P0-04 — simAVR EEPROM zamanlama modeli kabul kapısı

**Tarih:** 2026-11-03 · **Karar:** `SIMAVR_EEPROM_TIMING_MODEL_UNSUITABLE` · `COMBINED_ISR_REVIEW` · `PHYSICAL_NOT_RUN` · `RELEASE_BLOCKED`.

## Tespit — fiziksel ATmega328P ile simAVR 1.6 farkı

[Microchip ATmega328P EEPROM kontrol kaydı](https://onlinedocs.microchip.com/oxy/GUID-74F8229E-4C43-4FA0-BE7D-1AA303C6F8A4-en-US-6/GUID-F98CAF39-493F-4498-ABDA-481CD8E49FE4.html): EEPROM silme+yazma işleminin tipik programlama süresi **tek bayt başına 3,4 ms**; işlem sürerken `EEPE` meşgul kalır ve sonraki yazım bu bit temizlenene kadar bekler. Bu, her CPU frekansında 3,4 ms fiziksel süredir: nominal 16 MHz için **54.400**, 8 MHz için **27.200 CPU çevrimi/bayt** (tipik).

[simAVR v1.6 `avr_eeprom.c`](https://github.com/buserror/simavr/blob/v1.6/simavr/sim/avr_eeprom.c) kaynak incelemesi:
- `avr_eeprom_write()` içinde `p->eeprom[ee_addr]` **hemen değiştiriliyor**;
- aynı callback içinde `EEPE` **hemen temizleniyor**;
- `avr_cycle_timer_register_usec(...,3400,avr_eei_raise,...)` yalnız *ready interrupt* için kuruluyor, programlamanın bitmesini modellemek için değil;
- `avr_eei_raise()` `EEPE` bitini temizleyen gecikmiş yazma tamamlanma callback'i değildir.

Ubuntu 24.04 `simavr` paket sürümü **1.6+dfsg-3build2** ([Ubuntu paket kataloğu](https://packages.ubuntu.com/en/noble/simavr)). Yukarıdaki kaynak doğrulaması upstream v1.6 içindir; koşuda kullanılan derlenmiş kütüphane ikilisi kaynak seviyesinde ayrı disassemble edilmedi. Ancak görülen çok kısa EEPROM yazma penceresi bu simülasyon davranışıyla tutarlıdır.

[Önceki yoğun-marker QA #41673802315](https://github.com/uysalayhan23/PerPompa/actions/runs/41673802315) fiziksel model gibi yorumlanamaz:

| Önceki ölçüm | 16 MHz | 8 MHz |
|---|---:|---:|
| Simüle commit-marker `0` → final commit | 3.335 çevrim | 3.235 çevrim |
| Bu çevrimlerin CPU süresi | **0,208 ms** | **0,404 ms** |
| ATmega328P tipik **tek bayt** yazma süresi | **3,4 ms** | **3,4 ms** |
| UART ISR kayıt penceresinde | 0 | 0 |
| Timer1 ISR kayıt penceresinde | 1 | 1 |
| Remote LOW negatif | PASS | PASS |
| Pozitif birleşik UART/Timer/EEPROM kabulü | **REVIEW** | **REVIEW** |

**Sonuç:** İşaretçi çevriminin gerçek donanımda ne kadar süreceği bu simülatör modelinden hesaplanamaz. Testin UART ISR'yi aynı pencerede görmemesi ne güvenliğini, ne imkânsızlığını, ne de fiziksel en-kötü-durum stack payını kanıtlar. Geçmiş 8/16 MHz AVR derlemeleri PASS olarak kalır; endüstriyel kabul değildir.

## Yeni fail-closed QA aracı

`qa/p004/check_simavr_eeprom_timing.py` **yalnız kaynak okur**. `--self-test` ile sentetik anlık-yazma pozitif ve dört bozulmuş/eksik-tanımlı kaynak durumunda **5/5 QA kontrolü PASS**. Upstream v1.6 `avr_eeprom.c` üzerinde anlık yazım, erken `EEPE` temizleme ve `3.400 µs` ready IRQ zamanlayıcısı gerçek kaynakta doğrulandı; üç ayrı kaynak değişikliği negatif kontrol tarafından ayırt edildi. Bu testler EEPROM/UART/Timer birleşik firmware testi **değildir**.

Kullanım: `python3 qa/p004/check_simavr_eeprom_timing.py --self-test`; yerel kütüphane kaynağı bulunduğunda `python3 qa/p004/check_simavr_eeprom_timing.py --source /path/to/simavr/sim/avr_eeprom.c`. Son komut **kasıtlı olarak sıfır olmayan çıkış koduyla** biter: bilinen anlık EEPROM modeli için 2, bilinmeyen/henüz kalibre edilmemiş model için 3. Kaynak kontrolünden tek başına `TIMING_PASS` üretilemez.

Önceki birleşik QA betikleri, fiziksel EEPROM zamanlaması kanıtı olmadan serbest bırakma kapısı olarak kullanılmamalıdır. Yeni ve tekrarsız kabul için EEPROM meşgul süresi, gerçek/kalibre edilmiş model, UART RX ve Timer1 kesme girişleri, kayıt bütünlüğü ve tamamlanmış SP tepe verisi birbirinden bağımsız doğrulanmalıdır.

## Korunan kilitler

**Yeni Actions koşusu çalıştırılmadı.** Üretim v1.06 `.ino` blob'u `957f963c1d4f59a04f18a896e9f6c92890061a49` değişmedi. Geçici `qa/p004` aracı ve rapor dışında AVR kaynak düzenlemesi yok. v1.05 politika ve arşivleri, P0-04 saatlik otomasyonu, `PHYSICAL_NOT_RUN`, `RELEASE_BLOCKED` korunur.
