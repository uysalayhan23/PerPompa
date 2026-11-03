**2026-11-03 P0-04 EEPROM/UART/Timer stack QA (REVIEW ONLY):** Flash LCD MergedGates adayında EEPROM decode 30 B geçici kopyayı kaldıran QA-only öneri, GCC/Clang×8/16 MHz hedefli host karşılaştırması **48/48** + mutant **32/32**, gerçek setup/loop entegrasyonu **16/16** + mutant **8/8**, aynı sayılarla bağımsız ASan+UBSan PASS. GitHub Actions [41653905873](https://github.com/uysalayhan23/PerPompa/actions/runs/41653905873) gerçek 16/8 MHz 4/4 AVR A/B derledi, eşzamanlı EEPROM witness+commit ve 7 UART RX gözlendi. Statik SRAM **1326→1326 B**, optimized simAVR stack/heap boşluğu **200→201 B**; ayrı no-LTO decoder frame 44→10 B, ama gerçek optimize kazanç **yalnız 1 B**. Bu nedenle yama **DO_NOT_PROMOTE**, Actions parser genel sonuç FAIL (tip adı eşleşmedi). [P004_EEPROM_ISR_STACK_REVIEW_20261103.md](P004_EEPROM_ISR_STACK_REVIEW_20261103.md). Üretim değişmedi; STACK_ALL_PATHS_REVIEW, PHYSICAL_NOT_RUN, RELEASE_BLOCKED.

**2026-11-03 P0-04 Flash LCD yeni tam-kaynak host regresyonu (QA ONLY):** SHA256 `5c7009477ba56cb70850f22ece9860fcb222bd4c0a0c3a10622c21b66c79bfa5` adlı geçici adayın gerçek `setup()/loop()` gövdelerinde 12 yeni kaynaklı güvenlik/LCD senaryosu × GCC/Clang × 8/16MHz = **48/48 UBSan PASS**, **48/48 ASan+UBSan PASS**; beş ayrı metin mutantı **20/20 ×2 yakalandı**. Eski EEPROM kesinti ve AVR A/B testleri tekrarlanmadı. [Tam test ve stack risk raporu](P004_FLASH_FULLSOURCE_NATIVE_REVIEW_20261103.md). Mock host ≠ AVR/cihaz; `ALL_PATHS_STACK_PEAK_REVIEW`, `PHYSICAL_NOT_RUN`, `RELEASE_BLOCKED`. Üretim `.ino` blob SHA aynı kaldı.

**2026-11-03 P0-04 RAM kazanımı (QA ONLY):** Yalnız geçici MergedGates v1.06 adayındaki beş LCD `print` satırının sabit metinleri `F()` ile Flash'a taşındı; SHA256 `5c700947...`. GitHub Actions 41645418905 gerçek AVR A/B 16/8 MHz **2/2 aday derleme PASS**, statik SRAM **1436→1326 B**, gözlenen en dar stack–heap açıklığı **95→205 B**, Flash **+78 B**. LCD I2C işlem sayıları eşit ve 6/6 simülatör giriş aşaması görüldü. Bu yalnız sınırlandırılmış izdir; `FULL_NATIVE_NOT_RUN`, `ALL_PATHS_STACK_PEAK_REVIEW`, `PHYSICAL_NOT_RUN`, `RELEASE_BLOCKED`. Üretim firmware'i değişmedi. [P004_SRAM_FLASH_LCD_QA_20261103.md](P004_SRAM_FLASH_LCD_QA_20261103.md).

**2026-11-03 P0-04 stack/heap izleme kararı:** Geçici birleşik v1.06 `.ino` SHA256 `e0d85275…` üzerinde Actions gerçek AVR 8/16 MHz PASS ve 70 milyon adımlı simAVR ölçümü: statik SRAM 1436 B, stack derinliği gözlenen 492 B, heap büyümesi gözlenen 24 B, **gözlenen en dar stack–heap boşluğu yalnız 95 B** (her iki hedefte). Bu tüm yolların gerçek tepe değeri değildir. 16 MHz testinde HW Setup görüldü, 8 MHz testinde görülmedi. `STACK_SIM_TRACE_PASS` / `ALL_PATHS_STACK_PEAK_REVIEW` / `PHYSICAL_NOT_RUN` / `RELEASE_BLOCKED`. [P004_STACK_HEAP_ACTIONS_REVIEW_20261103.md](P004_STACK_HEAP_ACTIONS_REVIEW_20261103.md). Üretim firmware'i, manifestler, otomasyon ve fiziksel kilitler değiştirilmedi.

**2026-11-03 P0-04 REVIEW-ONLY GitHub Actions AVR kabulü:** Tam birleşik geçici v1.06 kaynak SHA256 `e0d852757f74fedc4c00dd9da05b9cf02bf660c72bf7dcaa4afc52c8a7710e21` için gerçek Arduino Pro Mini **8/16MHz AVR 2/2 PASS**: 27928/27932 B Flash, 1436 B statik SRAM, 612 B statik boş alan. Kanıt [P004_ACTIONS_AVR_REVIEW_20261103.md](P004_ACTIONS_AVR_REVIEW_20261103.md), GitHub Actions run `41637720429`. **Bu, RAM-only üretim blob'unun veya EEPROM kurtarma adayının firmware'e kabul edildiği anlamına GELMEZ**; STACK_PEAK_NOT_RUN, PHYSICAL_NOT_RUN ve RELEASE_BLOCKED. Daha eski AVR_NOT_RUN kayıtları bu QA çalıştırmasından önceki durumlardır.

**GÜNCEL RAM-only native kabul ve teslim (03.11.2026):** Güncel v1.06 özgün tam setup()/loop() kaynak GCC/Clang × 8/16 MHz × baseline/RAM **8/8 native koşu ve her koşuda 106/106 kontrol PASS**. LCD init fail-closed ve EEPROM motor kilidi negatif mutantları **2/2 tespit edildi**. RAM-only Git blob `957f963c1d4f59a04f18a896e9f6c92890061a49`; önceden yapılmış gerçek AVR **2/2 PASS** aynı blob için geçerli (tekrarlanmadı). Önceki FULL_LOOP_NOT_RUN kayıtları tarihseldir. `PHYSICAL_NOT_RUN; STACK_PEAK_NOT_RUN; RELEASE_BLOCKED`. EEPROM recovery 17/48 REVIEW-only, 31/48 SERVICE_HOLD; ayrı. Bkz. `RAM_20261103_AVR_SOURCE_QA.md`.

**RAM aday AVR güncellemesi (03.11.2026):** RAM-only `957f963c1d4f59a04f18a896e9f6c92890061a49` Windows Arduino CLI **16/8 MHz 2/2 PASS**, Flash 26760/26756, statik RAM 1362. Kaynak 13/13 ve statik mutant 3/3. **Tam güncel v1.06 setup()/loop() host/native NOT_RUN**, firmware `main`e gönderilmedi. [RAM_20261103_AVR_SOURCE_QA.md](RAM_20261103_AVR_SOURCE_QA.md). Önceki satırlardaki RAM AVR NOT_RUN ifadeleri bu tarihsel kayıttan önceki durumu anlatır; bugünkü son durum AVR=PASS, FULL_LOOP=NOT_RUN.

**ÖNCELİKLİ GÜVENLİK DÜZELTMESİ (2026-11-03):** Önceki EEPROM kurtarma adayındaki 48/48 PASS, bozulmuş commit marker'lı eski kaydın silinebildiği negatif kusuru kapsamıyordu. Kurtarma adayı geçersiz kılındı; tutucu adayda 17/48 kurtarılabilir, 31/48 SERVICE_HOLD. [EEPROM_RECOVERY_SAFETY_REVIEW.md](EEPROM_RECOVERY_SAFETY_REVIEW.md) — `RECOVERY_UI_AVR_NOT_RUN / RELEASE_BLOCKED`. RAM-only 8/16 MHz AVR de `NOT_RUN`.

# PerPompa v1.06 — LCD-only kabul durumu (2026-11-03)

**Kapsam:** v1.05 ile tek firmware farkı 16×2 LCD'nin PCF8574 I²C sürücüsüne çevrilmesidir. Eski haberleşme prototipleri kaldırılmıştır.

| Kabul | Durum |
|---|---|
| v1.05 kaynak blob ve altı header SHA | PASS |
| v1.05 ile LCD dışı kaynak farkı | LCD hata durumundaki motor engelleme kapısı hariç ZERO / PASS |
| AVR 5V/16MHz | PASS — 27540 Flash, 1368 SRAM |
| AVR 3.3V/8MHz | PASS — 27536 Flash, 1368 SRAM |
| Yeni .ino Git blob SHA | 55b19a8b1017bf1924dc568439f920ec9c90064f |
| Gerçek LCD/I²C ve enkoder testleri | NOT_RUN |
| Motor, EEPROM, Remote, USB ve pompa fiziksel kabulü | NOT_RUN |
| Ticari üretim | RELEASE_BLOCKED |

**Önemli:** Kaynak eşitliği ve AVR derleme PASS, fiziksel güvenliği kanıtlamaz. 680 bayt kalan SRAM dinamik yığın güvenliği anlamına gelmez.

v1.05 ve P0-04 otomasyonu yerinde çalışmaya devam eder; v1.05'in tüm dosyalarıyla arşive taşınması ancak açık devir kararı ve uygun kabulden sonra yapılabilir (HOLD). Kök AGENTS.md, v1.05 REVISION_MANIFEST.md, v1.04 arşivi, kullanıcı kılavuzu değiştirilmez. Sadece main + [skip ci], GitHub Actions için 2026-11-03 tarihinde en çok 50 çalıştırma izni verilmiştir; yalnız gerekli testlerde kullanılır. Work/Codex/force-push yok.

## 2026-11-03 EEPROM / SRAM ilk enerji kapısı

- [x] v1.06 birebir Git blob başlıklarıyla 229/229 × 2 host C++ PASS; 82 kesilme noktası × 2 ve 2/2 negatif mutant yakalandı. Kaynak ve yöntem: [PREPOWER_EEPROM_QA.md](PREPOWER_EEPROM_QA.md).
- [ ] **R0:** İlk EEPROM kaydının 50 kesilme konumundan 48'inde güvenli ama kullanıcı açısından otomatik kurtarmasız LOCKED görülüyor. P0-04 otomasyonuyla çakışmadan kurtarma tasarımı/testi ve kullanıcı kabulü — REVIEW.
- [ ] **R0/R1:** Gerçek v1.06 `.ino`da kullanılmayan `inputString.reserve(200)` dinamik RAM girişimi ve 680 bayt statik boş SRAM; heap/stack tepe ölçümü — REVIEW/NOT_RUN.
- [ ] Gerçek I²C, fiziksel EEPROM kesilmesi, motor STOP gecikmesi — PHYSICAL_NOT_RUN; RELEASE_BLOCKED.
- Firmware, v1.05, v1.04, P0-04 otomasyonu ve kilitli manifesto değişmez; bu kayıt tek başına firmware PASS/COMPLETE değildir.

## 2026-11-03 — İlk EEPROM kaydı kurtarma adayı / RAM incelemesi

- [x] Kaynak başlığıyla korumalı kurtarma prototipi: **48/48 kilitli ilk-kayıt**, **1.153** kurtarma-kesinti denemesi; g++ ve clang++ ayrı ayrı **6.579/6.579 HOST_CPP PASS**; 2/2 negatif mutant yakalandı. Detay: [EEPROM_RECOVERY_CANDIDATE_QA.md](EEPROM_RECOVERY_CANDIDATE_QA.md).
- [ ] Gerçek cihazda **açık, fiziksel yerel kurtarma onayı** ve yeniden kalibrasyon UI kabulü; henüz firmware'e bağlanmadı. **REVIEW/AVR_NOT_RUN/PHYSICAL_NOT_RUN**.
- [x] Kullanılmayan iki `inputString` satırının bulunduğu doğrulandı; iki satırlık RAM adayı blob `957f963c1d4f59a04f18a896e9f6c92890061a49` hazırlandı ama `main` firmware değişmedi.
- [ ] RAM adayı için gerçek Arduino Pro Mini **8/16 MHz AVR** ve özgün tam-setup/loop regresyonu; **NOT_RUN**.
- [ ] Çalışma anı heap/stack en yüksek kullanım ölçümü, EEPROM gerçek kesinti, ekran ve motor bench — **NOT_RUN**. **RELEASE_BLOCKED**.

## 2026-11-03 — Kurtarma sırasındaki güç kesintisi yeni stres regresyonu

- [x] Review-only kurtarma kaynak başlıklarıyla GCC/Clang **315371/315371 × 2** kontrol PASS; **161** kurtarma-kesintisi, **0** izinsiz NEW_READY, **0** farklı EEPROM alanı değişikliği. [EEPROM_RECOVERY_CUT_STRESS_QA.md](EEPROM_RECOVERY_CUT_STRESS_QA.md).
- [ ] Tam v1.06 `.ino setup()/loop()` kaynak entegrasyonu/native/AVR — **NOT_RUN**, menü henüz aktif firmware'de yok.
- [ ] RAM-only aday iki AVR derlemesi — **NOT_RUN**; firmware commit **HOLD**.
- [ ] Fiziksel EEPROM ve I²C LCD, durma gecikmesi, heap/stack tepe ölçümü — **NOT_RUN**, **RELEASE_BLOCKED**.
