# PerPompa v1.06 — İlk enerji öncesi EEPROM / SRAM QA (2026-11-03)

**Değişiklik türü:** Yalnız güvenlik testi / açık risk kaydı. Firmware veya EEPROM formatı değiştirilmedi.

## Kanıt kapsamı
- Canlı `main` başlangıç: `5707c87cddcb8c229742ec61c421cbfc82ac4759`; v1.06 `.ino` Git blob `55b19a8b1017bf1924dc568439f920ec9c90064f`.
- Test başlıkları doğrudan eşleşen Git blobları:
  - `p004-migration-guard-candidate.h`: `b2f342cd83626a4532d4ae1e136832923cf7d3b9`
  - `p0-04-settings-store.h`: `566dd4cdd43685e45998ec75029a71c9e367e3eb`
  - `hw-setup-runtime.h`: `574920f1228bc7db9237a816a3a6fab7dce85eb2`
  - `hw-setup-profile.h`: `4e4bcc7e13730917c436137edd3ce56d6c09d09e`
- Kaynak test aracı SHA256: `852d26d341ed4f1b6ce70a27a5d259fabb1ca7a5f85258ea04ce95f7bf925b39`.
- Linux g++ ve clang++ C++17 / `-Wall -Wextra -Werror -pedantic -fsanitize=undefined`: ayrı ayrı **229/229 PASS**. Yazı sırasında kesilme simülasyonu her derleyicide **82 nokta**.
- Negatif iki mutasyon **2/2 BLOCKED_RUNTIME**: çift EEPROM tanık kontrolünün atlanması ve STOP engelinin atlanması. Derleme/çalışma hataları açıkça ayrıldı.

## Açık risk — ilk EEPROM kaydı
- Boş EEPROM'da ilk kayda ait **50** farklı kesilme noktası incelendi. Sonraki boot: **48 LOCKED**, **1 LEGACY_NEEDS_REVIEW**, **1 NEW_READY**.
- Her `NEW_READY` durumunda veri tutarlılığı PASS. Ancak `LOCKED` koruması, otomatik kullanıcı kurtarması bulunmadığından, ilk fiziksel kurulumda beklenmedik servis engeline dönüşebilir.
- **Karar:** Motor izni vermek için bu kilidi gevşetme. Elektrik kesintisi/ilk kayıt kurtarma işlemi ve kullanıcı onayı için ayrıca güvenli tasarım ve tam AVR/fiziksel kabul gerekiyor. P0-04 saatlik otomasyonunun çalışmalarına karışma.

## Açık risk — RAM
- AVR derleme: 1.368 / 2.048 bayt statik SRAM; *raporlanan* boş 680 bayt. Heap ve stack tepe kullanımı ölçülmedi.
- Gerçek `.ino`da `String inputString = "";` ve `inputString.reserve(200);` bulunuyor; değişkenin uygulamada başka referansı yok. Bu gereksiz dinamik RAM tahsis girişimi olarak incelenmeli.
- **Karar:** Motor/USB/EEPROM kaynaklarıyla birlikte otomatik temizlik yapma; yalnız eşitlik + tam firmware host/negatif + AVR 2/2 kanıtı gelirse minimal değişiklik değerlendir.

**Ayrı durumlar:** HOST_CPP=PASS, FIRST_SAVE_RECOVERY=REVIEW, SRAM_STACK=NOT_RUN, PHYSICAL_POWER_CUT=NOT_RUN, HW_PUMP=NOT_RUN, RELEASE_BLOCKED.

**Not:** Simüle edilen EEPROM kesintileri gerçek güç düşümü, EEPROM fiziksel yazma atomikliği veya sürücü çıkışlarını kanıtlamaz. Mevcut `TEMPORARY PROPOSAL ONLY. DO NOT FLASH.` başlık kilidi korunur.
