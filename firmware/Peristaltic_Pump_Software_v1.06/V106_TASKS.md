# PerPompa v1.06 — İş listesi ve sürüm devri (2026-11-02)

Durum: DEVELOPMENT ONLY, NOT FLASHABLE. Korunan temel: v1.04 archive; mevcut v1.05 firmware ve çalışan P0-04 saatlik otomasyonu. Kök AGENTS.md ve v1.05 REVISION_MANIFEST.md kilitli; şimdi değiştirilmeyecek. Sadece main + [skip ci], force-push/Actions/Work/Codex yok. Test edilmemiş iş PASS sayılmaz.

## Kılavuz kararı
- **Tek mevcut kullanıcı kılavuzu:** docs/Peristaltik_Pompa_Kullanici_Kilavuzu_v1.04.pdf. Eski Türkçe PDF aynen kalacak. Yeni bir User Manual hazırlanmayacak.
- docs/Peristaltik_Pompa_Modbus_Eki_v1.06.md dosyasındaki **yalnız Modbus kullanım bölümü** v1.06 gerçek ekran/haberleşme kabulünden sonra mevcut kılavuzun yeni revizyonuna dahil edilecek.
- **Engineering Manual en son**: entegre firmware, gerçek register haritası, PLC/I²C/RS485 bağlantıları, gerçek test ölçümleri ve fiziksel kabul tamamlandıktan sonra yeni nihai teknik doküman hazırlanacak. Önceki erken Engineering ve User Manual preview dosyaları kaldırıldı; teknik taslaklar HARDWARE.md ve MODBUS_REGISTER_MAP.md içinde kalır.

## Ek görev: v1.05'in eksiksiz arşivlenmesi
Kullanıcı v1.05'in **tüm ilgili kaynakları, header, test, QA, doküman, manifesto ve test matrisleriyle beraber** firmware/archive/Peristaltic_Pump_Software_v1.05/ altına taşınmasını istiyor. Yalnız .ino'yu taşımak YASAK; Git blob/hash ve referans bütünlüğü korunacak.

**Ön koşul (HOLD):** P0-04 saatlik otomasyonu halen v1.05 yolunu kullanıyor, ayrıca kök AGENTS.md mevcut tek aktif firmware'i v1.05 diye tanımlıyor. v1.06 doğrulanmadan, P0-04 kaynak yaması kabul edilmeden ve otomasyon devralması ile kilitli politika güncellemesi için kullanıcı onayı olmadan bu klasörü taşıma/silme. Gerekli aşamada otomasyon, doküman linkleri ve firmware yol referanslarını atomik/güvenli geçişle güncelle; v1.04 arşivini değiştirme. Bu oturumdaki taşıma: NOT_RUN / HOLD.

## Öncelik ve kapılar

| Sıra | ID | Yapılacak iş | Durum |
|---|---|---|---|
| 01 | V106-01 | Saf Modbus RTU alıcı, CRC ve zamanlama | HOST_CPP önceki 101/101 x4; AVR NOT_RUN |
| 02 | V106-02 | Baud/Node/Parity/Stop menü modeli, EEPROM | HOST_CPP önceki 126/126 x4; AVR NOT_RUN |
| 03 | V106-03 | 16x2 LCD köprü adaptörü, ClickEncoder olayları, idle güvenliği | HOST_CPP 622/622 x4; fiziksel LCD NOT_RUN |
| 04 | V106-04 | Geçerli v1.05 motor ve P0-04 güvenliklerini v1.06 ana firmware'e aktarma | NOT_RUN; paralel otomasyon korunsun |
| 05 | V106-05 | Gerçek PCF8574 I²C LCD ve enkoder menü entegrasyonu | NOT_RUN |
| 06 | V106-06 | UART RS485 DE/RE ve MODBUS RTU RX zamanlaması, STEP bloklaması, bootloader | NOT_RUN |
| 07 | V106-07 | FC03/04/06/16, exception, register bütünlüğü, parametre doğrulama | NOT_RUN |
| 08 | V106-08 | PLC kontrol sahipliği, güvenli STOP/START, doz/hız, kalibrasyon | NOT_RUN |
| 09 | V106-09 | Watchdog, bağlantı kesilmesi, yarım frame, EEPROM/enerji kesintisi | NOT_RUN |
| 10 | V106-10 | Tam gerçek kaynak native + gerçek Arduino Pro Mini 8/16 MHz AVR ve bellek/timing | NOT_RUN |
| 11 | V106-11 | Gerçek PLC, RS485 ve I²C LCD, motor/doz/STOP fiziksel kabul | PHYSICAL_NOT_RUN |
| 12 | V106-12 | v1.06 devralma; P0-04 otomasyon ve politika için yetkili geçiş | HOLD |
| 13 | V106-13 | **v1.05 tüm ilgili dosyalarıyla firmware/archive içine taşı, SHA/yol referanslarını doğrula** | HOLD, devralma sonrası |
| 14 | V106-14 | Mevcut Türkçe kullanıcı PDF'sine yalnız Modbus bölümünü ekle | Son kullanıcı kabulünde |
| 15 | V106-15 | **Engineering Manual FINAL** | EN SON |

Yöntem: tespit et → en küçük kaynak onarımı → pozitif/negatif gerçek native test → gerçek 8/16MHz AVR → [skip ci] main teslimi ve HEAD/blob SHA. Gereken fiziksel G01-G10 olmadan endüstriyel RELEASE_BLOCKED korunur. RNG gerçek sıvı akış ölçümü değildir.
