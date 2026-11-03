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
