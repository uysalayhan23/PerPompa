# PerPompa v1.05 — Kurulum Donanım Ayarları Tasarımı
**Kayıt:** 2026-10-31 | **Durum:** SPEC + profil HOST_CPP 125/125 ×2 + bağımsız runtime/EEPROM/LCD/START HOST_CPP 486/486 ×2 PASS; UI/aktif firmware entegrasyonu UYGULANMADI | **Bağlı:** HW-SETUP-01C, P0-02 ve endüstriyel release kapıları

## Sürücü tercihi — kullanıcı kararı (2026-10-31)

- **Önerilen ilk kurulum seçimi: A4988.** Kullanıcı genellikle bunu kullanır. Yeni kurulum menüsü sürücü satırında ilk olarak `Driver: A4988` gösterebilir; bu yalnız **öneridir**, aktif/teyitli donanım bilgisi değildir.
- **Alternatif: DRV8825.** Bazen kullanılır; menüde görünür kalır, aynı test ve kabul ciddiyetiyle desteklenir. Bu seçenek ikinci sırada listelenir; hiçbir işlev koddan çıkarılmaz.
- **Güvenli ilk çalıştırma:** A4988 gösteriliyor olsa bile kullanıcı kart + sürücü + 1/4 mikro-adım eşleşmesini *açıkça onaylamadan* EEPROM yapılandırması geçerli veya motor start izni verilmiş sayılmaz. Blank/bozuk EEPROM fail-closed kalır.
- **Sürücü değişimi:** A4988 → DRV8825 veya tersi ancak motor dururken ve Remote/USB başlangıç etkisizken yapılır. Önceki kurulum onayı geçersiz sayılır; fiziksel kablo bağlantısı, MS modları, akım limiti, STEP zamanlaması ve seçili profil tekrar doğrulanıp kaydedilir. Önceki ayarın sessizce yeni sürücüye uygulanması yasak.
- **Hesaplama:** Aynı fiziksel 1/4 mikro-adım ve aynı kalibrasyonla 800 STEP/devir korunur; A4988/DRV8825 adını değiştirince hacim veya debi formülü kendiliğinden değişmez. Fiziksel motor performansı ve minimum pulse/enable süreleri ayrıca ölçülmelidir.

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
- `Board: 5V/16MHz` veya `Board: 3V3/8MHz`; `Driver: A4988` (öncelikli öneri) veya `Driver: DRV8825` (alternatif). İlk öneri onay yerine geçmez.
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

### Yeni kaynak durumu — 2026-10-31

`hw-setup-profile.h` bağımsız, henüz aktif `.ino` içinden çağrılmayan doğrulayıcıdır; `tests/hw-setup-profile-native.cpp` g++/UBSan ile iki F_CPU profilinde 125'er pozitif-negatif kontrol PASS gösterdi. Eski EEPROM 0..21 korunur; 32/48 önerilen yeni slotlar **henüz firmware'de kullanılmıyor**. `INTEGRATION/AVR_FOR_HW_SETUP/HW_BENCH=NOT_RUN`. Kullanıcı açık onaylı fiziksel sürücü/board uyumu ve kart üzeri güvenlik testleri olmadan motorun emniyetli olduğu iddia edilmez.

## HW-SETUP-01B — bağımsız EEPROM/START/LCD adaptör kanıtı (2026-10-31)

**Baz:** `main 0e2ff55983912bda580722b98f8e41f53d245bfc`; P0-01 kabul edilmiş aktif `.ino` blob `6617cc38bde9bddf1c32cd0506915c0054556127` **korundu**; v1.04 arşivi, `AGENTS.md` ve `REVISION_MANIFEST.md` korunur. Önceki P0-01 32/32 full-loop, 42/42 seçilmiş kaynak, iki AVR derleme PASS ve HW profil modülü 125/125 × 2 PASS yeniden koşulmadı.

