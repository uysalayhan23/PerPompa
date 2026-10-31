# PerPompa v1.05 — Kurulum Donanım Ayarları Tasarımı
**Kayıt:** 2026-10-31 | **Durum:** SPEC + HOST_CPP_MODEL_PASS; UI/firmware UYGULANMADI | **Bağlı:** P0-01 ve endüstriyel release kapıları

## Kullanıcının istediği 4 kombinasyon

| Arduino Pro Mini | Adım sürücüsü | Programlama profili | Saha doğrulaması |
|---|---|---|---|
| 5 V / 16 MHz | A4988 | `F_CPU=16000000` için derlenmiş firmware | NOT_RUN |
| 5 V / 16 MHz | DRV8825 | `F_CPU=16000000` için derlenmiş firmware | NOT_RUN |
| 3.3 V / 8 MHz | A4988 | `F_CPU=8000000` için derlenmiş firmware | NOT_RUN |
| 3.3 V / 8 MHz | DRV8825 | `F_CPU=8000000` için derlenmiş firmware | NOT_RUN |

**Fiziksel gerçek:** Yazılımdan seçilen "Board" parametresi kartın gerilimini ya da osilatör frekansını değiştirmez. Arduino IDE / derleme sırasında **doğru kart/işlemci** seçilip o kart için ayrı derlenmiş firmware yüklenmelidir. Menüdeki kayıt `F_CPU` ile karşılaştırılabilir ama dışarıdan bağımsız referans olmadan gerçek osilatörün/gerilimin doğru olduğunu *ölçemez*. A4988 / DRV8825 tipi de yalnız kullanıcının teyididir, otomatik algılama değildir.

## Hedef kullanıcı akışı (16×2 LCD)

- Var olan Dose/Pump/Cal ve ana menü dizisini değiştirmeden yeni `HW Setup` / `Kurulum` alt menüsü.
- `Board: 5V/16MHz` veya `Board: 3V3/8MHz`; `Driver: A4988` veya `Driver: DRV8825`.
- `Microstep: 1/4` şimdilik **bilgilendirme/sabit seçim**; geçerli `STEP_MODE=4`. Başka mikro-adım modu yapılacaksa hem kart/sürücü donanımı hem tüm dose/speed hesabı birlikte değiştirilip yeniden kalibre edilmelidir.
- `Save + Confirm`: motor duruyorken, Remote ve USB komutları etkisizken kurulumu kullanıcı onayıyla kalıcı kaydet. EEPROM boş, eski veya bozuksa "kurulum gerekli" ve güvenli çalışmama hali.
- Seçilen kart yazılımın `F_CPU` değerine uymuyorsa `Board mismatch` uyarısı; STEP yok, RNG idle. Seçilen sürücü/gerçek bağlantı eşleşmesi yazılımda sensör olmadığından doğrulanamaz; kullanıcı ve test fikstürü teyidi gerektirir.
- Kurulum sayfasına Pump/Dose/Cal sırasında girilemez. Ayar değişince veya save edilirken otomatik motor başlatma olmaz; Remote Hold güç açılış tehlikesi ayrı R-001 güvenlik kapısıdır.
- Arşivdeki v1.04 EEPROM dizilimi korunacak: yeni alanları eski slotlara yazma; ayrı sürümlü/doğrulamalı kayıt, güç kesintisinde bozuk değer güvenli reddedilsin.

## Hız ve hacim hesabı — önemli ayrım

- Dozajın dönüştürülmesinde **gerçek sürücü adından çok mikro-adım oranı + kalibrasyon** etkilidir. A4988 ve DRV8825, donanımda ikisi de 1/4 ayarındaysa mevcut 200 tam adım/devir × 4 = **800 STEP/devir** korunur.
- Birim hız için ideal STEP periyodu aynı kalır; **8 ve 16 MHz'de `delayMicroseconds` ve işlem overhead'i farklı** olabileceği için zamanlama gerçek derlemeye, osiloskop/logic analyzer ölçümüne ve sürücü özelliklerine göre ayrı doğrulanmalıdır. Menüden seçip hesap çarpanını 2 yapmak **yanlıştır**.
- A4988 veri sayfası asgari STEP HIGH/LOW 1 µs, DRV8825 1.9 µs. Ortak başlangıç tasarım hedefi en az 2 µs HIGH ve 2 µs LOW'dur; yazılım gecikme çağrısının gerçek darbe süresi kontrol edilmelidir. Sürücü seçiminde sınırlar yapılandırma verisi olarak kullanılabilir; motor yükü, ısınma, akım sınırlaması ve mekanik debi ek fiziksel test gerektirir.
- Çok düşük hızda eski 16-bit `int` gecikme ve `delayMicroseconds` sınırları değişmez. P0-01'i önce tamamla; güvenli düşük hız için gerekirse non-blocking timer tasarımı ayrıca yüksek riskli revizyon ve gerçek AVR deneyi gerektirir.

## Pozitif-negatif kabul

1. **Dört kart×driver kombinasyonu** kaydedilip doğru derleme profiliyle çalışmaya hazırlanır.
2. Çapraz F_CPU seçimi, bilinmeyen board/driver, yarım kalmış EEPROM, doğrulanmamış mikro-adım veya kurulum onaysızsa **motor çıkışları pasif**.
3. "Ayarlar" yalnız idle iken açılır; değişiklik anında/Remote aktifken STEP yok; RNG yanlış RUN bildirmez.
4. İki firmware derlemesi: Arduino Pro Mini ATmega328P 8 MHz ve 16 MHz; derlenen `F_CPU` ve EEPROM kayıt karşılaştırması.
5. 800 STEP/devir ve kalibrasyon sonuçları; Dose, Pump, Cal, USB, Remote, encoder, LCD regresyonları iki kart ve iki sürücüde gerçek HIL.
6. Gerçek donanımın 5 V/3.3 V çıkış düzeyi, sürücü giriş eşiği, motor güvenliği ve STEP min/max timing ölçülmeli.
7. Menünün, MCU saatini/gerilimini/sürücü akımını veya mikro-adım bağlantılarını fiziksel olarak ayarladığı iddia edilmeyecek.

## Kaynaklar
- Allegro A4988: https://www.allegromicro.com/-/media/files/datasheets/a4988-datasheet.pdf
- Texas Instruments DRV8825: https://www.ti.com/lit/ds/symlink/drv8825.pdf
- SparkFun Pro Mini 3.3 V/8 MHz ve 5 V/16 MHz: https://learn.sparkfun.com/tutorials/using-the-arduino-pro-mini-33v/introduction

## Görev ve teslim politikası
**Şu an yalnız tasarım/test modeli**; aktif .ino, v1.04 arşivi, kilitli manifest ve AGENTS.md değişmedi. P0-01'in kaynak+tam AVR derleme ve motor güvenlik kapıları sağlanmadan ayar menüsü firmware revizyonunu main'e alma. Endüstriyel RELEASE_BLOCKED korunur.
