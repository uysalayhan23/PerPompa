# PerPompa P0-04 — LCD menü stack A/B incelemesi (2026-11-03)


> **ÖLÇÜM DÜZELTMESİ — 2026-11-03 (sonraki doğrulama):** 205→196 B ham SP farkı, tamamlanmış SP'ye göre gerçek 9 B stack regresyonu olarak kanıtlanmış değildir. SPH/SPL ara örnekleme kusuru doğrulandı. `DO_NOT_PROMOTE` kararı, optimize tüm-yol faydası kanıtlanmadığı için korunur; kodun gerçekten 9 B kötüleştiği söylenemez. Kaynak ve kanıt: [P004_SP_ATOMIC_MEASUREMENT_CORRECTION_20261103.md](P004_SP_ATOMIC_MEASUREMENT_CORRECTION_20261103.md). Önceki ham sonuçlar tarihsel kanıt olarak korunur; `PHYSICAL_NOT_RUN`, `RELEASE_BLOCKED` değişmez.

**Sonuç: `QA_REGRESSION_FOUND` · `DO_NOT_PROMOTE` · `STACK_ALL_PATHS_REVIEW` · `PHYSICAL_NOT_RUN` · `RELEASE_BLOCKED`.**

## Kaynak ve minimal aday

Yalnız QA MergedGates + Flash-LCD v1.06 kaynağı üzerinde `format_menu_line()` içindeki 48 B yerel ara tampon kaldırıldı; ekrandaki 16 karakterlik satır doğrudan işleniyor. EEPROM, UART, motor, ISR ve üretim firmware'i değiştirilmedi.

- Baz tam .ino SHA256: `5c7009477ba56cb70850f22ece9860fcb222bd4c0a0c3a10622c21b66c79bfa5`.
- QA aday tam .ino SHA256: `87fb708ca17e33cea2d13606344605d85768b200c5d8d182e4c30c05a4d3bd71`.
- SHA kilitli aday üretimi: `qa/p004/compact_menu_format_candidate.py`; AVR/simAVR kanıt testi: `qa/p004/run_compact_menu_avr_ab.sh`.
- Gerçek 8/16 MHz AVR ve simAVR incelemesi: [GitHub Actions 41659926842](https://github.com/uysalayhan23/PerPompa/actions/runs/41659926842). Workflow sahibiyle sınırlı, `contents:read`, 1 iş, 15 dakika, upload/deploy yok.
- Host kaynak paritesi: `qa/p004/menu_format_equivalence.cpp.gz`, ham C++ SHA256 `b2141adb0582b2e7d428c46d5ff270e18ca7b7adb9e8cae171247471d40c16c3`; arşiv SHA256 `fb207bcc82d4ae97d45ade101f21135bb108695f0b437436562c8e524b9e5fff`. `gzip -dc qa/p004/menu_format_equivalence.cpp.gz > /tmp/menu_format_equivalence.cpp` ile kaynak açılabilir.

## Yeni pozitif-negatif kanıt

GCC ve Clang × `F_CPU=8000000/16000000`, UBSan ile 30.000'er kontrollü/rastgele VALUE/OPTION/ACTION satırı eski ve yeni biçimlendiricide **harf harf eşleşti**: toplam **120.000/120.000 PASS**. Üç kasıtlı bozulma (değer/option boşluğu, satır dolgu karakteri, değer suffix'i) dört yapılandırmada **12/12 yakalandı**. Bu test gerçek .ino'dan kesilmiş fonksiyonları mock menüyle kullanır, fiziksel cihaz veya tam `setup()/loop()` kabulü değildir. ASan denemesi bu yerel ortamda runtime/virtual-address kısıtı nedeniyle çalışmadı; **ASAN_NOT_RUN**.

## Gerçek optimize AVR ile başarısız A/B

| Ölçüm | Baz 16 MHz | Aday 16 MHz | Baz 8 MHz | Aday 8 MHz |
|---|---:|---:|---:|---:|
| AVR derleme | PASS | PASS | PASS | PASS |
| Flash (B) | 28.010 | 27.932 | 28.006 | 27.928 |
| Statik SRAM (B) | 1.326 | 1.326 | 1.326 | 1.326 |
| Ayrı no-LTO tanı `format_menu_line` çerçevesi (B) | 76 | 24 | 76 | 24 |
| **Optimize simAVR gözlenen stack derinliği (B)** | **492** | **501** | **492** | **501** |
| **SimAVR gözlenen en dar stack–heap boşluğu (B)** | **205** | **196** | **205** | **196** |
| I²C LCD işlem adedi | 563 | 563 | 291 | 291 |
| Giriş uyaranı aşamaları | 6/6 | 6/6 | 6/6 | 6/6 |

Derleyiciyle ölçülen **52 B yerel çerçeve küçülmesi**, optimize AVR çalışmasında toplam stack kazancı sağlamadı; aksine testte stack **9 B büyüdü**. Kaydedilen GitHub Actions genel sonucu bu regresyonu yakaladığı için **FAIL** oldu, `PASS` değildir. 78 B Flash kazancı, stack güvenliği kötüleşmesini telafi etmez.

## Teslim ve güvenlik

**`DO_NOT_PROMOTE`**: bu QA format optimizasyonu üretim v1.06'ya alınmaz ve güvenlik kabulüne yükseltilmez. Geçici workflow aktif `.github/workflows/` dizininden kaldırıldı; tarihsel commit ve koşu sonuçları kanıt olarak kalır. Üretim v1.06 .ino Git blob `957f963c1d4f59a04f18a896e9f6c92890061a49` korunur. v1.04 arşivi, v1.05 kabul politikası ve P0-04 saatlik otomasyonu değiştirilmez.

`STACK_ALL_PATHS_REVIEW` · `PHYSICAL_NOT_RUN` · `RELEASE_BLOCKED`.