**Gerçek bağımsız C++11 kaynak/test dosyaları:** `hw-setup-runtime.h` (Git blob `574920f1228bc7db9237a816a3a6fab7dce85eb2`), `tests/hw-setup-runtime-native.cpp` (`0ba7d885e29c039eab00c695706a9c02284a73c7`), `tests/hw-setup-runtime-gate.cjs` (`b6cc4368f68713a33a2f36ee917320d4066b1aac`), `tests/hw-setup-runtime-avr-gate.cjs` (`b7c7c8a72a4ff927a76758657b56a682e9acdc71`), `HW_SETUP_01B_INTEGRATION_PLAN.md` (`d586ce758bf8629720ed31f48a25d501188f3d69`).

**Kapsam:** Eski EEPROM 0–21 dokunulmaz. Profiller 32–43 ve 48–59 sürümlü/CRC'li iki slotta, bağımsız motor izni baytı 64'te tutulacak şekilde bağımsız adaptör hazırlandı. Boş/hatalı/onaysız profilde izin yok. A4988 sadece öneri; DRV8825 alternatif; `F_CPU` 8/16 MHz ve 1/4 mikro-adım onay eşleşmesi gerekli. Kurulum yalnız motor/Remote/USB idle iken açılır; girişte izin kilidi yazılır; yarım EEPROM güncellemesi eski profili yeniden yetkilendirmeden başarısız olur; açık fiziksel onay, commit sonu ve geri okuma kontrolünden sonra izin verilir. Yerel/Remote/USB Dose/Pump/Cal için **dokuz ayrı START izni**, STOP reddi ve 16×2 LCD iki satır metni adaptörde modeller. Adaptör **henüz aktif .ino içinden çağrılmıyor**, dolayısıyla mevcut motora yeni güvenlik sağlamaz.

**Gerçek native test:** C++11 `g++ -Wall -Wextra -Werror -pedantic -fsanitize=undefined` ile `-DF_CPU=16000000UL` için **486/486 PASS** ve `-DF_CPU=8000000UL` için **486/486 PASS**. Ek Clang `8000000UL` **486/486 PASS**. EEPROM inhibit kontrolünü silen **geçici negatif mutant**, `HW_SETUP_RUNTIME_FAIL: power loss during setup locks previous profile` verip exit **1** ile yakalandı. Mutant GitHub'a gönderilmedi. QA aracı `HW_SETUP_RUNTIME_CPP_TWO_FCPU_PASS=2/2` döndürdü. İki kart profili burada yalnız host `F_CPU` makrolarıyla sınandı; Arduino Pro Mini binary derlemesi değildir.

**Açık kabul:** Bağımsız AVR probe için araç hazır ama **AVR_PROBE=NOT_RUN**; kabul edilmiş .ino içinde `setup()/loop()` entegrasyonu **NOT_RUN**, entegre 8/16 MHz Arduino derlemesi **NOT_RUN**, gerçek STEP/RNG/LCD/Remote/USB/HW **NOT_RUN**, P0-02 STOP latency **REVIEW**, endüstriyel `RELEASE_BLOCKED`. EEPROM tek bayt inhibit fiziksel kilit değildir; inhibit baytının kendisi yazılırken güç kaybının elektriksel etkileri, donanım brownout ve bağımsız güvenlik HIL'i ayrıca REVIEW.

**Sonraki tek iş:** HW-SETUP-01C — mevcut 11 menü kaydını ve 0–21 EEPROM adreslerini bozmadan yeni HW Setup sayfasının gerçek `setup()/loop()` motor START/STEP/STOP girişlerine bağlanan **geçici kaynak yamasını** üret; özgün tam loop pozitif-negatif, Remote/USB/Local/Cal, LCD/EEPROM ve iki gerçek AVR derleme kapıları PASS olursa ancak o zaman aktif firmware değişikliğini `[skip ci]` ile main'e taşı. Fiziksel ve release kilitlerini koru.
