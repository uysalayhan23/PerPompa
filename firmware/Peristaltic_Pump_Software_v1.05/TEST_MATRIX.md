# PerPompa v1.05 — Regresyon Test Matrisi

**Statü sözlüğü:** `SOURCE_CHECK` kaynak incelemesi; `MODEL_CHECK` kaynak davranışına dayalı model; `HOST_CPP` gerçek fonksiyon derleme/çalıştırma; `AVR_COMPILE` hedef Arduino derlemesi; `HW_BENCH` cihaz giriş/çıkış; `HW_PUMP` gerçek sıvı/motor doğrulaması. `NOT_RUN`, `REVIEW`, `FAIL` ve `PASS` birbirine dönüştürülmez.

**İlk temel:** v1.04 ve v1.05 `.ino` Git blob `0d965c08947ba0b3170d22fbff806fc2e6f1e83a` (31.10.2026). Önceki oturumda 32/32 kaynak desen denetimi ve model kontrolleri raporlandı; **AVR veya gerçek kart PASS değildir**. Ayrıntılı native test kanıtı henüz eklenmedi.

| ID | Pozitif durum | Negatif/sınır durum | Gereken kanıt | Başlangıç durumu |
|---|---|---|---|---|
| M01 Dose | Ayarlı hacim adım sayısı ve bitiş | sıfır cal, aşırı steps, negatif veya sıfır hacim | Gerçek hesap fonksiyonu + step çıkışı | HOST_CPP normal hesap PASS / zero_cal sanitiser FAIL gözlemi; tam Dose/step REVIEW |
| M02 Pump | Geçerli hızda kararlı periyot | sıfır hız, çok düşük rpm/mL/min, overflow | AVR 8/16 MHz timing, stop | HOST_CPP normal hesap PASS / zero_rate sanitizer FAIL gözlemi; gerçek AVR timing REVIEW |
| M03 Cal | 30 tur 24.000 mikro-adım hedefi | bitiş/reset, sapmış süre | Native + fiziksel ölçüm | REVIEW |
| M04 USB | p/d/c/w/x geçerli komut | boş, eksi, fazla, yarım, yabancı char, stop | Seri emülasyon + RNG | REVIEW |
| M05 Remote Hold | D5 LOW start / HIGH stop | bounce, ilk açılış LOW, menüde LOW | Tam döngü modeli + cihaz | REVIEW |
| M06 Remote Pulse | yeni LOW toggle | tekrarlı LOW, bounce, açılışta LOW | Tam döngü modeli + cihaz | REVIEW |
| M07 RNG | normal step aktif; stop/bitince idle | USB yanlış durum, hata sonrası idle | A0 dijital sinyal, PLC rölesi | REVIEW |
| M08 EEPROM | geçerli ayarlar kaydı/geri yükleme | boş ve kısmi kayıt, eski v1.03 bellek | Simülasyon + güç çevrimi | REVIEW |
| M09 DIR | CW/CCW, menü değişimi | kayıtlı CCW açılışı | D6 sinyal gözlemi | REVIEW |
| M10 Encoder | 1 tık/1 adım, buton | hızlı/ters/basılı adım, stop kuyruğu | Mock/native ve fiziksel encoder | REVIEW |
| M11 LCD | 16×2 değişen karakter önbelleği | taşma, yeniden çizim, uzun sayı | Mock/native ve fiziksel LCD | REVIEW |
| M12 Release | v1.04 arşivi değişmez, v1.05 bağımsız | yanlış klasör, arşiv yazma, HEAD yarışması | Blob SHA ve expected_sha | SOURCE_CHECK |

İlk test altyapısı [tests/reference-gate.cjs](tests/reference-gate.cjs) yalnız statik ve sayısal model kapısıdır. `node` çıktısındaki PASS, `AVR_COMPILE` veya `HW_PUMP` anlamına gelmez. Firmware düzeltme kabulü için ek gerçek kaynak/derleme testleri şarttır.

## Endüstriyel kabul uzantısı (2026-10-31)

G01–G10 ürün release kapıları [INDUSTRIAL_RELEASE_GATE.md](INDUSTRIAL_RELEASE_GATE.md) içinde takip edilir; bu test matrisi bunların yerine geçmez. [RISK_REGISTER.md](RISK_REGISTER.md) R-001–R-012 her test alanının neden gerekli olduğunu gösterir. HOST_CPP test hazırlığı [tests/native-math-harness.cjs](tests/native-math-harness.cjs) dosyasındadır: **2026-10-31 host izole hesap fonksiyonu denemesi ve koşu sonuçları HOST_CPP_EVIDENCE.md içindedir; AVR_COMPILE ve HW_* tümü NOT_RUN.** Bir test aracının varlığı PASS değildir.

