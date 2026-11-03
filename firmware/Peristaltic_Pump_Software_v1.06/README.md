# PerPompa v1.06 — v1.05 + I²C LCD

**Durum: DEVELOPMENT / AVR_NOT_RUN / PHYSICAL_NOT_RUN / RELEASE_BLOCKED.**

v1.06 hedefi: GitHub main üzerindeki doğrulanmış v1.05 kaynağının pompa davranışı, menü, enkoder, Remote kontak, USB kontrol, kalibrasyon, motor STEP, RNG ve EEPROM güvenliği değiştirilmeden kopyalanması; yalnız HD44780 16×2 LCD sürücüsünün 4-bit paralel bağlantıdan PCF8574 I²C backpack (`hd44780_I2Cexp`, `Wire`) bağlantısına geçirilmesi.

- Motor, Remote/USB, kalibrasyon, menü ve kayıt işlevleri v1.05 ile aynı tutulur. Başka özellik, menü seçeneği veya EEPROM kaydı eklenmez.
- LCD: SDA=A4, SCL=A5; önceki paralel LCD D8–D13 sinyalleri artık kullanılmaz.
- 5 V/16 MHz ve 3,3 V/8 MHz gerçek Arduino Pro Mini AVR derlemesi 2/2 PASS, kaynak farkı ve güvenlik kontrolleri doğrulanmadan v1.06 firmware main'e aktarılmaz.
- 3,3 V kontrolcüde PCF8574 I²C pull-up gerilimi ve seviye uyumu fiziksel olarak doğrulanmalıdır.
- Türkçe kullanıcı kılavuzu `docs/Peristaltik_Pompa_Kullanici_Kilavuzu_v1.04.pdf` korunur; Engineering Manual gerçek kabulden sonra hazırlanır.
- Kök `AGENTS.md`, v1.05 `REVISION_MANIFEST.md`, arşiv v1.04 ve P0-04 saatlik otomasyonu korunur. v1.05'i arşivleme HOLD.

Bu klasör henüz onaylı, yüklenebilir v1.06 firmware içermez.
