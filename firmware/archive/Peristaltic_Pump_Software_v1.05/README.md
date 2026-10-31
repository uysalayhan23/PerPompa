# Peristaltic Pump Software v1.05 — Geliştirme sürümü

**Durum:** Geliştirme başlangıcı / fiziksel kabul edilmedi  
**Tarih:** 2026-10-31  
**Temel sürüm:** v1.04 (`main`, başlangıç HEAD `b3b7b2fb859534ae2e7be5edb35da92c7f1cd690`)  
**Konum:** `firmware/archive/Peristaltic_Pump_Software_v1.05/`

## Sürümün oluşturulması

Bu klasördeki `Peristaltic_Pump_Software_v1.05.ino` dosyası v1.04 kaynak dosyasının **değiştirilmemiş kopyasıdır**. Dosya adı v1.05 olarak düzenlenmiş, ancak kodun içeriği ve kontrol mantığı henüz değiştirilmemiştir. `firmware/Peristaltic_Pump_Software_v1.04/` referans sürüm olarak korunur.

> `archive` dizini içinde bulunması, v1.05'in tamamlandığı anlamına gelmez; kullanıcının isteğiyle burada üzerinde çalışılacak sürüm tutulur.

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