## 2026-10-31 P0-01 test-first güncellemesi

- `HOST_CPP` özgün hesap fonksiyonu: 4 normal PASS; sıfır cal ve sıfır hız için 2 sanitizer hata reproduksiyonu; negatif hız ve 16 bit alt-dilimi sınırları ayrı REVIEW.
- İzole geçici **aday** C++: 4 normal + 16 negatif + 3098 geçerli değer paritesi PASS. Aday sonuçları özgün kaynak düzeltmesi veya motor çıkışı kabulü değildir. Detay [P0_01_CANDIDATE.md](P0_01_CANDIDATE.md).
- `AVR_COMPILE`, `FULL_FIRMWARE_NATIVE`, `HW_BENCH`, `HW_PUMP`: **NOT_RUN**. `P0-01` firmware değişikliği **BLOCKED**, üretim sürümü **RELEASE_BLOCKED**.

- `HOST_CPP` STEP/RNG mock: `dose(2,625)`, `pump(625)`, `dose(0,0)` pozitif PASS; `pump(0)`, `dose(1,0)` **istenmeyen STEP darbesi ve RNG aktif** yeniden üretildi; [tests/p0-01-step-baseline.cpp](tests/p0-01-step-baseline.cpp). Tam Arduino, gerçek zaman/donanım = NOT_RUN.

## 2026-10-31 — AVR nesne denemesi / güvenli çıkış adayı

`tests/avr-object-probe.cjs` gerçek .ino hesap fonksiyonlarını çıkarıp `clang++ --target=avr -mmcu=atmega328p` ile 8 MHz + 16 MHz nesneler üretmek için eklendi; yerel özgün hesap gövdesi denemesinde **2/2 AVR_OBJECT_PASS**, **2 warning per target** (`conv` geçersiz mode için başlatılmamış). **Tam Arduino/avr-gcc derlemesi değil.**
`tests/p0-01-output-candidate.cpp`: **18/18 HOST_CPP adaya ait çıktı regresyonu PASS**, no-STEP/no-RNG invalid girişlerde; ancak tam `loop()` test ve gerçek kart NOT_RUN.

## M13 — HW-01 Kurulum profili (2026-10-31)

- Pozitif: 5V/16MHz+A4988, 5V/16MHz+DRV8825, 3.3V/8MHz+A4988, 3.3V/8MHz+DRV8825; doğru Arduino hedef derlemesi ve cihazda güvenli başlangıç.
- Negatif: yanlış `F_CPU`, eski/boş/bozuk EEPROM, geçersiz sürücü, kaydedilmemiş kurulum, 1/4 dışı mikro-step, motor/Remote aktifken ayar değiştirme. STEP çıkışı ve RNG idle kriterleri.
- Host `tests/hw-profile-gate.cpp`: **19/19 PASS** (yalnız bağımsız konfigürasyon mantığı modelidir); gerçek LCD/encoder/EEPROM/AVR firmware ve donanım **NOT_RUN**. Kaynak: [HARDWARE_SETTINGS_SPEC.md](HARDWARE_SETTINGS_SPEC.md).

### HW-01 sürücü önceliği (2026-10-31)
- Sürücü menüsünde A4988 ilk öneri, DRV8825 ikinci seçenektir; ikisi de desteklenmeye devam eder.
- A4988 ön seçimi boş EEPROM/ilk kurulumda otomatik motor izni sayılmaz. DRV8825'e geçişte onay yeniden alınır.
- Bağımsız C++ modeline bu beş ilave pozitif/negatif test yazıldı. **Kaynakla aynı SHA değerine sahip host C++ sürümü 24/24 PASS (g++ -std=c++17, -Wall/-Wextra/-Werror/-pedantic, ASan+UBSan); bu Arduino firmware ya da donanım testi değildir**; dört fiziksel profil ve gerçek AVR/Arduino doğrulaması hâlâ NOT_RUN.

### P0-01 Arduino derleme kapısı — 2026-10-31

