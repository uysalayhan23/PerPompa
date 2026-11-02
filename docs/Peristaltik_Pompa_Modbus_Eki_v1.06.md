# PerPompa — Türkçe kullanıcı kılavuzu için Modbus RTU bölümü (v1.06 TASLAK)

Bu dosya ayrı bir User Manual değildir. Eski ve korunmuş [Türkçe Kullanıcı Kılavuzu](Peristaltik_Pompa_Kullanici_Kilavuzu_v1.04.pdf) içine **v1.06 kabulünden sonra** yalnız bu Modbus bölümü eklenecek. Mevcut PDF değiştirilmedi.

## Modbus Settings menüsü

| Seçenek | Varsayılan | Seçilebilir değerler |
|---|---|---|
| Baud Rate | **9600** | 2400, 4800, 9600, 19200, 38400, 57600, 115200 |
| Node Address | **1** | 1–247 |
| Parity | **EVEN** | EVEN, ODD, NONE |
| Stop Bits | **1** | 1 veya 2 |
| Save + Apply | — | Boştayken yerel uzun basışla onay |
| Back | — | Kaydetmeden geri dön |

Planlanan kullanım: enkoderi döndürerek alan seç; tıklayarak düzenlemeyi aç; çevirerek değeri değiştir; tıklayarak tamamla; Save + Apply üzerinde yerel uzun basışla kalıcı kaydı onayla. Motor, Remote veya seri kontrol aktifken haberleşme ayarı değiştirilemez. EEPROM bozuksa/modül ayarları çelişiyorsa otomatik start izni verilmez.

PLC ve pompa aynı baud/parity/stop formatında olmalıdır. Bir RS485 hattında eşzamanlı yalnız bir Modbus Master bulunur. Haberleşme parametresini değiştirmek PLC bağlantısını kesebilir. USB/seri bağlantı yalnız firmware yükleme ve güncelleme içindir.

"Motor çalışıyor" bilgisi komut edilen STEP sinyalidir; gerçek sıvı akışı ölçümü değildir. STOP ve yerel donanım/kalibrasyon kilitleri korunur.

Bu bir **geliştirme taslağıdır**. Entegre v1.06 firmware, gerçek I²C LCD, PLC/RS485, AVR 8/16MHz ve fiziksel kabul NOT_RUN. Kullanıma hazır sürüm veya elektrik bağlantı talimatı değildir. Teknik bağlantılar ve nihai register tablosu en son hazırlanacak Engineering Manual'de yer alacaktır.
