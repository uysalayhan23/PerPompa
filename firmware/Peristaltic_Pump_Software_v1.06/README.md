# PerPompa v1.06 — v1.05 + I²C LCD

**Durum:** AVR 8/16 MHz BUILD PASS; LCD ve pompa fiziksel testleri NOT_RUN; RELEASE_BLOCKED. v1.05 aktif firmware ve P0-04 çalışma yolu olarak korunmaktadır.

## Tek işlevsel değişiklik
v1.05'teki HD44780 16×2 LCD, D8–D13 paralel sürücüsü yerine PCF8574 tabanlı I²C backpack ile `Wire + hd44780_I2Cexp` üzerinden çalıştırılır. SDA=A4, SCL=A5. 16×2 metinler ve enkoder etkileşimi korunur.

v1.05'e göre yalnız LCD sürücüsü ve ekranın ilk başlatılamaması halinde motorun pasif kalması güvenlik kapısı eklenmiştir. Güncel v1.06 .ino Git blob SHA: `55b19a8b1017bf1924dc568439f920ec9c90064f`. Önceki LCD-only sürüm SHA: `7933e4fe697a8778603ba553eb620e7788dc5ede`. Referans v1.05 kaynak SHA: `b737292fdf33eec5aa62dac662d698f0dc705c01`.

Diğer altı header birebir v1.05 Git bloblarıdır. Motor STEP, menü, kalibrasyon, USB, Remote giriş, RNG, EEPROM/P0-04 ve güvenlik kuralları değişmedi. Yeni alan protokolü, ağ menüsü veya haberleşme donanımı eklenmemiştir.

## Derleme kanıtı
Windows / Arduino CLI 1.5.2-rc.1 / AVR Boards 1.8.8 / hd44780 1.3.2, TimerOne 1.1.1, ClickEncoder. **Güncel LCD fail-closed adayı:** 16MHz: Flash 27.540/30.720, RAM 1.368/2.048 (kalan 680). 8MHz: Flash 27.536/30.720, RAM 1.368/2.048 (kalan 680). `AVR_ACCEPTANCE.md` ayrıntılı kayıt. Boş RAM değeri stack için garantili güvenlik marjı sayılmaz.

Fiziksel LCD, kart ve pompa doğrulanmadığı için bu kaynak sürümü henüz üretim kullanımına onaylı değildir. Kök `AGENTS.md`, v1.05 manifestosu ve P0-04 otomasyonu yerinde kalır. v1.05 arşivleme HOLD. Mevcut PDF kullanıcı kılavuzu korunur; Engineering Manual fiziksel kabulden sonra.
