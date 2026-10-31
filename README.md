# PerPompa

Arduino Pro Mini ile çalışan peristaltik pompa için yazılım, Türkçe kullanıcı kılavuzu ve Remote Pins/RNG devre notları.

## Sürüm politikası

- **v1.04:** [`firmware/archive/Peristaltic_Pump_Software_v1.04`](firmware/archive/Peristaltic_Pump_Software_v1.04) içinde korunan referans sürüm.
- **v1.05 (geliştirme):** [`firmware/Peristaltic_Pump_Software_v1.05`](firmware/Peristaltic_Pump_Software_v1.05) altında aktif çalışma kopyası. Başlangıç kodu v1.04 ile birebir aynıdır; henüz motor veya kullanıcı davranışında değişiklik yoktur. Geliştirme notları kendi README dosyasındadır.

## Dosyalar

| [Aktif yazılım v1.05](firmware/Peristaltic_Pump_Software_v1.05/Peristaltic_Pump_Software_v1.05.ino) | v1.04 ile aynı kod, yeni geliştirmelerin başlangıç noktası |
| [v1.05 README](firmware/Peristaltic_Pump_Software_v1.05/README.md) | Geliştirme planı ve kabul kuralları |

| Dosya | İçerik |
|---|---|
| [Arşivlenen v1.04](firmware/archive/Peristaltic_Pump_Software_v1.04/Peristaltic_Pump_Software_v1.04.ino) | Pompa kontrolü, Remote Pins start/stop, RNG çalışma sinyali ve iyileştirilmiş encoder/menü kullanımı |
| [v1.04 arşiv notları](firmware/archive/Peristaltic_Pump_Software_v1.04/README.md) | Değişiklikler, test edilen davranışlar ve henüz doğrulanmayan noktalar |
| [Türkçe kullanıcı kılavuzu](docs/Peristaltik_Pompa_Kullanici_Kilavuzu_v1.04.pdf) | Ürün, ilk start, menüler, çalışma modları, kalibrasyon, Remote Pins ve RNG kuru kontak anlatımı |
| [Teknik bağlantı README](docs/README_Remote_Pins_RNG_Devre.md) | D5 Remote Pins girişi, A0 RNG sinyali, iç röle sürücüsü ve PLC bağlantı notları |
| [Teknik devre şeması](docs/Remote_Pins_RNG_Devre_Semasi.pdf) | Remote Pins ve RNG bağlantılarının iki sayfalık çizimi |
| [Kılavuz kapak görseli](docs/assets/cover_no_text.png) | Yan panel yazısı kaldırılmış ürün fotoğrafı |
| [Önceki yazılımlar](firmware/archive) | v1.01–v1.04 önceki sürümleri |

## Pompanın çalışma şekli

- **Dose:** Ayarlanan miktara ulaşınca durur.
- **Pump:** Seçilen hızda, stop verilene kadar çalışır.
- **Cal.:** 30 dönüşlük kalibrasyon çevrimi yapar. Ölçülen toplam sıvı miktarı `7|Cal.` menüsüne girilir.

`Remote Pins` yalnızca start/stop komutu verir. Hangi işlemin yapılacağını `6|Mode:` seçimi ve ilgili hacim/hız ayarları belirler. **Hold** seçildiğinde kontak kapanması start, uzaktan başlatılmış çalışma sırasında açılması stop verir. **Pulse** seçildiğinde her yeni kontak kapanışı, pompa durmuşsa start, çalışıyorsa stop verir. Ayrıntılı örnekler kullanıcı kılavuzundadır.

`RNG` çalışma çıkışı, ürün içine röle eklendiğinde dışarıya **normalde açık, potansiyelsiz kuru kontak** olarak verilir. Bu kontak yazılımın motoru çalıştırma komutunu bildirir; gerçek sıvı akışını ölçmez. Röle montajı tamamlanmadan kartın A0 pini kuru kontak değildir. Elektriksel bağlantı için teknik README ve şemaya bakın.

## Yazılımı açma

1. [Aktif v1.05 `.ino` dosyasını](firmware/Peristaltic_Pump_Software_v1.05/Peristaltic_Pump_Software_v1.05.ino) bulunduğu klasörle birlikte Arduino IDE'de açın.
2. Arduino Pro Mini kartını ve kartınızın gerçek saat/gerilim seçeneğini belirleyin.
3. `ClickEncoder` ve `TimerOne` kütüphanelerini kurun. `LiquidCrystal` ve `EEPROM` de kullanılır.
4. Devre bağlantılarını teknik şemaya göre kontrol edip yazılımı karta yükleyin.

PLC'nin 24 V çıkışını Remote Pins girişine doğrudan bağlamayın. A0 pinine çıplak röle bobini bağlamayın; iç röle için uygun sürücü ve koruma diyodu kullanın.

## Sürüm durumu

v1.05 henüz v1.04'ten kopyalanmış geliştirme sürümüdür; yeni sürüm için derleme veya fiziksel kabul yapılmadı. v1.04 için yerel C++ sözdizimi kontrolü ve simüle menü ile Pump start/stop denemeleri geçti. Bu çalışma sırasında Arduino derleyicisi ve fiziksel kart bulunmadığından gerçek kart derlemesi ve donanım testi henüz yapılmadı. Encoderın mekanik tık yapısına göre `ENCODER_STEPS_PER_NOTCH` değerinin doğrulanması gerekebilir.

## Kaynak ve kütüphaneler

Yazılımın başlangıç noktası [iGEM Aachen Open-Source-Peristaltic-Pump](https://github.com/iGEM-Aachen/Open-Source-Peristaltic-Pump) projesinin v1.01 dosyasıdır. Projede kullanılan encoder kütüphanesi [0xPIT/encoder](https://github.com/0xPIT/encoder), zamanlayıcı kütüphanesi [TimerOne](https://github.com/PaulStoffregen/TimerOne) projesidir. Bu depoda orijinal kaynak için yeni bir lisans beyanı eklenmemiştir.