Yeni [tests/dual-arduino-build-gate.cjs](tests/dual-arduino-build-gate.cjs) iki gerçek hedefi zorunlu kontrol eder: \`arduino:avr:pro:cpu=16MHzatmega328\` ve \`arduino:avr:pro:cpu=8MHzatmega328\`. Arşiv SHA sabit, geçici build dizini, otomatik Actions veya firmware yükleme yok. Arduino CLI eksikse FAIL/NOT_RUN. **Bu oturumda Arduino CLI/avr-g++ yok; AVR_ARDUINO_BUILD=NOT_RUN, P0-01 firmware main commit yapılamaz.** Çalıştırılabilir test altyapısının bulunması, derlemenin geçtiği anlamına gelmez.


## 2026-10-31 — Kullanıcıdan gerçek Arduino başlangıç derlemeleri

Kullanıcının Windows PowerShell'inden alınan çıktı: Arduino CLI 1.5.2-rc.1; Arduino AVR Boards 1.8.8; TimerOne 1.1.1 ve LiquidCrystal/ClickEncoder kurulu ortam. **Değişmemiş v1.05** `.ino` için `arduino:avr:pro:cpu=16MHzatmega328` **PASS** (flash 15680/30720, RAM 740/2048) ve `arduino:avr:pro:cpu=8MHzatmega328` **PASS** (flash 15676/30720, RAM 740/2048). Bu iki PASS **yalnız baseline firmware derlemeleridir**; P0-01 güvenlik düzeltmesinin veya fiziksel ürünün PASS olduğu anlamına gelmez.

Testli üretim değişikliği yerine [tests/p0-01-candidate-dual-build.cjs](tests/p0-01-candidate-dual-build.cjs) eklendi: v1.04 ve aktif v1.05 baseline blob SHA'yı kontrol eder, yalnız sistem geçici klasöründe aday kaynak oluşturur; **değiştirilmiş adayın** 16MHz ve 8MHz Arduino derlemelerini dener ve temizler. Gerçek cihaz/ana dal firmware'ini değiştirmez. Host pozitif-negatif ve tam `loop`/USB/Remote testleri olmadan firmware commit yok; kullanıcı test sonucu bekleniyor. HW Setup, P0-01 güvenli kapandıktan sonra.

## 2026-10-31 — Kullanıcı tarafından gerçek P0-01 aday AVR derlemesi

**Kaynak GitHub main:** `49c4ccc8c946eb10531a181add1b6b4f8d50cf8d`; **aday üretici:** `tests/p0-01-candidate-dual-build.cjs` (git blob: `5c28eb71bacfcfef08c6436bb629d5f79ac7506a`); **aktif .ino baseline:** `0d965c08947ba0b3170d22fbff806fc2e6f1e83a`.

Kullanıcı Windows PowerShell + Arduino CLI + AVR kart/kütüphaneleriyle adayın **geçici klasörde** iki derlemesini gerçekten çalıştırdı. Aşağıdaki çıktılar kullanıcının doğrudan ilettiği derleme kanıtıdır:

| Kart/FQBN | Flash | SRAM | Derleme | Kanıt |
|---|---:|---:|---|---|
| `arduino:avr:pro:cpu=16MHzatmega328` | 16.108/30.720 bayt (%52) | 740/2048 (%36) | `CANDIDATE_AVR_COMPILE_PASS` | Kullanıcı CLI çıkışı |
| `arduino:avr:pro:cpu=8MHzatmega328` | 16.104/30.720 bayt (%52) | 740/2048 (%36) | `CANDIDATE_AVR_COMPILE_PASS` | Kullanıcı CLI çıkışı |

`P001_CANDIDATE_DUAL_BUILD_PASS=2/2`, süreç çıkış kodu `0`. **AVR_COMPILE(CANDIDATE)=PASS**, önceki baseline 2/2 PASS da korunur. Bu, çalışır durumda olan üretim firmware'ine aktarım yapıldığı veya motor/Remote/RNG/USB güvenlik regresyonlarının geçtiği anlamına gelmez. **P0-01 kaynak teslimi WAITING_FOR_FULL_LOOP_NEGATIVE_POSITIVE; HW_BENCH=NOT_RUN; HW_PUMP=NOT_RUN; RELEASE_BLOCKED.**

**Sonraki teknik iş:** Tam `loop()` normal ve geçersiz veri akışı, `in_action/usb_start` durumu, `STEP` pulse sayısı ve `RNG` idle, Cal 30 tur, stop/yön/Remote korumalarını öz kaynakla test et. Yalnız gerçek kaynak native pozitif-negatif ve ilgili derleme kapıları birlikte geçerse en küçük firmware yamasını `[skip ci]` ile ana dala göndermeyi değerlendir. A4988 esas, DRV8825 alternatif HW menüsü P0-01 kapanmadan uygulanmaz.

## 2026-10-31 — Seçilmiş gerçek kaynak dalı için ek HOST_CPP kapısı

[tests/p0-01-host-action-gate.cjs](tests/p0-01-host-action-gate.cjs) eklendi: mevcut P0-01 aday üreticisindeki dört gerçek C++ fonksiyonu ve `Start` / USB motor dispatch kodunu kaynak metinden **aynen ayıklayarak** `g++` ile mock GPIO üzerinde çalıştıracak şekilde tasarlandı. Pozitif-negatif vakalar: sıfır/hatalı kalibrasyon, aşırı step, düşük hız, Dose tamamlanma, Pump STEP/RNG idle, USB `p/d/c`, 30-tur Cal ilk adımı. Bu **bütün Arduino loop() yürütümü değildir** ve gerçek kart/sürücü testinin yerine geçmez. Bu ortamda yeni scriptin JavaScript syntax ve kaynak anchor/ayıklama uygunluğu kontrol edildi; **scriptin kaynak-ayıklanmış native koşusu henüz NOT_RUN**. Önceki elle oluşturulmuş C++ seçili-dal mock testinde **42/42 PASS**, ancak bu test özgün dosya yerine seçilmiş kod kopyaları kullanmıştır; o nedenle ayrı delil türüdür ve AVR/Full firmware güvenlik PASS sayılmaz.


## 2026-10-31 — P0-01 gerçek kaynak-ayıklanmış C++ çıktısı (native host)

**Girdi:** Canlı GitHub baseline v1.05 `.ino` blob `0d965c08947ba0b3170d22fbff806fc2e6f1e83a` ve `tests/p0-01-candidate-dual-build.cjs` içerisindeki dört kaynak fonksiyonu + üç çağıran-kod yaması. `tests/p0-01-host-action-gate.cjs` Start ve USB motor dispatch bölümlerini doğrudan yamalanmış kaynaktan ayıklayıp mock GPIO/RNG'li geçici C++ dosyası üretir.

**Çalıştırılan:** Seçilmiş kaynak fonksiyonları ve Start/USB dispatch kodundan üretilmiş **7.707 bayt** C++; `g++ -std=c++17 -O0 -Wall -Wextra -Werror -fsanitize=undefined,float-cast-overflow -fno-sanitize-recover=all`. **`SOURCE_EXTRACTED_BRANCH_HOST_CPP=42/42 PASS`**. Üretilen dosya ile host testine verilen gerçek içerik eşleşti (bayt sayısı 7707, FNV-1a32 `a4383df7`); ilk denemede eklenen yalnız bir satır başı normalleştirilerek birebir eşleşme sağlandı. Pozitif: normal mL/rot hesapları, Dose/Pump STEP, 30 tur Cal ilk adımı, USB p/d/c; negatif: sıfır/eksi girdi, aşırı adım, düşük hız, sıfır veya aralık dışı gecikmede STEP yok ve RNG idle.

**Sınır:** `SOURCE_EXTRACTED_BRANCH_HOST_CPP=PASS`, fakat kod **tüm `.ino`yu, Arduino kütüphanelerini, seri parse döngüsünü, EEPROM veya gerçek Remote stop/başlatma yollarını** yürütmez. Tam `loop/setup` host/native simülasyonu **NOT_RUN**, fiziksel `HW_BENCH` ve `HW_PUMP` **NOT_RUN**. User-supplied geçici P0-01 aday **AVR_COMPILE=2/2 PASS**, ancak bu birleşim endüstriyel sürüm kabulü değildir.

**Karar:** P0-01 adayının matematiksel ve seçilmiş çıkış dalları gözlenen testleri PASS. **Firmware main'e gönderilmedi:** ana döngü, stop gecikmesi ve güç/Remote/USB koşullarındaki tam negatif akışlar açık; ayrıca sürücünün gerçek A4988/DRV8825 zamanlaması doğrulanmadı. Aktif .ino ve v1.04 aynı SHA ile korunuyor. Bir sonraki kabul kapısı tam bütünleşik döngü + motor güvenliği (gerçek cihaz testine kadar REVIEW) ve bağımsız QA'dır.
