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
