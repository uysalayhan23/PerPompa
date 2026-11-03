# PerPompa P0-04 — Nominal EEPROM meşgul zamanlamalı AVR ISR QA kabulü (2026-11-03)

**Karar:** `NOMINAL_SIMAVR_QA_4/4_PASS` · `PATCH_SOURCE_7/7_PASS` · `NEGATIVE_PARSER_12/12_REJECT` · `ISR_ALL_PATHS_REVIEW` · `CALIBRATION_NOT_RUN` · `PHYSICAL_NOT_RUN` · `RELEASE_BLOCKED`.

## Test-first gerçek kusur ve QA sürücüsü

SimAVR v1.6 orijinal EEPROM kodunda veri ve `EEPE` meşgul durumu yazı isteğinde hemen değişiyordu. Bu nedenle önceki marker→commit yolu 8/16 MHz'de yalnız 3.235/3.335 çevrim sürüyor, EEPROM meşgul penceresinde UART RX ISR çakışması gözlenemiyordu. Bu tarihsel sonuçlar `REVIEW` olarak saklanmıştır.

Yalnız QA denemesi için `qa/p004/patch_simavr_eeprom.py` orijinal [simAVR v1.6, commit `1b6e6ed10a92438da47fe99864698db9ac2169b6`](https://github.com/buserror/simavr/tree/1b6e6ed10a92438da47fe99864698db9ac2169b6) sürücüsünü **geçici runner kopyasında** dönüştürür. Mevcut sistem kütüphanesine veya PerPompa üretim firmware'ine dokunulmaz. Yama yalnız **EEPM=00 / nominal birleşik silme-yazma** modunu ele alır: yazılacak byte 3400 µs sonra commit edilir; `EEPE` aynı süre boyunca BUSY, ikinci yazma/okuma girişimleri bloke, ready kesmesi commit'ten sonra. Diğer EEPM modları **fail-closed / desteklenmiyor**.

**Test-first:** Orijinal v1.6 işlevinin erken EEPROM verisi görünürlüğü C mock testi **beklenen RED**; yama uygulanmış aynı derlenebilir C fixture'ında gecikmiş commit, `EEPE` BUSY, ikinci yazmanın reddi, ready IRQ ve desteklenmeyen modun reddi **GREEN**. Ayrı `qa/p004/test_patch_simavr_eeprom.py` testi, GitHub runner'ındaki **gerçek sabitlenmiş upstream kaynakla 7/7 PASS**, kasıtlı bozulmuş callback, kaynak yazma, eksik/tekrar header ve tekrar patch durumlarını doğru reddetti. Gerçek kaynak doğrudan runner'da yamalandı ve `libsimavr.a` derlendi. Önceki 9/9 bağımsız saf Python referans modeli testleri tekrar çalıştırılmadı.

## Birleştirilmiş gerçek AVR makine kodu yürütümü — yalnız sınırlı QA simülasyonu

[Başarılı GitHub Actions #41688915037](https://github.com/uysalayhan23/PerPompa/actions/runs/41688915037) sahibine özel, salt-okunur (tek iş, 15 dakika, `contents:read`) koşuda **yamalı simavr v1.6 statik kütüphanesi** bağlandı; sistemin orijinal simavr kütüphanesinin kullanılmasını engelleyen dinamik bağ kontrolü yapıldı.

`qa/p004/run_nominal_eepe_avr_qa.sh` kendi ortamında SHA kilitli MergedGates + Flash LCD QA `.ino` `5c7009477ba56cb70850f22ece9860fcb222bd4c0a0c3a10622c21b66c79bfa5` kaynağını gerçek Pro Mini **16 MHz / 8 MHz** AVR ELF olarak derledi ve mevcut `stack_integrated_marker_probe.c` ile **gerçek kodun** UART RX `__vector_18` ve Timer1 `__vector_13` PC girişlerini saydı. Fiziksel kart ve EEPROM kullanılmadı.

| Yeni nominal QA sonucu | 16 MHz pozitif | 8 MHz pozitif | 16 MHz Remote LOW | 8 MHz Remote LOW |
|---|---:|---:|---:|---:|
| AVR derleme | PASS | PASS | Aynı ELF | Aynı ELF |
| Statik SRAM | 1.326 B | 1.326 B | 1.326 B | 1.326 B |
| EEPROM commit-marker → final commit | 1.634.947 çevrim | 819.907 çevrim | yok | yok |
| Gerçek AVR UART RX ISR, kayıt içinde | **7** | **7** | uygulanmaz | uygulanmaz |
| Gerçek AVR Timer1 ISR, kayıt içinde | **204** | **205** | uygulanmaz | uygulanmaz |
| EEPROM commit | 1 | 1 | 0 | 0 |
| Motor STEP HIGH gözlemi | 0 | 0 | 0 | 0 |
| Tamamlanmış SP'ye göre gözlenen en dar heap açıklığı | **277 B** | **275 B** | **383 B** | **377 B** |
| Bounded QA kabulü | **PASS** | **PASS** | **PASS** | **PASS** |

Kaynak regresyonu 7/7 PASS, sınırlı 4/4 AVR senaryo PASS ve **12/12** parser negatif testi doğru yakalama ile tamamlandı. Firmware `8/16 MHz` Flash sırasıyla **28.010/28.006 B**. Pozitif test süresi yaklaşık **102 ms**, nominal 3,4 ms/bayt modeliyle tutarlı. UART ISR sadece gönderilen veri sayısıyla değil, yürütülen AVR vektör PC'siyle teyit edildi. `SPH/SPL` iki bayt güncellemesi tamamlanmadan alınan örnekler stack tepe sayılmadı.

**İlk Actions #41688431206**: gerçek upstream 7/7 test ve AVR EEPROM C modülü derleme PASS; ancak eski `run_avr.c` `strncpy` uyarısını `-Werror` nedeniyle reddeden alakasız build hatası, iş genel FAIL. Ardından sadece `libsimavr` hedefi derlenerek **tek** dar kapsamlı düzeltme koşusu #41688915037 PASS. Bu iki run kanıtı ayrıştırıldı; başarısız koşu saklandı.

## Kritik kabul sınırları

Bu **gerçek AVR ELF'in nominal simülatör içindeki yürütümüdür**, gerçek donanımda doğrulanmış EEPROM programlama süresi/cihaz toleransı **değildir**. Model kaynakları sabit tipik **3.400 µs** değerini kullanır; cihazın gerçek sıcaklık/gerilim/EEPROM write-mode tolerans aralığı veya donanım kalibrasyonu yok. Örneğin tam çalışma koşullarında ISR gecikmeleri, seri trafik yoğunluğu, watchdog ve reset, EEPROM elektriksel güç kesintisi, bütün ISR iç içe geçmeleri, gerçek LCD, fiziksel motor STEP davranışı ve worst-case yığın/heap sınırı kanıtlanmadı. Burada elde edilen en dar açıklıklar sadece koşulan yazılımsal yol içindir.

- `NOMINAL_QA_ISR_OVERLAP_4/4_PASS` yalnız bu SHA ve bu senaryolar için.
- `ALL_PATHS_STACK_AND_ISR=REVIEW`; `PHYSICAL_NOT_RUN`, `CALIBRATION_NOT_RUN`, `RELEASE_BLOCKED` **değişmez**.
- Üretim v1.06 `.ino` Git blob `957f963c1d4f59a04f18a896e9f6c92890061a49` **değiştirilmedi**. v1.04/v1.05 kilitlerine ve P0-04 otomasyonu ayarına dokunulmadı (son okumada **disabled**).
- Birleşik QA kalıcı kodu `qa/p004` dizininde tutulur; tek kullanımlık `.github/workflows/p004-nominal-eepe-once.yml` **main'den kaldırılır**. Tarihsel Actions run/artifact kanıtı kalır.
