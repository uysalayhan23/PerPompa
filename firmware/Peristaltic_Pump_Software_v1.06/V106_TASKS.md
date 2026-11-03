# PerPompa v1.06 — LCD-only kabul durumu (2026-11-03)

**Kapsam:** v1.05 ile tek firmware farkı 16×2 LCD'nin PCF8574 I²C sürücüsüne çevrilmesidir. Eski haberleşme prototipleri kaldırılmıştır.

| Kabul | Durum |
|---|---|
| v1.05 kaynak blob ve altı header SHA | PASS |
| v1.05 ile LCD dışı kaynak farkı | ZERO / PASS |
| AVR 5V/16MHz | PASS — 27486 Flash, 1367 SRAM |
| AVR 3.3V/8MHz | PASS — 27482 Flash, 1367 SRAM |
| Yeni .ino Git blob SHA | 7933e4fe697a8778603ba553eb620e7788dc5ede |
| Gerçek LCD/I²C ve enkoder testleri | NOT_RUN |
| Motor, EEPROM, Remote, USB ve pompa fiziksel kabulü | NOT_RUN |
| Ticari üretim | RELEASE_BLOCKED |

**Önemli:** Kaynak eşitliği ve AVR derleme PASS, fiziksel güvenliği kanıtlamaz. 681 bayt kalan SRAM dinamik yığın güvenliği anlamına gelmez.

v1.05 ve P0-04 otomasyonu yerinde çalışmaya devam eder; v1.05'in tüm dosyalarıyla arşive taşınması ancak açık devir kararı ve uygun kabulden sonra yapılabilir (HOLD). Kök AGENTS.md, v1.05 REVISION_MANIFEST.md, v1.04 arşivi, kullanıcı kılavuzu değiştirilmez. Sadece main + [skip ci], Actions/Work/Codex/force-push yok.
