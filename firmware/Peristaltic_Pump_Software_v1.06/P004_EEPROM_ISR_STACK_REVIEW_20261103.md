# PerPompa P0-04 — EEPROM/UART/Timer birleşik stack QA (2026-11-03)


> **ÖLÇÜM DÜZELTMESİ — 2026-11-03 (sonraki doğrulama):** 200→201 B ham stack boşluğu, güncellenmesi tamamlanmış SP'ye dayalı karşılaştırma değildir. 16 bit SPH/SPL geçici örnekleme kusuru saptandığından gerçek 1 B kazanç olarak kullanılmaz. `DO_NOT_PROMOTE` kararı **kanıtlı yarar ve tam güvenlik kabulü bulunmadığı** için değişmez. Kaynak ve kanıt: [P004_SP_ATOMIC_MEASUREMENT_CORRECTION_20261103.md](P004_SP_ATOMIC_MEASUREMENT_CORRECTION_20261103.md). Önceki ham sonuçlar tarihsel kanıt olarak korunur; `PHYSICAL_NOT_RUN`, `RELEASE_BLOCKED` değişmez.

**Karar: EEPROM streaming optimizasyonu QA REVIEW / DO_NOT_PROMOTE.** Üretim .ino ve mevcut v1.06 firmware'i değiştirilmedi. Bu çalışma tam stack üst sınırını veya fiziksel güvenliği kanıtlamaz.

## Girdi ve tek QA değişikliği
- Başlangıç: geçici MergedGates / Flash LCD adayı `.ino` SHA256 `5c7009477ba56cb70850f22ece9860fcb222bd4c0a0c3a10622c21b66c79bfa5`. Önceki 110 bayt LCD SRAM kazancı korunmuştur, tekrar test edilmemiştir.
- Orijinal EEPROM header SHA256: `355b6ba47f322dc24ea257cdd5f2a35dadbeb4ab55187e04e7a9ab0f8ea59088`.
- Tek geçişli aday header SHA256: `83add43e3016e0cd1f9fd1ed6ec29850d9616de9c196a08d692a8893d5110dc8`; `qa/p004/p0-04-store-stream-qa.h` altında review-only.
- Değişiklik: `Store::decode` sırasında 30 baytlık otomatik EEPROM veri kopyasının yerini tek geçişli CRC ve alan çözümlemesi alır. EEPROM kayıt adresleri, format, CRC, commit son baytı, motor ve UART denetim kodu aynıdır. *Bu aday üretime yükseltilmemiştir.*

## Kaynakla çalışan yeni native pozitif ve negatif kanıtlar
- Gerçek `p0-04-settings-store.h` orijinal/yeni içerikleri eşzamanlı derleyen host: GCC ve Clang × F_CPU 8/16 MHz; **48/48 karşılaştırma PASS**, CRC/commit kasıtlı mutantları **32/32 beklenen FAIL**. Ayrı ASan+UBSan derlemesi **48/48 PASS**, **32/32 mutant yakalama**.
- Gerçek Flash LCD v1.06 `setup()/loop()` kodlu mock: EEPROM yazımı sırasında Timer ISR çağrısı, sonradan gelen UART, ilk onay öncesi STOP, yarım EEPROM kaydı sonrası fail-closed yeniden açılış, USB calibration write sırasında STEP güvenliği. GCC/Clang × F_CPU8/16: **16/16 PASS**; iki mutant **8/8 yakalandı**. Ayrı ASan+UBSan: **16/16 PASS**, **8/8 mutant yakalama**.
- Bu host olay modelleri gerçek AVR interrupt stack derinliği ya da donanım kabulü değildir. Test kaynakları ve JSON kanıtları bağımsız QA arşivinde korunmuştur.

## Aynı kaynak için gerçek AVR ve simAVR A/B

[GitHub Actions 41653905873](https://github.com/uysalayhan23/PerPompa/actions/runs/41653905873), `ubuntu-24.04`, Arduino AVR Boards 1.8.8; SHA-kilitli QA arşivinden derleme; yalnız `contents: read`, owner-only event, 15 dakika, tek eşzamanlı iş ve upload/deploy yok.

| Ölçüm | 16 MHz baz | 16 MHz aday | 8 MHz baz | 8 MHz aday |
|---|---:|---:|---:|---:|
| Gerçek AVR derleme | PASS | PASS | PASS | PASS |
| Flash bayt | 28.010 | 27.994 | 28.006 | 27.990 |
| Statik SRAM bayt | 1.326 | 1.326 | 1.326 | 1.326 |
| Simülasyon stack derinliği (gözlenen), bayt | 497 | 496 | 497 | 496 |
| Simülasyon stack–heap boşluğu (gözlenen), bayt | 200 | 201 | 200 | 201 |
| EEPROM witness / commit / UART RX | 1 / 1 / 7 | 1 / 1 / 7 | 1 / 1 / 7 | 1 / 1 / 7 |
| Ayrı no-LTO `Store::decode` stack frame, bayt | 44 | 10 | 44 | 10 |

**Temel ayrım:** no-LTO derleyici çerçevesi **34 bayt küçüldü**, fakat gerçek optimize firmware'de görülen toplam stack kazanımı **yalnız 1 bayt**. Eşzamanlı EEPROM/UART/Timer sınırlandırılmış izinde çakışma görülmedi, ancak gerçek dünya en kötü ISR/heap birleşimi, zamanlama ve fiziksel pompa **NOT_RUN/REVIEW**.

Actions işinin genel sonucu **FAIL**: otomatik tanı ayrıştırıcısı `Store<EEPROMClass>::decode` adını aradı, gerçekte GCC `Store<Eeprom>::decode` raporladı. SHA, 4 AVR derlemesi ve simAVR günlükleri doğrudan incelendi; ayrıştırıcı FAIL'inin üzerine *job PASS* yazılmadı. Elde edilen 1 bayt gerçek kazanç nedeniyle aynı derlemeyi tekrarlamak anlamlı görülmedi.

## Teslim kararı
- **QA kaynak bulgusu korunur**, ancak fayda/davranış riski dengesi nedeniyle bu onarım **DO_NOT_PROMOTE**. Ana firmware veya ilgili EEPROM header'ına aktarım, sürüm atlatma, commit/push yok.
- Geçici owner-only Actions workflow'u ve hatalı otomatik ayrıştırma betiği aktif depodan kaldırılır. `qa/p004` altında yalnız inceleme kaynakları ve bu rapor kalır.
- `AVR_CANDIDATE_2/2_BUILD_PASS` (dört A/B derleme), `HOST_CPP_TARGETED_PASS`, `SIMAVR_BOUNDED_TRACE_PASS` kayıtları kabul kapsamıyla sınırlıdır.
- `STACK_ALL_PATHS_REVIEW` · `PHYSICAL_NOT_RUN` · `RELEASE_BLOCKED`.
- Eski v1.04 arşivi, v1.05 manifesto, P0-04 saatlik otomasyonu ve üretim v1.06 blob'u `957f963c1d4f59a04f18a896e9f6c92890061a49` korunur.
