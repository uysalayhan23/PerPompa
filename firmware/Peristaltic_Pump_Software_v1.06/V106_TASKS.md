# PerPompa v1.06 — LCD-only kabul planı (2026-11-03)

Hedef: v1.05 ile yalnız 16×2 LCD'nin PCF8574 I²C sürücüsü farklı olacak. Başka firmware, EEPROM, motor, menü veya iletişim işlevi eklenmeyecek.

1. Canlı main v1.05 .ino ve bağlı başlık dosyalarının Git blob SHA'larını doğrula.
2. Yalnız `LiquidCrystal` başlığını `Wire + hd44780_I2Cexp` ile, paralel LCD nesnesini I²C nesnesiyle değiştir. LCD dışı kaynak farkı sıfır olmalı.
3. Eski menü, STEP, Remote, USB, kalibrasyon, RNG ve EEPROM/P0-04 güvenliklerini pozitif ve negatif testlerle koru.
4. Her iki Arduino Pro Mini (16/8 MHz) gerçek AVR derlemesi 2/2 PASS, Flash/SRAM ve kaynak bütünlüğü doğrulanmadan v1.06 .ino için commit yapma.
5. Gerçek LCD/enkoder ve pompa güvenliği fiziksel NOT_RUN; RELEASE_BLOCKED sürer.
6. v1.05 klasörünün ilgili bütün dosyalarla arşive taşınması HOLD: P0-04 otomasyonu ve politika devri için ayrıca güvenli kabul gerekir.
7. Mevcut Türkçe kullanıcı kılavuzu korunur; Engineering Manual en son.

Sadece main, [skip ci], force-push/Work/Codex/Actions yok. Kök AGENTS.md, v1.05 REVISION_MANIFEST.md ve v1.04 arşivi değişmez. NOT_RUN hiçbir koşulda PASS sayılmaz.
