# PerPompa v1.05 — Endüstriyel Risk ve Kusur Kayıt Defteri
**Tarih:** 2026-10-31 | **Sürüm:** 1.0 | **Aktif firmware görevi:** P0-01
Risk seviyesi ilk triage içindir; FMEA sayısal S/O/D puanları, ürün kullanım koşulları bilinmeden uydurulmayacaktır. Hiçbir açık kritik kayıt kapatılmadı.

| ID | Öncelik | Tehlike / tetikleyici | Kaynakta mevcut delil | Koruma/kabul talebi | Statü |
|---|---|---|---|---|---|
| R-001 | R0 | Boş/bozuk EEPROM Remote slotu Hold'a iner; giriş kapalıysa açılışta motoru başlatabilir | `setup()` geçersiz değeri 0 yapıyor; Hold=0 | Açılış/restart risk analizi + gerçek giriş testleri | REVIEW |
| R-002 | R0 | Kontrol döngüsü bloklandığında stop tepkisi uzar | `pump/dose` beklemeli STEP üretimi | Maksimum kabul stop latency ve cihaz testi | REVIEW |
| R-003 | R0 | Motor sıkışması/akış olmamasında RNG yanlış proses onayı sanılır | A0 yalnız komut edilmiş adım durumunu bildirir | Kullanım kılavuzu ve gerekiyorsa fiziksel akış izleme / bağımsız güvenlik | REVIEW |
| R-004 | R1 | Kalibrasyon/hız sıfır → aritmetik nonfinite, geçersiz step | `steps_calc`, `delay_us_calc`; host C++ UBSan iki geçersiz dönüşüm raporladı | AVR ve ana fonksiyon çağıran yollar; geçerli değerlerde değişmezlik | HOST_CPP_CONFIRMED / AVR REVIEW |
| R-005 | R1 | Düşük hız / büyük hacim → AVR int/delay sınırı | `long` gecikme `int` parametresine geçiyor | AVR 8/16MHz gerçek timing, sınır doğrulaması | REVIEW |
| R-006 | R1 | EEPROM'dan kayıtlı CCW açılışta DIR pinine uygulanmıyor | `setup()` sabit LOW; menü güncellemesinde DIR yazılıyor | CW/CCW+enerji açılış testi | REVIEW |
| R-007 | R1 | USB geçersiz/eksik komut aktif işlem/RNG durumunu karıştırır | `inChar` son karakter ve çalışma modu olarak kullanılıyor | Seri bozucu/fault injection ve yanlış step/RNG regresyonu | REVIEW |
| R-008 | R1 | Debi/dozaj hatası: hortum yıpranması, basınç, sıcaklık vb. | Fiziksel metroloji yok | Tekrarlanabilirlik, doğruluk, belirsizlik, ömür testi | NOT_RUN |
| R-009 | R0 | Güç kalitesi/EMC/ESD olayında beklenmedik start veya duramama | Donanım doğrulaması yok | Elektriksel güvenlik ve EMC laboratuvar testleri | NOT_RUN |
| R-010 | R1 | Encoder tık hatası veya menü kayması | v1.04 filtreleme var, fiziksel tık doğrulaması yok | Mevcut algoritmayı koruyup fiziksel regresyon | REVIEW |
| R-011 | R1 | Kayıt bozulması/EEPROM aşınması | Alan bazlı EEPROM.write, bütünlük kaydı yok | Sürüm/CRC/enerji kesme, ömür analizinin uygunluğu | REVIEW |
| R-012 | R0 | Yazılımın tek başına emniyet işlevi sanılması | Ayrı emniyet donanımı gösterilmedi | Risk değerlendirme, uygun güvenlik mimarisi uzman denetimi | NOT_RUN |

**Sıra:** R0 bağımsız tehlike analizi ilk iş; firmware değişikliğinde manifestin tek aktif P0-01 işi korunur. Kritik gözlemler yeni kanıt geldikçe tarihlenmiş olarak güncellenir. Bu kayıt, donanımda ölçülmüş arıza anlamına gelmez.
