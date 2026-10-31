# Peristaltic Pump Software v1.05 — Geliştirme sürümü

**Durum:** Geliştirme başlangıcı / fiziksel kabul edilmedi  
**Tarih:** 2026-10-31  
**Temel sürüm:** v1.04 (`main`, başlangıç HEAD `b3b7b2fb859534ae2e7be5edb35da92c7f1cd690`)  
**Konum:** `firmware/Peristaltic_Pump_Software_v1.05/`

## Sabit revizyon kontrolü

**Zorunlu belgeler:** [REVISION_MANIFEST.md](REVISION_MANIFEST.md) (değişmez koruma ve kabul şartları), [NEXT_TASK.md](NEXT_TASK.md) (tek aktif iş), [TEST_MATRIX.md](TEST_MATRIX.md) (modül bazlı pozitif-negatif regresyon) ve [kök AGENTS.md](../../AGENTS.md) (yeni oturum devralma).

İlk aktif iş: **P0-01 test-first**. Kodu değiştirmeden referans testleri oluşturma ve doğrulama. Native/AVR ve gerçek cihaz kabulü henüz NOT_RUN; statik veya model sonuçları native PASS değildir.

## Endüstriyel kalite ekibi ve ticari ürün kabulü

- [Dokuz rol ve bağımsız inceleme protokolü](QUALITY_TEAM.md)
- [Endüstriyel RELEASE_BLOCKED / G01–G10 kabul kapıları](INDUSTRIAL_RELEASE_GATE.md)
- [Risk kayıt defteri R-001–R-012](RISK_REGISTER.md)
- [Gerçek kaynak C++ hesap testi hazırlığı](tests/native-math-harness.cjs)

Bu dokümanlar **uygunluk sertifikası veya bağımsız laboratuvar testi değildir**. Birden fazla yapay zekâ rolünün değerlendirmesi bağımsız uzman sertifikası sayılmaz. Kök AGENTS.md ve kilitli REVISION_MANIFEST.md aynen geçerlidir.

## Kurulum donanım profilleri (tasarım aşamasında)

Kullanıcının talep ettiği kart/sürücü seçimleri: **Pro Mini 5V/16MHz veya 3.3V/8MHz** ve **A4988 veya DRV8825**. Ayrı [kurulum ayarları tasarımı](HARDWARE_SETTINGS_SPEC.md) ve [19 senaryolu C++ profil kapısı](tests/hw-profile-gate.cpp) hazırlandı. **Menü henüz çalışan .ino'ya eklenmedi**; iki farklı `F_CPU` için gerçek Arduino derlemesi ve fiziksel doğrulama bekliyor. Menü kartın fiziksel saatini veya gerilimini değiştirmez.

## Sürümün oluşturulması

Bu klasördeki `Peristaltic_Pump_Software_v1.05.ino` dosyası v1.04 kaynak dosyasının **değiştirilmemiş kopyasıdır**. Dosya adı v1.05 olarak düzenlenmiş, ancak kodun içeriği ve kontrol mantığı henüz değiştirilmemiştir. `firmware/archive/Peristaltic_Pump_Software_v1.04/` arşivlenmiş referans sürüm olarak korunur.

> v1.05 aktif geliştirme sürümüdür; henüz tamamlanmış veya fiziksel olarak kabul edilmiş değildir.

## Devam eden fonksiyonlar (v1.04 ile aynı)

- Dose, Pump ve 30 dönüşlük Cal. çalışma biçimleri
- Encoder, menü ve 16x2 LCD kontrolü
- D5 Remote Pins: Hold / Pulse başlangıç ve durdurma
- A0 RNG: komut edilen adım durumunu bildiren çıkış (gerçek akış sensörü değildir)
- EEPROM ayar kaydı ve seri port USB komutları

## Önceliklendirilmiş geliştirme planı

1. **P0-01 — Sayısal güvenlik:** Kalibrasyon/hız sıfırı, hesap bölmeleri, taşma ve geçersiz step gecikmesi; önce hedefli pozitif-negatif test.
2. **P0-02 — Stop ve motor zamanlaması:** Bloklayan gecikmeler ve durma yanıtı için sınır denemeleri.
3. **P0-03 — USB komut doğrulaması:** Eksik, negatif, sıfır ve hatalı sayılar; durum takibi.
4. **P1 — Remote/EEPROM/encoder/LCD:** Mevcut davranışları bozmayan regresyon testleri.
5. **P2 — Arduino Pro Mini derleme, pin doğrulaması ve belgeler:** Derleyici/sanal test ile fiziksel kart kabulünü ayrı raporlama.

## Güvenlik ve kabul ölçütleri

- Tespit et → kaynakta minimal onar → pozitif-negatif test et → yalnız doğrulanan değişikliği gönder → uzak HEAD/blob SHA kontrol et.
- Çalışan v1.04 sürümüne dokunma. Henüz kanıtlanmamış fonksiyonlara PASS / COMPLETE yazma.
- Motor, pompa ve elektriksel bağlantıların gerçek donanımda kabulü ayrıca yapılmalıdır.
- GitHub Actions ve force-push kullanılmaz; teslim commit'lerinde `[skip ci]` bulunur.

## Sürüm geçmişi

| Tarih | İşlem | Sonuç |
|---|---|---|
| 2026-10-31 | v1.04 dosyası değişmeden v1.05 adıyla kopyalandı; ayrı README oluşturuldu | Başlangıç; uygulama davranışı değiştirilmedi |
