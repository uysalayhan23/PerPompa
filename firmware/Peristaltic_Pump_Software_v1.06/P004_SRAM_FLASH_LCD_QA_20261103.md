# PerPompa P0-04 — LCD sabit metinleriyle SRAM azaltma A/B kabulü (2026-11-03)

**Disposition: QA CANDIDATE PASS; PRODUCTION INTEGRATION HOLD.** Bu çalışma yalnız SHA-kilitli geçici MergedGates/EEPROM recovery adayı içindir, ana firmware için değildir.

## Somut kaynak kusuru / minimal iyileştirme

Önceki tam v1.06 kurtarma adayı, 16×2 LCD için kullanılan sabit uyarı yazılarını AVR'nin sınırlı SRAM'inde tutuyordu. Yalnız 5 ayrı `lcd.print(...)` ifadesindeki toplam 7 sabit mesaj `F("...")` ile AVR Flash belleğine taşındı. Metinlerin harfleri, arayüz koşulları, EEPROM ve motor/Remote güvenlik yolları değiştirilmedi.

**Kaynaklar:**
- Baz geçici aday arşivi: `qa/p004/read_only_candidate.tar.gz`, SHA256 `b9d04926776ce3703043c507fa804c6cc6ff6ca494f84df6b6926b37ff3d38f2`.
- Baz geçici `.ino`: SHA256 `e0d852757f74fedc4c00dd9da05b9cf02bf660c72bf7dcaa4afc52c8a7710e21`.
- Yalnız 5 yazdırma satırı değişen QA adayı: SHA256 `5c7009477ba56cb70850f22ece9860fcb222bd4c0a0c3a10622c21b66c79bfa5`.
- Tekrar üretim: `qa/p004/flash_lcd_literals_candidate.py` (beklenen SHA'yı denetler), `qa/p004/run_flash_sram_ab.sh` (salt test, fiziksel karta yükleme içermez).
- Tek kullanımlık GitHub-hosted [Actions 41645418905](https://github.com/uysalayhan23/PerPompa/actions/runs/41645418905). Runner: Ubuntu 24.04; Arduino AVR Boards 1.8.8; iki gerçek Pro Mini FQBN; `simavr` sınırlandırılmış LCD/enkoder/Remote izlemesi. `contents: read`, sahibiyle sınırlı tetik, 15 dakika tek iş, firmware upload veya deploy yok.

## Gerçek AVR 8/16 MHz A/B sonuçları

| Ölçüm | 16 MHz baz | 16 MHz Flash | 8 MHz baz | 8 MHz Flash |
|---|---:|---:|---:|---:|
| Derleme | PASS | **PASS** | PASS | **PASS** |
| Program Flash, bayt | 27.932 | 28.010 | 27.928 | 28.006 |
| Statik SRAM, bayt | 1.436 | **1.326** | 1.436 | **1.326** |
| Kalan statik SRAM, bayt | 612 | **722** | 612 | **722** |
| Ölçülen stack–heap en dar açıklık, bayt | 95 | **205** | 95 | **205** |
| Ölçülen stack derinliği, bayt | 492 | 492 | 492 | 492 |
| LCD I2C işlem sayısı | 563 | 563 | 291 | 291 |

Her iki frekansta da **110 B gerçek statik SRAM tasarrufu**, **110 B gözlenen stack–heap açıklığı artışı**, 78 B ek program Flash kullanımı ölçüldü. Pozitif/negatif A/B kapısı, baz ve yama kaynak SHA doğrulaması; bazın 95 B açıklığından daha düşük veya eşit sonuçların reddi; adayın en az 32 B SRAM ve 16 B simülasyon açıklığı kazanması şartıyla PASS. LCD init iki taraf için görüldü, I2C yazma sayısı eşleşti, simülatör giriş aşamaları 6/6 tamamlandı. Bu, ekranda fiziksel piksel eşdeğerliği veya tam davranış/parite kanıtı değildir.

## Kabul ve kilitler

- `AVR_COMPILE_16MHz=PASS`, `AVR_COMPILE_8MHz=PASS` **yalnız QA adayı için**.
- `BOUNDED_SIMAVR_AB=PASS` **yalnız test edilen uyarı/LCD/Remote izleri için**.
- `FULL_NATIVE_REGRESSION=NOT_RUN`, `ALL_PATHS_STACK_PEAK=REVIEW`, `ISR_STACK_PEAK=NOT_RUN`, `PHYSICAL_NOT_RUN`, `RELEASE_BLOCKED`.
- 205 B gözlenen açıklık, tüm gerçek çalışma koşulları için garantili en düşük açıklık değildir; gerçek LCD ve pompa üzerinde doğrulama yoktur.
- Üretimdeki `firmware/Peristaltic_Pump_Software_v1.06/Peristaltic_Pump_Software_v1.06.ino` Git blob'u `957f963c1d4f59a04f18a896e9f6c92890061a49` **değiştirilmedi**. Firmware'e yükseltme, arşiv taşıma, v1.05 kilitleri veya kullanıcı onayı varsayılmadı.
- Çalışma kanıtları GitHub Actions üzerinde mevcuttur; çalışma dosyaları `qa/p004` altında kalıcı; tek kullanımlık workflow aktif GitHub `.github/workflows/` klasöründen kaldırılacaktır.

**Sonuç:** En küçük LCD metin iyileştirmesi ölçülebilir SRAM faydası sağladı. Test-only yama saklandı; firmware main'e alınmadı.
