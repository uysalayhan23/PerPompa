# P0-01 — Aday revizyon / değişiklik kapısı
**Tarih:** 2026-10-31 | **Durum:** CANDIDATE_ONLY / FIRMWARE_NOT_DELIVERED
**Orijinal kaynak:** v1.05 `.ino` blob `0d965c08947ba0b3170d22fbff806fc2e6f1e83a`.

## Tespit — doğrudan özgün fonksiyon C++ kanıtı
G++ 14.2.0 sanitizer altında orijinal `steps_calc()`, `delay_us_calc()` gövdeleri izole derlendi:
- 4 geçerli hesap PASS (4800 mikro-adım, 625 µs yarım periyot, 2400 mikro-adım/3 tur, 3750 µs/10 rpm).
- Kalibrasyon = 0 ve hız = 0 için iki ayrı inf→long UBSan hatası yeniden üretildi.
- Negatif hız hesaplanan negatif gecikme oluşturdu; çok düşük hız >16-bit gecikme üretiyor; büyük hacim/küçük kalibrasyonun hedef AVR 32-bit long sınırını aşması kaydedildi.

## Aday (yalnız yerel geçici test)
Geçerli parametreleri koruyan hesap adayında: hacim/hız <=0, geçersiz mode/decimals, gereken durumda calibr<=0, signed-32 steps aşımı, yarım periyot için güvenli değer aralığı dışı koşullar **0 (geçersiz)** döndürüyor. C++ testinde 4 pozitif + 16 negatif + 3098 geçerli değer paritesi geçti. Bu, 8/16 MHz AVR gerçek motor zamanlaması kanıtı değildir; modelin varsayımsal zamanlama üst sınırı onaylanmadı.

**Kritik eksik:** Hesap fonksiyonundan 0 döndürmek tek başına güvenli firmware onarımı değildir: `pump(0)` ve `dose(_steps>0, 0)` hâlâ STEP darbesi üretebilir. Güvenli adayın `loop`/USB `in_action`, `dose` ve `pump` hata/stop/RNG kapılarını da emniyetli biçimde ele alması zorunludur. Sadece hesap korumasını ana dala gönderme.

## Devam / kabul
1. Hedef AVR kütüphaneleri ve Arduino Pro Mini gerçek 8/16 MHz derlemesi hazırla; şu anda `AVR_COMPILE=NOT_RUN`.
2. Negatif girişte **STEP=0 darbe, RNG idle, yerel/Remote/USB kontrol durumları tutarlı** kanıtı; pozitif normal davranışların eşdeğerliği, Cal 30 tur ve maksimum stop süresi.
3. Hedefli gerçek kaynak pozitif-negatif ve tüm ilgili regresyon PASS + kart derleme PASS olmadan `.ino` main commit yok.
4. Motor sürücüsü, elektrik emniyeti ve debi fiziksel kabulü ayrıca `NOT_RUN`. Ticari `RELEASE_BLOCKED`.

**Fayda/riski:** Bilinmeyen girişlerde kontrolsüz hız/step riskini azaltır; düşük hızları reddetmek ürünün kullanılabilir aralığını daraltabilir. Gerçek ihtiyaç ve tasarım limitleri henüz belirlenmeden bütün düşük hızları otomatik destekliyormuş gibi gösterme. Düzeltmeyi hiç uygulamamak, doğrulanmış sıfır/negatif giriş kusurunu açık bırakır; test kapısı geçmeden uygulamak ise yeni motor/stop riski yaratabilir.
