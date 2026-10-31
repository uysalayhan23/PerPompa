# PerPompa v1.05 — NEXT_TASK / Çalışma devri

**Aktif manifest:** [REVISION_MANIFEST.md](REVISION_MANIFEST.md) v1.1 — kullanıcı onaylı tespit → onar → test → teslim → sıradaki iş döngüsü  
**Başlangıç main HEAD:** `8624b2b78b0bcf4f83dd3388b5533b46e7b7d667` (tarihsel; her zaman canlı HEAD oku)  
**Arşiv v1.04 kaynak blob kilidi:** `0d965c08947ba0b3170d22fbff806fc2e6f1e83a`  
**Aktif görev:** `P0-02 USB geçici adayda UART kuyruğu önce, 40ms tick sonra; 28/28 kaynak + 4/4 mutant + 13/13×2 native PASS, full-loop/AVR NOT_RUN; firmware HOLD`  
**Aşama:** P0-01 aktif yazılım kaynak 32/32 tam loop + 42/42 kaynak + 2/2 AVR PASS; fiziksel ve release BLOCKED. HW-SETUP-01 SPEC/TEST_FIRST.  
**Kod kabulü:** P0-01 SOURCE/HOST_CPP/AVR_COMPILE=PASS, yeni aktif blob `6617cc38bde9bddf1c32cd0506915c0054556127`; fiziksel HW_BENCH/HW_PUMP/USER_ACCEPTANCE NOT_RUN; RELEASE_BLOCKED.

## Endüstriyel ürün kontrol ekibi (2026-10-31)

**Takım:** [QUALITY_TEAM.md](QUALITY_TEAM.md) A01–A09 protokolü oluşturuldu; bu, bağımsız canlı 9 ajan veya otomatik arka plan görevi olduğu iddiası değildir.
**Endüstriyel serbest bırakma:** [INDUSTRIAL_RELEASE_GATE.md](INDUSTRIAL_RELEASE_GATE.md) G01–G10 = NOT_RUN; **RELEASE_BLOCKED**. [RISK_REGISTER.md](RISK_REGISTER.md) R-001–R-012 açık, risk sayıları kalibrasyon değil.
**P0-01 host C++ delili (2026-10-31):** [tests/native-math-harness.cjs](tests/native-math-harness.cjs) için Node sözdizimi doğrulaması ve yerel geçici çalışma kopyasıyla g++ 14.2.0 / UBSan denemesi yapıldı. **4/4 normal matematik fonksiyonu koşusu PASS**, kalibrasyon=0 ve hız=0 için **2/2 beklenen hata (float→long geçersiz dönüşüm) yeniden üretildi**; aşırı steps > AVR signed 32-bit üst sınırı için model REVIEW. Ayrıntı [HOST_CPP_EVIDENCE.md](HOST_CPP_EVIDENCE.md). Bu sadece özgün `steps_calc` ve `delay_us_calc` gövdelerinin host testidir: tam firmware/AVR veya motor testleri NOT_RUN.
**Kilit:** `AGENTS.md`, `REVISION_MANIFEST.md`, v1.04 arşivi ve aktif v1.05 .ino değiştirilmedi.

## Son P0-01 sonuç / açık kapı

Güncel kaynak `main` başlangıç HEAD `e6875170ae886e750bdaf70c84428e9484b5c3e5`. Geçici host C++ testinde orijinal fonksiyonlar için **4 pozitif hesap PASS**, **sıfır kalibrasyon/hız 2 sanitizer FAIL tekrar üretildi**; ayrıca negatif hız ve çok düşük hız uygunsuz hesapları görüldü. İzole **aday** için **4 pozitif, 16 negatif, 3098 geçerli parametre paritesi PASS (HOST_CPP geçici aday, AVR değil)**. Gerçek .ino hâlâ değişmedi. Hesapta 0 döndürmenin `pump/dose` içinde STEP'i durdurmadığı açık tasarım kapısı. Bu nedenle **P0-01 firmware çözümü COMPLETE DEĞİL**.

**P0-01 ek native test (2026-10-31):** [tests/p0-01-step-baseline.cpp](tests/p0-01-step-baseline.cpp) için host C++17 ASan/UBSan: Dose normal PASS, Pump normal PASS, Dose sıfır hacim no pulse PASS; zero-delay Pump ve Dose iki **beklenen kusur yeniden üretildi** (STEP+RNG). Tam `.ino`/AVR fiziksel PASS yok.\n\n**Sıradaki güvenli iş:** AVR derleme altyapısı + özgün kodun full-loop simülasyonunda `STEP=0`, `RNG=idle`, doğru stop durumu için test hazırlığı; ancak yeterli native/AVR kanıtı sonrası bir minimal yama ve ikinci bağımsız QA. Yeni oturum önce kaynak ve HEAD'i yeniden doğrulamalı.

## 2026-10-31 — Yeni test ve somut bulgular

- **HOST_CPP kaynak karşı-örnek yeniden doğrulandı:** 4/4 normal hesap, sıfır cal ve sıfır hız UBSan hatası; negatif hızın negatif gecikme üretmesi, küçük hızın 62500 µs istemesi; STEP/RNG uygunsuz sıfır delay iki durum yeniden üretildi. Yerel g++ 14.2.0; bu tam `.ino` test sonucu değildir.
- **AVR nesne üretimi**: Clang 17 AVR backend ile kaynak hesap fonksiyonları ATmega328P hedefinde 8 ve 16 MHz `ELF e_machine=83` nesne oluşturdu; her iki koşuda 2 *uninitialized conv* derleyici uyarısı. **AVR_OBJECT_PASS**, fakat **Arduino builder/AVR-libc/kütüphane bağlama NOT_RUN**. Tekrar edilebilir araç: [tests/avr-object-probe.cjs](tests/avr-object-probe.cjs).
- **Yalnız aday output host testi**: [tests/p0-01-output-candidate.cpp](tests/p0-01-output-candidate.cpp) ile normal Dose/Pump ve sıfır/negatif delay, yanlış RNG/STEP engelleme koşulları **18/18 HOST_CPP PASS** (mock GPIO). Aday üretim firmware'i değildir.
- Açık risk: Pump invalid geri dönüşü `in_action` / USB kalıcı çalışma durumunu otomatik temizlemez; `loop()` ve USB dahil tam entegrasyon kanıtı olmadan `.ino` commit yok.
- **Firmware/source SHA değişmedi**; arşiv kilitli. `AVR_ARDUINO_BUILD`, `FULL_LOOP_CPP`, `HW_BENCH`, `HW_PUMP` = NOT_RUN. `RELEASE_BLOCKED`.

## Yeni donanım kurulum talebi — HW-01 (kullanıcı, 2026-10-31)

**Tercih (kullanıcı, 2026-10-31):** A4988 ana/önerilen sürücü, DRV8825 ara sıra kullanılan alternatif. İlk gösterilen sürücü seçeneği A4988 olacak; kullanıcı fiziksel sürücüyü teyit edene kadar motor kilidi açılmayacak. Sürücü değiştirilince kurulum onayı yenilenmeli. Bu bir tasarım kararıdır, aktif firmware'e henüz uygulanmadı.\n\n**İstenen seçenekler:** Arduino Pro Mini **5 V/16 MHz** veya **3.3 V/8 MHz**; sürücü **A4988** veya **DRV8825** (kullanıcının "DRV88225" yazımı DRV8825 olarak yorumlandı). Kullanıcı kurulum sırasında menüden seçecek; bu seçim **fiziksel clock/voltajı değiştirmez**, eşleşen iki ayrı `F_CPU` derlemesi gerektirir. Sürücü/mikrostep otomatik algılanmaz.

- **Tasarım/tekrar edilebilir kaynak:** [HARDWARE_SETTINGS_SPEC.md](HARDWARE_SETTINGS_SPEC.md)
- **Host model:** [tests/hw-profile-gate.cpp](tests/hw-profile-gate.cpp); **19/19 HOST_CPP MODEL PASS** (g++17 ASan/UBSan). Bu gerçek firmware menüsü veya kart kabulü değildir.
- **Uygulama statüsü:** **DESIGN_PASS / FIRMWARE_NOT_STARTED / AVR_ARDUINO_BUILD=NOT_RUN / HW=NOT_RUN**. Tam menü kurulumu, EEPROM sürümlemesi, default/restart kilidi ve dört profil için testler zorunlu.
- **Öncelik:** P0-01 sayısal emniyet kapısına entegre tasarım bağımlılığı; P0-01 güvenle teslim edilmeden HW-01 nedeniyle aktif .ino'ya menü/refactor yapma. P0-01 ardından HW-01'i P1 kullanılabilirlikten önce ele al.
- **Kullanıcıdan ileride gereken fiziksel kontrol:** Kartın üzerindeki gerçek clock/gerilim işareti, kurulu sürücünün gerçekten A4988 veya DRV8825 olması, ve sürücünün mikro-adım modunun 1/4 olarak teyidi. Bu bilgiler fiziksel kabulün parçasıdır.

### P0-01 Arduino derleme kapısı — 2026-10-31

Yeni [tests/dual-arduino-build-gate.cjs](tests/dual-arduino-build-gate.cjs) iki gerçek hedefi zorunlu kontrol eder: \`arduino:avr:pro:cpu=16MHzatmega328\` ve \`arduino:avr:pro:cpu=8MHzatmega328\`. Arşiv SHA sabit, geçici build dizini, otomatik Actions veya firmware yükleme yok. Arduino CLI eksikse FAIL/NOT_RUN. **Bu oturumda Arduino CLI/avr-g++ yok; AVR_ARDUINO_BUILD=NOT_RUN, P0-01 firmware main commit yapılamaz.** Çalıştırılabilir test altyapısının bulunması, derlemenin geçtiği anlamına gelmez.

## P0-01 kapsamı

1. İlgili gerçek kaynak: `steps_calc`, `delay_us_calc`, `dose`, `pump`, çağıran `loop/setup`.
2. Pozitif veri: geçerli kalibrasyon ve hacimde adım sayısı; 30 dönüş kalibrasyonu; normal hızdaki yarım periyot; Dose/Pump/Cal, Remote ve RNG gerilememesi.
3. Negatif veri: kalibrasyon=0, hız=0, negatif USB değeri, aşırı büyük hacim, düşük hız (16-bit tür dönüşümü ve AVR `delayMicroseconds` sınırı), pozitif maksimum giriş.
4. Her bulguya `SOURCE_CHECK`, `MODEL_CHECK`, `HOST_CPP`, `AVR_COMPILE`, `HW_BENCH` ayrımı koy. Tahmini sayılar motor ölçümü değildir.
5. En küçük aday düzeltmenin geçerli davranış üzerinde etkisini ölç. Negatif test ve gerçek kaynak pozitif testleri geçmeden `.ino` ana dala aktarma.
6. Test iskeleti: `tests/reference-gate.cjs`; komut `node firmware/Peristaltic_Pump_Software_v1.05/tests/reference-gate.cjs`. Bu statik/model kapısıdır, native AVR kabulü değildir.

## Başlangıçta bilinen REVIEW konuları

- Geçersiz/boş EEPROM Remote değeri Hold'a dönüşebilir; ayrıca açılışta kapalı Hold kontağı start verir.
- Kaydedilmiş CCW yönü başlangıçta D6'ya yazılmıyor.
- USB aktif modunun son `inChar` değerine bağlı olması, beklenmeyen karakter ve eksik veri.
- RNG bir adım komutu göstergesidir, motor dönüşü/akış doğrulaması değildir.

## Değiştirilmemesi gerekenler

v1.04 ve diğer arşivler, pinler, referans davranışlar, encoder/LCD iyileştirmeleri, mevcut kullanıcı kılavuzu ve teknik çizimler. Her düzeltmede yalnız kanıtlı hedef.

## Yeni sohbet için kısa komut

"PerPompa v1.05 main'de `AGENTS.md`, v1.05 `REVISION_MANIFEST.md`, `NEXT_TASK.md`, `TEST_MATRIX.md` ve kaynak kodu güncel HEAD üzerinden oku; v1.04 SHA kilidini doğrula; P0-01 test-first çalışmasına devam et; pozitif-negatif native kanıt yoksa firmware commit yapma; [skip ci] ve uzak HEAD/blob SHA kuralları geçerli."

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

## 2026-10-31 — Tam setup/loop host C++ entegrasyon test aracı

`tests/p0-01-full-loop-host.cjs` dosyası üretildi. Gerçek v1.05 kaynak gövdesi ve mevcut P0-01 aday yaması üzerinden **tam setup(), loop() ve diğer firmware fonksiyonlarını**, GPIO/LCD/encoder/EEPROM/Remote/Serial sahte çevrebirimleriyle `g++` altında çalıştırmayı hedefler. Normal Dose/Pump/Cal, 24.000 Cal adımı, Remote Hold/Pulse debounce, USB p/d/c/x, sıfır/eksi hız, RNG idle, yerel stop senaryoları içerir. **R-001 güç açılışı Hold LOW otomatik motor başlatma ve R-007 bilinmeyen USB karakteri sonrası sahte RUN durumu ayrı açık risk olarak gözlenir; güvenlik PASS değildir.**

**Durum:** Kaynağa dayalı üreticinin JavaScript sözdizimi PASS, 4 fonksiyon ve 3 aday yaması ayrıştırılabilir; **üretilen tam C++ dosyasının gerçek native derlemesi/çalıştırması henüz NOT_RUN**. Test aracının varlığını PASS sayma. P0-01 aday için Windows Arduino 2/2 AVR derleme PASS korunur. Üretim firmware .ino değişmedi ve endüstriyel RELEASE_BLOCKED.


## Risk önceliği devri — P0-01 tam döngü aracı sonrası (2026-10-31)

Özgün `setup()`, geçersiz EEPROM Remote alanını 0 (Hold) yapıyor; Hold ve D5 LOW'da açılışta `remote_start()` çağrısı var: **R-001 SOURCE_CONFIRMED / HW NOT_RUN**. USB aktif modda yabancı `inChar` işleme alınmıyor, `usb_start` korunuyor ve `RNG` idle yapılmıyor: **R-007 SOURCE_CONFIRMED / FULL_LOOP_CPP NOT_RUN**. Yeni [tests/p0-01-full-loop-host.cjs](tests/p0-01-full-loop-host.cjs), bu durumları ayrı açık risk olarak test etmek için hazırlandı; yalnız kaynak+JS yapı kontrolü yapıldı, gerçek C++ tam döngü çıktısı **henüz NOT_RUN**. P0-01'in geçerli matematik davranışı ve aday iki AVR derlemesi PASS kanıtı korunur. Çalışan `.ino` yaması main'e aktarılmadı; endüstriyel kabul BLOCKED. R-001 bağımsız emniyet değerlendirmesi olmadan sessizce Remote Hold davranışı değiştirilmesin.

## 2026-10-31 — Gerçek tam setup/loop C++ simülasyonu

**Kaynak/girdi:** Baseline v1.05 `.ino` Git blob `0d965c08947ba0b3170d22fbff806fc2e6f1e83a`, `tests/p0-01-candidate-dual-build.cjs` ile üretilen **geçici P0-01 adayı**, tam `setup()`, `loop()` ve kalan .ino fonksiyonları; fiziksel cihaz yerine C++ LCD/encoder/EEPROM/Serial/Remote/GPIO sahte çevrebirimleri.

**Yerel koşu, g++ C++17 / UBSan + float-cast-overflow:** İlk tam kaynak testinde `#include` satırlarını temizleyen **test aracı** yanlışlıkla satır sonu yorum parçalarını bıraktığından C++ derlemesi başarısızdı; `#include` satırını tümüyle kaldıracak hedefli test aracı onarımı uygulandı. **Derlenip çalıştırılan tam kaynak C++ simülasyonunda `FULL_SETUP_LOOP_HOST_CPP=22/22 PASS`** ve **iki önceden bilinen risk `KNOWN_RISK_REPRODUCTIONS=2/2` olarak yeniden üretildi**. Pozitif/negatif koşular: Dose/Pump/Cal (Cal 24.000 mikro-adım), yerel STOP, Remote Hold/Pulse debounce/STOP, USB p/d/c/x, sıfır/eksi giriş, RNG/STEP yanlış-pozitif yokluğu. İlk sürümde 24/24 görünen sayı, **22 davranış PASS + iki kusur yeniden üretimi** olarak dürüst biçimde ayrıldı. Kaynak C++ programının doğrudan host derlemesi yapıldı; fiziksel AVR üzerinde yürütüm değildir.

**Açık kaynak kusurları:** R-001 Hold + LOW güç açılışında istemsiz çalıştırma; R-007 USB etkin adımlama sonrası tanınmayan byte durumunda `usb_start` ve RNG açık kalması. İki kusur da otomatik iyileşme veya PASS değildir. Bu nedenle `P0-01=BLOCKED`; `FULL_SETUP_LOOP_HOST_CPP` seçili kontroller PASS ama bütün endüstriyel güvenlik kabulü **FAIL/REVIEW**, `HW_BENCH/HW_PUMP=NOT_RUN`, `RELEASE_BLOCKED`. Önceki `AVR_COMPILE(CANDIDATE)=2/2 PASS` ve `SOURCE_EXTRACTED_BRANCH_HOST_CPP=42/42 PASS` korunur; **üretim .ino'ya yama gönderilmedi**.

**Sıradaki iş:** R-001 açılış emniyeti ve R-007 USB / RUN durum makinesi için manifest önceliğinde kaynak-kanıtlı minimal onarım ve tam USB/Remote negatif regresyon; ayrıca gerçek stop gecikmesi/ürün kabulü ayrıdır. Onarımın kendi gerçek AVR derlemesi ve kaynak regresyonu olmadan P0-01'i COMPLETE ilan etme veya HW menüsüne geçme.

## P0-01 / R-001 + R-007 güvenli geçici aday (2026-10-31)

`tests/p0-01-r001-r007-candidate.cjs` dosyası P0-01’in önceki adayını **değiştirmeden** kullanır; altı eşleşme-kilitli, fail-closed kaynak düzeltmesini yalnız geçici `.ino` kopyasına uygular: EEPROM Remote geçersizse Pulse, Hold+LOW açılışında otomatik çalıştırma yok, debounced HIGH ile arm edilmeden LOW başlatmıyor, USB x/unknown sırasında STEP LOW + RNG idle + `usb_start=false`. Geçerli Hold/Pulse EEPROM modları korunur; fakat **Hold LOW güç açılışında otomatik start davranışı güvenlik için tasarımsal olarak değişir**; profesyonel fiziksel kabul/bağımsız emniyet değerlendirmesi gerekir.

`tests/p0-01-full-loop-host.cjs` eski kapsamını değiştirmeyen **opt-in `P001_RISK_CANDIDATE=1`** moduna genişletildi. Bu mod yeni adayın tam setup/loop C++ mock testlerini dener ve önceki 2 bilinen kusur tekrarını artık güvenli davranış beklentisine dönüştürür. **Geliştirme ortamında bu yeni adayın gerçek bütünleşik C++ koşusu ve iki AVR derlemesi NOT_RUN.** Yerel izole R-001/R-007 15/15 host C++ model PASS (g++14.2.0, UBSan); bu gerçek .ino simülasyon PASS anlamına gelmez. Önceki P0-01 aday AVR 2/2 ve selected C++ 42/42 ve full-loop 22/22 + 2 kusur reproduksiyonu kayıtları değiştirilmedi. **Firmware main commit yok, P0-01 BLOCKED, RELEASE_BLOCKED**.

Sıradaki gerekli kapı: risk adayı için 8/16 MHz gerçek Arduino derleme PASS, `P001_RISK_CANDIDATE=1` ile tam C++ setup/loop pozitif-negatif PASS, USB parse ve Remote debounce derin incelemesi. Hardware/stop latency ayrıca NOT_RUN; A4988 varsayılan + DRV8825 alternatif menü P0-01 kapatılmadan uygulanmayacak.

### 2026-10-31 — R-007 USB'den yerel çıkış güvenlik iyileştirmesi

Kaynak incelemesi: `exit_action_menu()` RNG'yi temizlerken `usb_start` değişkenini temizlemiyor, USB kontrol menüsüne yeniden girildiğinde eski RUN durumu korunabilir. R-001/R-007 **geçici adayına 7. doğrulanabilir yama noktası** eklendi: çıkışta `usb_start=false`, STEP LOW ve RNG idle. Mevcut tam döngü testinin risk modu, normal USB çalışmasından sonra encoder ile çıkış / yeniden giriş için üç yeni pozitif-negatif kontrol içerir. `main` firmware değişmedi; adayın tam C++ ve yeni iki AVR derlemeleri **NOT_RUN**. Isolated host 15/15 önceki test korunuyor, ancak yeni çıkış yolu o 15 kontrol kapsamında değildir. P0-01 **BLOCKED**, release **BLOCKED**.

### R-001/R-007 dar kapsamlı host kanıt güncellemesi (2026-10-31)

Gerçek kaynakta tanımlı Remote başlatma/durdurma koşullarına karşılık gelen **izole C++ host test modeli** g++ 14.2.0, C++17, `-Wall -Wextra -Werror -pedantic -fsanitize=undefined,float-cast-overflow` ile tekrar çalıştırıldı: **17/17 PASS**. Kapsam: EEPROM geçersiz/Pulse/Hold seçimleri, boot LOW fail-closed, debounce, Hold/Pulse başlangıç ve durma, yabancı USB karakterinde RUN/RNG/STEP LOW, USB menüsünden yerel çıkış ve yeniden giriş. Yerel test kaynağı SHA-256: `1cfd7c79ed09f048abfe161349d99e6e1ebfd28d77276bacc2da2c56b0a1cca9`. Bu bir **elle kurulmuş izole kaynak davranış modeli** olup gerçek tam `.ino` veya Arduino binary testi **değildir**. P0-01 seçilmiş kod **42/42**, önceki tam loop **22/22 + 2 risk reproduksiyonu**, önceki aday AVR **2/2** geçmiş delilini değiştirmez. **Yeni 7-anchor R001/R007 adayı AVR_COMPILE ve FULL_LOOP_CPP = NOT_RUN; firmware teslim BLOCKED, HW/RELEASE BLOCKED.**


## 2026-10-31 — P0-01 R001/R007 tek seferde çift derleme + tam kaynak host QA aracı

Yeni `tests/p0-01-one-shot-qa.cjs` araç dosyası, mevcut v1.05 aktif `.ino` ve değişmez v1.04 blob `0d965c08947ba0b3170d22fbff806fc2e6f1e83a` kontrolünü yapar; mevcut Windows Arduino CLI kurulumunu PATH veya LOCALAPPDATA/ArduinoCLI altından bulur. Daha sonra ayrı ayrı **gerçek g++ tam setup/loop host** (`P001_RISK_CANDIDATE=1`), önceki 42/42 kaynak-ayıklanmış host ve **yeni 7-anchor risk adayının 16/8 MHz Arduino CLI gerçek derlemeleri** için marker ve exit-code denetimi uygular. Bir kapı FAIL/NOT_RUN ise fail-closed BLOCKED döndürür. Firmware'i veya kartı değiştirmez, GitHub Actions çalıştırmaz. `g++` gerekli; Windows'ta yoksa host QA NOT_RUN/FAIL bildirilir. Başarılı yazılım QA bile G01–G10 fiziksel serbest bırakma yerine geçmez.

**Bu oturumda gerçek `tests/p0-01-one-shot-qa.cjs` Node sözdizimi PASS ve eksik kaynak dosyası üzerinde fail-closed çıkış gözlemi PASS.** İzole R-001/R-007 kaynak-davranış C++ modeli g++/UBSan ile **17/17 PASS** yeniden üretildi (önceki 17/17 korundu). Ancak bu konteynerde güncel GitHub kaynak dosyaları derleyiciye yerel olarak taşınamadı ve AVR CLI/AVR core kurulu değil; **yeni aday FULL_SETUP_LOOP_CPP=NOT_RUN ve AVR_COMPILE=NOT_RUN**. Önceki 42/42, 22/22+2 risk reproduksiyonu, eski aday 2/2 AVR kanıtları korunuyor. **P0-01 firmware=BLOCKED, HW_SETUP=NOT_STARTED, HW_BENCH/HW_PUMP=NOT_RUN, RELEASE_BLOCKED.**

Tek komut (repo ZIP kökünde, Node ve native g++ ve Arduino CLI kurulu ise):
`node firmware/Peristaltic_Pump_Software_v1.05/tests/p0-01-one-shot-qa.cjs`.
Çıktı `P001_R001_R007_SOFTWARE_QA=PASS` ve üç alt kapı PASS vermeden `.ino` değişikliği main'e aktarılmaz.


## 2026-10-31 — P0-01 güvenli durum yazılım teslimi (gerçek kullanıcı QA)

**Task ID:** P0-01 (aritmetik/STEP korumaları) + açık izinli R-001 Remote/EEPROM startup ve R-007 USB/STOP güvenliği. **Base main HEAD:** `54e92626ab22219712331a023109a0f176b6681b`; eski .ino blob `0d965c08947ba0b3170d22fbff806fc2e6f1e83a`; **teslim edilen yeni .ino blob:** `6617cc38bde9bddf1c32cd0506915c0054556127`; değişmez v1.04 aynı eski SHA. Commit SHA için güncel `main` kaydını doğrula.

**Negatif kök kusur:** kalibrasyon/hız 0, aşırı adım veya AVR int/delay sınırında hatalı STEP/RNG; boş EEPROM Remote slotu Hold'a iner ve D5 LOW'da boot start; USB bilinmeyen bayt veya yerel menü çıkışı eski RUN/RNG göstergesini bırakabilir.

**Kaynakta en küçük değişiklik:** dört aday hesap/motor fonksiyonu, üç motor çağıran yol kapısı ve yedi Remote/EEPROM/USB stop yaması, mevcut `tests/p0-01-r001-r007-candidate.cjs` ile SHA-kilitli üretilen içeriğin birebir teslimi. Pinler STEP D7/DIR D6, Remote D5, RNG A0; 200 tam adım × 1/4 = 800 STEP/devir ve 30 tur Cal korunur. **Hold LOW boot otomatik başlangıç güvenlik gerekçesiyle bilinçli olarak kaldırıldı; debounced HIGH ardından yeni LOW gerekir.**

**Gerçek Windows 31.10.2026 pozitif-negatif regresyon:** Clang64 host C++ `R001_R007_FULL_LOOP_CANDIDATE_HOST_CPP=32/32 PASS`; önceki `SOURCE_EXTRACTED_BRANCH_HOST_CPP=42/42 PASS`; Arduino CLI `16MHzatmega328` 16154/30720 flash, 741/2048 SRAM **PASS**; `8MHzatmega328` 16150/30720 flash, 741/2048 SRAM **PASS**. `P001_R001_R007_SOFTWARE_QA=PASS`, exit 0. Bu veriler kullanıcının Windows derleyici çıktısından alınmıştır. İlk Clang UBSan probu PowerShell argüman hatasıyla başarısızdı, PASS sayılmıyor; sonraki gerçek 32/32 host koşusu başarılıdır.

**Kod yazılım kabulü:** `SOURCE_CHECK/HOST_CPP/AVR_COMPILE=PASS`. Fiziksel kartta durma gecikmesi, akım/STEP zamanlaması, sıvı hassasiyeti, USB seri `w`/kısmi bozuk paketler, kayıtlı CCW açılışı ve G01-G10 endüstriyel kapılar **REVIEW/NOT_RUN**. `HW_BENCH/HW_PUMP/USER_ACCEPTANCE=NOT_RUN`, `RELEASE_BLOCKED`. Genel ürün veya donanım COMPLETE değil.

**Yeni QA akışı:** kabul edilen aktif `.ino` blobunda `tests/p0-01-one-shot-qa.cjs`, `tests/p0-01-accepted-source-qa.cjs` dosyasına yönlenir; doğrudan aktif kaynak için gerçek setup/loop 32, tarihsel v1.04+P0-01 adayı için 42 ve doğrudan aktif kaynak için Arduino 16/8 MHz derlemesi zorunludur. Eski baseline modu korunur. Yeni test yolunun JS syntax ve sahte başarı/FAIL enjekte edilen fail-closed kontrolü PASS; **post-merge gerçek g++/Arduino tekrar koşusu NOT_RUN** (önceki gerçek *aynı blob aday* Windows koşuları PASS).

**Sonraki tek aktif görev:** kullanıcı talebiyle A4988 ilk öneri, DRV8825 alternatif `HW Setup` için `HARDWARE_SETTINGS_SPEC.md` bazlı EEPROM sürümleme, konfigürasyon onayı ve `F_CPU` uyuşma güvenlik kapısını test-öncelikli tasarla. Menüyü gerekli ayrı native+AVR kapıları olmadan aktive etme. R0 P0-02 stop latency değerlendirmesi ve P0-03 USB `w`/bozuk paket analizi öncelikli REVIEW olarak kalır.


## 2026-10-31 — HW-SETUP-01: doğrulanmış *bağımsız* kurulum profili bileşeni

**Base main:** `6fab83a24e2cb53841361c37f28c26b72be62131`; **kilitli kabul edilmiş P0-01 .ino blob:** `6617cc38bde9bddf1c32cd0506915c0054556127` (dokunulmadı). Kilitli v1.04, AGENTS ve manifesto değişmedi.

**Gerçek eklenen kaynak:** [hw-setup-profile.h](hw-setup-profile.h) — doğrudan Arduino uyumlu C++11 header, *aktif v1.05 .ino tarafından henüz include/çağrılmıyor*. A4988 yalnız ilk öneri, bilinmeyen board/onaysız sürücü motor izni değil; DRV8825 alternatif; geçerli 16MHz/8MHz profile ve doğru F_CPU olmadan motor izin mantığı false; 1/4 mikro-adım sabit; motor/Remote/USB aktifken kurulum onayı yok. Board/driver değişimi eski onayı geçersiz kılar; sürücü tipini ve gerçek saat/gerilimi otomatik algılamaz.

**EEPROM:** V1.04 eski 0–21 adresleri değiştirilmedi. Yalnız veri modeli olarak 32 ve 48 adreslerinde iki 12 baytlık sürümlü slot; CRC-8 + tamamlayıcı bayt + son-yazılacak commit marker; enerji kesintisi sırasında yarım kayıt reddedilir, önceki sağlam kayıt geri yüklenir; iki geçerli slotta seq modulo 256, eşit nesil/çelişkili içerik ve 128 uzaklığı fail-closed. **EEPROM gerçek write/read firmware entegrasyonu NOT_RUN**; geliştirici açıkça `encodePending` çıktısının commit baytını en son yazmalı. Önceki sürücü fiziken değiştirilirse eski kayıt hâlâ geçerli görünebileceğinden yeni donanımın gerçek teyidi ve fiziksel çalışma engeli ayrıca zorunludur.

**Yeni pozitif-negatif native QA:** [tests/hw-setup-profile-native.cpp](tests/hw-setup-profile-native.cpp), doğrudan bu gerçek header'ı include eder; [tests/hw-setup-native-gate.cjs](tests/hw-setup-native-gate.cjs) derler/çalıştırır. Yerel g++ 14.2.0 C++11 `-Wall -Wextra -Werror -pedantic -fsanitize=undefined`:
- `HW_SETUP_NATIVE_FCPU_16000000=125/125 PASS`
- `HW_SETUP_NATIVE_FCPU_8000000=125/125 PASS`
- `HW_SETUP_NATIVE_TWO_CPU_PASS=2/2`
- Boş/bozuk/CRC'si yeniden hesaplanmış geçersiz EEPROM, eksik kullanıcı onayı, F_CPU uyuşmazlığı, sürücü değişimi, ayar sırasında motor/Remote/USB aktifliği, power-loss partial writes, sürüm hatası, nesil wrap/ambiguous/replay ve eski onaylı kaydın üstündeki yeni onaysız kayıt pozitif-negatif kontrolleri dahil.
- Negatif test duyarlılığı: güvenlik CPU eşleşmesi kasıtlı atlanan **yalnız geçici mutant** C++11 başarıyla derlendi, kontrol `HW_SETUP_HOST_FAIL unknown CPU always blocked` ile exit 1 döndürdü. Mutant GitHub'a aktarılmadı.

**Gerçek kabul sınırı:** `HW_SETUP_PROFILE_HEADER_HOST_CPP=125/125 × 2 PASS`, `FW_INTEGRATION=NOT_RUN`, `NEW_AVR_COMPILE=NOT_RUN`, `EEPROM_HARDWARE=NOT_RUN`, `HW_BENCH/HW_PUMP=NOT_RUN`, `USER_ACCEPTANCE=NOT_RUN`, endüstriyel `RELEASE_BLOCKED`. 125 testin hiçbiri aktif v1.05 .ino üzerinde kurulum menüsünün çalıştığını göstermez; çalışan P0-01 .ino değiştirilmedi. P0-01 32/32 + 42/42 + AVR 2/2 önceki PASS kanıtları yeniden koşulmadı.

**Sıradaki adım:** HW-SETUP-01B — profil başlatma/EEPROM güvenli-yazma adaptörü, motor START noktalarında merkezi yetki kapısı ve 16×2 LCD kurulum akışı; yalnız kaynaklı gerçek setup/loop pozitif-negatif + 8/16MHz Arduino gerçek derlemesi PASS olduktan sonra bu entegrasyonu üretim .ino'ya ekle. P0-02 stop latansı ve tüm G01–G10/HW RELEASE REVIEW/HOLD kilitlerini koru.

## HW-SETUP-01B — bağımsız EEPROM/START/LCD adaptör kanıtı (2026-10-31)

**Baz:** `main 0e2ff55983912bda580722b98f8e41f53d245bfc`; P0-01 kabul edilmiş aktif `.ino` blob `6617cc38bde9bddf1c32cd0506915c0054556127` **korundu**; v1.04 arşivi, `AGENTS.md` ve `REVISION_MANIFEST.md` korunur. Önceki P0-01 32/32 full-loop, 42/42 seçilmiş kaynak, iki AVR derleme PASS ve HW profil modülü 125/125 × 2 PASS yeniden koşulmadı.

**Gerçek bağımsız C++11 kaynak/test dosyaları:** `hw-setup-runtime.h` (Git blob `574920f1228bc7db9237a816a3a6fab7dce85eb2`), `tests/hw-setup-runtime-native.cpp` (`0ba7d885e29c039eab00c695706a9c02284a73c7`), `tests/hw-setup-runtime-gate.cjs` (`b6cc4368f68713a33a2f36ee917320d4066b1aac`), `tests/hw-setup-runtime-avr-gate.cjs` (`b7c7c8a72a4ff927a76758657b56a682e9acdc71`), `HW_SETUP_01B_INTEGRATION_PLAN.md` (`d586ce758bf8629720ed31f48a25d501188f3d69`).

**Kapsam:** Eski EEPROM 0–21 dokunulmaz. Profiller 32–43 ve 48–59 sürümlü/CRC'li iki slotta, bağımsız motor izni baytı 64'te tutulacak şekilde bağımsız adaptör hazırlandı. Boş/hatalı/onaysız profilde izin yok. A4988 sadece öneri; DRV8825 alternatif; `F_CPU` 8/16 MHz ve 1/4 mikro-adım onay eşleşmesi gerekli. Kurulum yalnız motor/Remote/USB idle iken açılır; girişte izin kilidi yazılır; yarım EEPROM güncellemesi eski profili yeniden yetkilendirmeden başarısız olur; açık fiziksel onay, commit sonu ve geri okuma kontrolünden sonra izin verilir. Yerel/Remote/USB Dose/Pump/Cal için **dokuz ayrı START izni**, STOP reddi ve 16×2 LCD iki satır metni adaptörde modeller. Adaptör **henüz aktif .ino içinden çağrılmıyor**, dolayısıyla mevcut motora yeni güvenlik sağlamaz.

**Gerçek native test:** C++11 `g++ -Wall -Wextra -Werror -pedantic -fsanitize=undefined` ile `-DF_CPU=16000000UL` için **486/486 PASS** ve `-DF_CPU=8000000UL` için **486/486 PASS**. Ek Clang `8000000UL` **486/486 PASS**. EEPROM inhibit kontrolünü silen **geçici negatif mutant**, `HW_SETUP_RUNTIME_FAIL: power loss during setup locks previous profile` verip exit **1** ile yakalandı. Mutant GitHub'a gönderilmedi. QA aracı `HW_SETUP_RUNTIME_CPP_TWO_FCPU_PASS=2/2` döndürdü. İki kart profili burada yalnız host `F_CPU` makrolarıyla sınandı; Arduino Pro Mini binary derlemesi değildir.

**Açık kabul:** Bağımsız AVR probe için araç hazır ama **AVR_PROBE=NOT_RUN**; kabul edilmiş .ino içinde `setup()/loop()` entegrasyonu **NOT_RUN**, entegre 8/16 MHz Arduino derlemesi **NOT_RUN**, gerçek STEP/RNG/LCD/Remote/USB/HW **NOT_RUN**, P0-02 STOP latency **REVIEW**, endüstriyel `RELEASE_BLOCKED`. EEPROM tek bayt inhibit fiziksel kilit değildir; inhibit baytının kendisi yazılırken güç kaybının elektriksel etkileri, donanım brownout ve bağımsız güvenlik HIL'i ayrıca REVIEW.

**Sonraki tek iş:** HW-SETUP-01C — mevcut 11 menü kaydını ve 0–21 EEPROM adreslerini bozmadan yeni HW Setup sayfasının gerçek `setup()/loop()` motor START/STEP/STOP girişlerine bağlanan **geçici kaynak yamasını** üret; özgün tam loop pozitif-negatif, Remote/USB/Local/Cal, LCD/EEPROM ve iki gerçek AVR derleme kapıları PASS olursa ancak o zaman aktif firmware değişikliğini `[skip ci]` ile main'e taşı. Fiziksel ve release kilitlerini koru.

## HW-SETUP-01C — geçici gerçek .ino adayı + fail-closed yazılım QA geçidi (2026-10-31)

**Canlı temel:** `main bf3bf14982008dbf894303af7ed8054c05502d4e`; P0-01 kabul edilmiş v1.05 `.ino` **Git blob `6617cc38bde9bddf1c32cd0506915c0054556127`**. V1.04 arşivi, `REVISION_MANIFEST.md`, `AGENTS.md` değişmedi. Önceki P0-01 **32/32 full-loop**, **42/42 seçilmiş kaynak** ve **2/2 AVR**; profil **125/125 ×2**, bağımsız runtime **486/486 ×2** geçmiş PASS kanıtları yeniden koşulmadı, yeni adayın PASS kanıtı olarak aktarılmadı.

**Yeni dosyalar:** `tests/hw-setup-01c-candidate.cjs` test-öncelikli **geçici** gerçek .ino aday oluşturucu; `tests/hw-setup-01c-qa.cjs` adayın gerçeğe dayalı `setup()/loop()` C++ mock regresyonu ve 16/8 MHz Arduino CLI derlemesini *aynı oturumda* isteyen birleşik kapı. Aracın kendisi kartı flash etmez, aktif .ino'ya yazmaz, Actions/Work/Codex kullanmaz.

**Kaynak tespit/yama planı ve yapısal dry-run:** Mevcut kaynakta 11 menü öğesi olduğu için yeni `HW Setup` ACTION indeksi **11** olarak eklenir ama *eski* EEPROM döngüleri yalnız indeks **0–10** (0–21 byte) ile sınırlanır. Dosyaya yeni `hw-setup-runtime.h` dahil edilir; boot, encoder Board/Driver/Confirm/Cancel, **uzun basmayla ayrı fiziksel teyit**, 16×2 `lcdRows` bağlanır. Motorun **yerel** Start, **Remote** start, **USB** p/d/c ve gerçek **dose/pump STEP** noktalarında bağımsız `SafeSetup::permit` kontrolü; reddedilirse STEP LOW + RNG idle + USB start kapatılması adayda hedeflenir. `tests/hw-setup-01c-candidate.cjs` için **14/14 kaynak anchor eşleşmesi**, **3/3 legacy EEPROM döngüsü korunması**, 27.191 karakterlik geçici aday üretimi, JS parse/dry-run ve test aracı kasıtlı simüle hata durumunda **exit 2 / BLOCKED** denetimleri PASS. **Bunlar SOURCE_CHECK/TEST_TOOL kontrolleridir, native firmware PASS değildir.**

**Yeni aday pozitif-negatif C++ test planı:** Kaynağın tüm `setup()/loop()` gövdesi mock encoder/EEPROM/GPIO/LCD/Serial ile derlenecek; boş EEPROM'da Local/Remote/USB/Cal no-STEP, onaylı dört giriş modu, 24.000 Cal STEP, Remote Hold/Pulse debounce+STOP, USB x, canlı inhibit baytı/CRC tamperi, Board/Driver rotasyon, 16×2 menü, uzun basma ile Save+Confirm, iptal/yeniden giriş, eski EEPROM verisine dokunmama senaryoları. `F_CPU=16000000UL` ve `8000000UL` host testleri geçtikten sonra **aday `.ino` dosyasının kendisi** iki gerçek Arduino Pro Mini hedefinde derlenecek. QA herhangi bir FAIL veya eksik marker için `HW_SETUP_01C_SOFTWARE_QA=BLOCKED` döndürür.

**Kapanmamış kapılar:** `HW_01C_FULL_SETUP_LOOP_CPP=NOT_RUN`, `HW_01C_AVR_16MHZ=NOT_RUN`, `HW_01C_AVR_8MHZ=NOT_RUN`; bu nedenle **HW Setup entegrasyonu aktif .ino'ya yazılmadı**. Ayrıca kaynak adayının gerçek STEP başına EEPROM okumasının darbe periyoduna ve **P0-02 stop gecikmesine etkisi REVIEW** (hız, fiziksel timing ve jitter ölçümü gerekir); elektronik brownout/inhibit byte atomikliği REVIEW. `HW_BENCH/HW_PUMP/USER_ACCEPTANCE=NOT_RUN`, endüstriyel `RELEASE_BLOCKED`.

**Sıradaki güvenli kapı:** Doğrulanmış güncel repo ZIP kökünde `node firmware/Peristaltic_Pump_Software_v1.05/tests/hw-setup-01c-qa.cjs` komutu (native C++17 + UBSan ve Arduino CLI 8/16 core gerekir). Gerçek host/AVR PASS çıktısı olmadan yeni firmware blob üretim main'e aktarılamaz. Test kusuru çıkarsa en küçük testli kaynak yamasıyla düzelt; P0-01'i ve ürün/release kilitlerini koru.


## 2026-10-31 — HW-SETUP-01C Windows gerçek QA + P0-02 STEP sıcak-yol engeli

**Kullanıcının Windows PowerShell kanıtı**: geçici, gerçek v1.05 kaynaktan üretilmiş HW Setup adayının `setup()/loop()` C++ regresyonu **16MHz 52/52 PASS** ve **8MHz 52/52 PASS**. Arduino CLI gerçek AVR derlemeleri **16MHz PASS** (19.254/30.720 Flash, 990/2048 RAM) ve **8MHz PASS** (19.250/30.720 Flash, 990/2048 RAM). `HW_SETUP_01C_SOFTWARE_QA=PASS`, exit **0**. Fiziksel test yapılmadı. C++ derleyicisinin önceden var olan `menu[].suffix=="uL"` / `"uL/min"` işaretçi eşitliği ve enum switch eksikleri için verdiği **dört warning** ayrı REVIEW; uyarılar PASS demek değildir.

**Ek kaynak denetimi ile kanıtlanan sıcak-yol kusuru:** `SafeSetup::permit()` her başarılı çağrıda 1 izin baytı ve iki 12-bayt profil slotunu okuyor: **25 EEPROM.read/izin**. Geçici adayın `case 0` / USB `case 9` ile `dose()/pump()` gerçek STEP yolunda iki `hwMotorAllowed()` çağrısı var: **50 EEPROM.read / yetkili STEP**. `tests/hw-setup-01c-step-timing-gate.cjs` gerçek kaynak yollarından bunu hesaplayıp **`HW_SETUP_01C_TIMING_GATE=BLOCKED`, exit 2** verdi. Bu **kaynak-audit bulgusudur**, gerçek AVR mikrosaniye ölçümü değildir.

**Kabul ayrımı:** `FULL_SETUP_LOOP_CPP=52/52×2 PASS`, `AVR_COMPILE=2/2 PASS`, **`STEP_TIMING_REGRESSION=BLOCKED/REVIEW`**. Yeni adayın gerçek frekansına, STOP tepki süresine ve P0-02'ye etkisi bilinmediği için **aktif .ino teslimi HOLD**. Kilitli P0-01 firmware Git blob **`6617cc38bde9bddf1c32cd0506915c0054556127`**, v1.04, AGENTS, manifesto değişmedi; `HW_BENCH/HW_PUMP=NOT_RUN`, `RELEASE_BLOCKED`.

**Sıradaki iş:** STOP/izin iptali davranışını koruyarak her STEP'teki 2× tam EEPROM profil taramasını kaldıran güvenli aday geliştirmek; CRC bozulması, EEPROM izin iptali, yerel/USB/Remote Dose/Pump/Cal, 16×2 LCD ve encoder negatif regresyonunu yeniden çalıştırmak. Değişmiş kaynak için **yeni** gerçek full-loop C++ ve iki AVR derlemesi zorunlu. P0-02 fiziksel zamanlama ayrıca REVIEW.


## 2026-10-31 — HW-SETUP-01D: yinelenen EEPROM iznini kaldıran geçici aday

**Başlangıç:** canlı `main 5bdd27f3e62b3b0fc2852a847fb35cdf2944160c`. Aktif v1.05 P0-01 `.ino` Git blob `6617cc38bde9bddf1c32cd0506915c0054556127` **değişmedi**. v1.04 arşiv, manifesto, `AGENTS.md`, kilitli profil/runtime başlıkları aynı. Önceki HW-SETUP-01C Windows **52/52 ×2** tam kaynak host ve **2/2 AVR** PASS ile profil **125/125 ×2**, runtime **486/486 ×2** ve P0-01 **32/32+42/42+2/2** tarihi test kanıtları aynen korundu; yeni HW-SETUP-01D'ye doğrudan aktarılmadı.

**Kök kusur:** gerçek `01C` adayında her yetkili STEP için Local `case 0` / USB `case 9` içindeki `hwMotorAllowed()` ve `dose()/pump()` içindeki ikinci çağrı toplam **2 × (1 inhibit + 24 profil EEPROM) = 50 EEPROM.read / STEP** yapıyordu. Bu, P0-02 jitter/stop gecikmesi için **kaynakta kanıtlı regresyon riski**, gerçek zamanlama ölçümü değildir.

**En küçük geçici `01D` düzeltmesi:** `tests/hw-setup-01d-candidate.cjs`, önceki onaylı P0-01 SHA'sını ve 14 HW Setup yamasını kilitleyen `01C` adayı üzerine dört eşleşme-kilitli kaynak değişikliği uygular. Local/USB üst işlemlerindeki **tekrar profil taraması kaldırılır**, ancak `dose()/pump()` içindeki **son STEP öncesi tam profil + CRC + arming doğrulaması korunur**; Local geçersiz mod fail-closed, Local ret STEP LOW/RNG idle ve menü çıkışı, USB ret `usb_start=false`; Remote başlatma ön kontrolü korunur. 27.237 karakterlik yeni geçici adayı oluşturur, asıl `.ino`yu değiştirmez.

**Yeni gerçek kaynak denetimi:** `tests/hw-setup-01d-step-timing-gate.cjs` Local Pump/Dose/Cal + USB Pump/Dose/Cal üzerinde kaynak yollarından **25 EEPROM.read / yetkili STEP**, tek son izin kontrolü, CRC/arming/Remote izin yolu varlığını doğruladı: `HW_SETUP_01D_TIMING_GATE=SOURCE_CHECK_PASS`. İki negatif mutant: gerçek `dose()` son izni kaldırıldığında `BLOCKED exit 2`, Local üst izni geri eklendiğinde `BLOCKED exit 2`. **İlk Remote başlatmasında ek preflight halen mevcut**; ilk adımın toplam işi sıradan sonraki adımlardan farklıdır. Statik sayı AVR mikro-saniye veya jitter ölçümü değildir.

**Gerçek header native kontrol:** Git blob `64157ee0ff275f2a504ef30728f94d0ff9f2f1b9` olan `tests/hw-setup-01d-runtime-hot.cpp` dosyası, değişmemiş `hw-setup-runtime.h` ve `hw-setup-profile.h` ile `g++ -std=c++11 -Wall -Wextra -Werror -pedantic -fsanitize=undefined` altında `F_CPU=16000000UL` **12/12 PASS**, `F_CPU=8000000UL` **12/12 PASS**: dokuz Local/Remote/USB izni için 25 okuma, canlı CRC bozulması sonraki kontrolde ret, inhibit byte iptali ve STOP reddi. **Bu gerçek bağımsız runtime testidir; bütün `setup()/loop()` testi değildir.**

**Yeni tam kaynak QA hazır ama henüz çalıştırılmadı:** `tests/hw-setup-01d-qa.cjs`, gerçek source `.ino` + geçici `01D` yamayı derleyen mevcut 52/52×2 host testine **8 yeni pozitif-negatif kontrol** ekler: Local/USB Pump/Dose için tam **25 read/STEP**, çalışma sırasında CRC bozma ve arming iptaliyle sonraki STEP engeli, Local STOP. Yeni adaya özel **iki gerçek Arduino CLI derleme** zorunludur; ortamda Arduino CLI yok. **01D FULL_SOURCE_CPP=NOT_RUN, AVR_COMPILE_8/16=NOT_RUN, firmware main commit=HOLD**. 01D için yalnız JS sözdizimi/aday ankrajları/gerçek header native ve kaynak yükü denetimleri PASS.

**P0-02 sınırı:** 50→25 EEPROM okuması kaynak düzeyinde doğrulandı; **P0-02 gerçek STEP zamanlaması, motor stop latansı, jitter, brownout/arm atomicity ve HW_BENCH/HW_PUMP = REVIEW/NOT_RUN; endüstriyel RELEASE_BLOCKED.** Motorun güvenli olduğunu yalnız simülasyondan iddia etme.

**Bir sonraki geçit:** Yeni `tests/hw-setup-01d-qa.cjs` testinin **full setup/loop 16/8 MHz host pozitif-negatif + çift Arduino AVR** gerçek çıktısını al. Gerekirse hedefli kaynak kusuru düzelt; 01D motor kaynak değişikliğini ancak yeni bütün kapılar PASS ve timing/STOP riski yeterince değerlendirildiyse `[skip ci]` main'e gönder. Fiziksel STOP/ürün kabulünü yine ayrı REVIEW/BLOCKED tut.


## 2026-10-31 — HW-SETUP-01D yazılım kabulü ve doğrulanmış kaynak teslimi

**Task ID:** HW-SETUP-01D / P0-02. **Base `main`:** `cc79cfef14520a002fdb12fa2667d9ea691eae82`. **Eski P0-01 v1.05 .ino blob:** `6617cc38bde9bddf1c32cd0506915c0054556127`. **Yeni aktif v1.05 .ino Git blob:** `04185aaa799b0af0bfa663b8acc8f5013a515066`. **Değişmez v1.04 blob:** `0d965c08947ba0b3170d22fbff806fc2e6f1e83a`. GitHub `main` teslim commit SHA'sı uzak HEAD teyidinde doğrulanmalıdır. `AGENTS.md`, `REVISION_MANIFEST.md`, profil ve runtime header dosyaları değiştirilmedi.

**Özgün kaynak kusuru / negatif kanıt:** Önceki geçici HW-SETUP-01C adayı hem Local/USB action tarafında hem `dose()/pump()` STEP kaynağında `hwMotorAllowed()` çağırarak bir yetkili STEP için **50 EEPROM.read** yapıyordu. Bu, gerçek AVR gecikmesi ölçülmüş olduğu anlamına gelmez; P0-02 zamanlama riskiydi. `tests/hw-setup-01d-step-timing-gate.cjs` kaynak-audit denetimi ile hem eksik STEP izni hem yinelenmiş izin negatif mutantları **exit 2 BLOCKED** verdi.

**En küçük kaynak düzeltmesi:** `tests/hw-setup-01c-candidate.cjs` ile eşleşen 14 kaynak yaması, üzerine `tests/hw-setup-01d-candidate.cjs` içindeki 4 eşleşme-kilitli düzeltme; gerçek motor pulsunda tek son `SafeSetup::permit()` kontrolü korunur, yinelenen Local/USB tam profil okuması çıkarılır. Local izin reddinde STOP/STEP LOW/RNG idle ve menüden çıkış; Remote başlangıç izin kontrolü; EEPROM 32/48 iki slot CRC, 64 inhibit byte, A4988 ilk öneri/DRV8825 alternatif, 8/16 MHz F_CPU uyumu ve 16×2 LCD kurulum akışı. Geçici kabul adayının `Peristaltic_Pump_Software_v1.05.ino` **27.237 karakterlik** çıktısı byte-özdeş şekilde yeni aktif blob `04185aaa799b0af0bfa663b8acc8f5013a515066` olarak aktarılır.

**Gerçek Windows pozitif-negatif kabul kanıtı (kullanıcı PowerShell, 2026-10-31):**
- `HW_SETUP_01D_SOURCE_HOT_PATH=PASS`: Local/USB Dose/Pump/Cal için kaynak düzeyinde **25 EEPROM.read / yetkili STEP**. Bu süre/jitter ölçümü değildir.
- `HW_SETUP_01D_FULL_SOURCE_FCPU_16000000=68/68 PASS`: gerçek `setup()/loop()` gövdeli C++ mock/UBSan; negatif testler izin baytı iptali, CRC bozulması, Local/USB/Remote başlatma, STOP, 24.000 STEP Cal, encoder/LCD/EEPROM. Bu fiziksel AVR'de yürütüm değildir.
- `HW_SETUP_01D_FULL_SOURCE_FCPU_8000000=68/68 PASS` (aynı kaynak, ikinci F_CPU).
- `HW_SETUP_01D_AVR_16MHzatmega328=PASS`: Flash **19.268/30.720 (%62)**, SRAM **990/2048 (%48)**.
- `HW_SETUP_01D_AVR_8MHzatmega328=PASS`: Flash **19.264/30.720 (%62)**, SRAM **990/2048 (%48)**.
- `HW_SETUP_01D_SOFTWARE_QA=PASS; ACTIVE_INO_UNCHANGED; RELEASE_BLOCKED`, test aracı çıkış kodu **0**. Önceki P0-01 32/32 + 42/42 + 2/2, profil 125/125 ×2 ve runtime 486/486 ×2 tarihsel PASS korunur, tekrar çalıştırılmadı.
- Clang dört warning verdi: `menu[].suffix` sabit-string işaretçi karşılaştırmaları ve `switch` içindeki `Open/Pressed` enum kapsaması. Bunlar bu kabul edilmiş adayda değiştirilmedi; ayrı **REVIEW**.

**Karar / ayrılmış kapılar:** `HW-SETUP-01D SOURCE_CHECK/HOST_CPP/AVR_COMPILE=PASS` ve aktif firmware'e yazılım kaynağı teslim edilebilir. **P0-02 gerçek stop latency, ölçülmüş STEP high/low frekansı, jitter, EEPROM okuma maliyeti, brownout güvenliği, fiziksel A4988/DRV8825/mikro-adım onayı, HW_BENCH/HW_PUMP/USER_ACCEPTANCE = REVIEW/NOT_RUN.** 25 EEPROM.read/STEP hala donanım hızını etkileyebilir; 50→25 statik iyileşme zamanlama performans garantisi değildir. Endüstriyel `RELEASE_BLOCKED`, G01–G10 `NOT_RUN` sürer. **Gerçek güvenlik donanımı doğrulanmadan cihaza yükleme/ürün kullanımı için onay verme.**

**Post-merge ayrı test:** `tests/hw-setup-01d-accepted-qa.cjs` kabul edilmiş aktif Git blob `04185aaa799b0af0bfa663b8acc8f5013a515066` ve v1.04 arşiv blob SHA'yı doğrulayıp, aktif `.ino` üzerinden aynı kaynak tam döngü mock + 8/16MHz Arduino CLI derleme kapılarını tekrar çalıştırır. `tests/hw-setup-01d-accepted-timing-gate.cjs` artık geçici aday üretimine bağlı olmadan gerçek aktif firmware kaynağında 25 okuma/STEP kaynak denetimi yapar. İki yeni test aracının Node sözdizimi **PASS**; post-merge gerçek Windows derleme **NOT_RUN**. Önceki `01D` aday QA aracı eski blob SHA'ya kilitli olduğundan post-merge yürütüm için kullanılmamalı.

**Sonraki aktif risk-öncelikli görev:** P0-02, STOP gecikmesi ve motor STEP zamanlamasını güvenli şekilde ölçmeye yönelik gerçek kaynak/host timing budget analizi, 8/16 MHz öngörülen gecikme sınırı ve gerçek donanım bench kabul planı; durum REVIEW. Sonrasında string pointer karşılaştırması warningleri için ayrı kaynak negatif test ve minimal onarım. GitHub Actions/Work/Codex/force-push yok; tüm policy/hardware/release kilitleri korunur.

## 2026-10-31 — P0-02 STOP ve derleyici uyarıları: kaynakta gerçek inceleme, geçici testli aday

**Baz:** `main 216d55f0e49c5b6c798a40fd5816cc5eb3ca0186`, aktif `Peristaltic_Pump_Software_v1.05.ino` Git blob **`04185aaa799b0af0bfa663b8acc8f5013a515066`**. **Yeni firmware değişikliği yapılmadı.** Önceki HW-SETUP-01D Windows tam `setup()/loop()` 68/68×2 + gerçek AVR 2/2 PASS, eski P0-01 ve diğer geçmiş kanıtlar korundu ve yeniden koşulmadı. Arşiv v1.04 blob, `AGENTS.md`, `REVISION_MANIFEST.md`, HW profil/runtime kilitleri değişmedi.

**Kaynak R0/P0-02 somut riskleri (gerçek `.ino`):**
1. `pump()` ve `dose()`, bir STEP için `delayMicroseconds(_delay_us)` işlemini iki kez beklemeli yapar; `_delay_us` kabulü **4–16000 µs**, yani yalnız iki yarım-periyot **8–32000 µs/STEP**. STEP/RNG, 25 EEPROM okuması, ISR, LCD, USB ve diğer kod süresi buna eklenir. **32 ms gerçek en kötü STOP gecikmesi değildir**; yalnız uzun STEP'in CPU bekleme yoludur.
2. `handle_remote_input()`, `loop()` ACTION/Serial/STEP işlemlerinin **sonrasında** çağrılır. Remote değişimi için ayrıca **30 ms debounce** gerekir. Dolayısıyla bir sonraki kontrol ve kararlı giriş beklenirken STOP gecikebilir. Fiziksel üst sınır **henüz ölçülmedi**.
3. USB `case 9` içindeki komutlar `Serial.parseInt()` kullanır: `p` = **2**, `d` = **3**, `w` = **1** senkron çağrı. Kaynakta `Serial.setTimeout()` yoktur; Arduino Stream varsayılanı **1000 ms/çağrı** (Arduino resmi Stream referansı / AVR çekirdek). Eksik ya da ayırıcı karakteri olmayan paketlerde ardışık timeout olasılığı **p≈2000 ms, d≈3000 ms, w≈1000 ms**; bunlar **kaynak + dokümantasyon koşullu çıkarımıdır, gerçek cihaz ölçümü değil**. Bloklayan ayrıştırma sırasında USB `x` veya Remote STOP işlenmesi gecikebilir; `x` dalı STOP/RNG LOW uygular ama ayrıştırıcının bekleyişini kesemez.
4. `Save Sett.` menüsündeki `delay(700)` ve 11×2 EEPROM yazımı, bir diğer açık bloklama yoludur (menüdeki kayıt işlemi sırasında). R0 stop-latency için ayrı incelenmelidir.

**Gerçek kaynak-hedefli native test:** `tests/p0-02-targeted-source.cpp` blob **`0973ab25b16a3e041e5d2208dd757fe95683956f`**, kabul edilmiş `.ino`dan **byte-özdeş** çıkarılmış `pump()` (407 karakter), `dose()` (594), `handle_remote_input()` (834) gövdelerini kullanır; üç fonksiyonun hem boyut hem içerik FNV kaynak karşılaştırması kontrol edildi. C++17 `g++ -Wall -Wextra -Werror -pedantic -fsanitize=undefined`, hem `F_CPU=16MHz` hem `8MHz` için ayrı **8/8 PASS**: 4 µs yarım periyot 8 µs model bekleme, 16000 µs yarım periyot 32000 µs model bekleme, devre dışı dozda STEP 0 ve Remote Hold 30ms debounce pozitif/negatif. **Mock zaman / CPU değildir; Arduino gerçek fiziksel STOP ölçümü değil**. `tests/p0-02-targeted-source-gate.cjs` kaynak-gövdesi eşleşmesini ve iki native derlemeyi tek komutta tekrar çalıştırır; gate aracı JS syntax PASS, bu scriptin bütün bileşik gerçek oturum koşusu **NOT_RUN**.

**USB negatif:** `tests/p0-02-source-audit.cjs` mevcut kaynakta 6× `parseInt`, STEP iki bekleme, Remote STOP'un sırası, kayıt 700ms ve **üç suffix pointer + bir switch warning** kaynak doğrulamalarını **SOURCE_AUDIT PASS** olarak gerçekleştirdi. `tests/p0-02-usb-blocking-negative.cjs` komut dallarından **2/3/1 senkron `parseInt`** çağrısını birebir saptadı: `P002_USB_BLOCKING_NEGATIVE_SOURCE_REPRO=PASS`. **Kaynak kusuru gösterildi; STOP güvenlik PASS değil.** Her iki kodun Node syntax ve canlı kaynak ile JS kontrollü source dry-run PASS; gerçek `Serial`/UART host zaman aşımı davranışı, tam yeni firmware fonksiyon testi ve AVR zamanlama **NOT_RUN**.

**Derleyicinin dört warning'i:** `menu[1].suffix=="uL"` **iki**, `menu[3].suffix=="uL/min"` **bir** yerde `const char*` içerik yerine işaretçi karşılaştırması kullanılıyor. Dördüncü `switch(b)` uyarısı `Open` ve `Pressed` no-op enumlarını açıkça listelemiyor. `tests/p0-02-warning-candidate.cjs` kabul edilen blob SHA ile sınırlandırılmış **geçici en küçük yama** hazırlar: 3× `strcmp(...)` ve açık `case Open/Pressed: break;`; **aktif .ino değişmedi**. `tests/p0-02-string-switch-native.cpp` test dosyası blob **`dd25fc39216ec63fb1fe2cff8a749296e82690dd`**, g++17/UBSan'da hem 16 hem 8 MHz define ile **7/7 PASS**: aynı yazıyı farklı bellekte tutan gösterge eski `==` hatasını yeniden üretir, `strcmp` doğru çalışır, enum no-op yan etkisizdir. Bunlar hedefli C++ testleri; **yeni warning-fix adayının tam setup/loop + AVR derlemesi NOT_RUN**.

**Tek komut kaynak/host kapsamı:** `tests/p0-02-one-shot.cjs`; kaynak ve USB negatif denetimleri + kaynakla aynı üç gerçek fonksiyonun 8/8×2 C++ testi + warning 7/7×2 C++ testini zorunlu koşar. **Bileşik kullanıcı Windows koşusu NOT_RUN**, yerel g++ hedefli testler ayrıca PASS.

**Karar:** `P0-02 SOURCE_CHECK=PASS (kusur kanıtı)`; `TARGETED_EXCERPT_HOST_CPP=8/8×2 PASS`, `WARNING_TARGET_CPP=7/7×2 PASS`; `STOP_LATENCY_HW/AVR_TIMING=REVIEW/NOT_RUN`. **R0 bloklayıcı USB parser henüz ONARILMADI ve P0-02 COMPLETE değil.** Bir sonraki tek aktif iş, USB `p/d/w/x` komutlarını ve eski paket biçimini koruyan **nonblocking + bounded input** geçici tasarımını, eksik/bozuk paketlerde STOP'u önceliklendiren gerçek `setup()/loop()` negatif-pozitif testle hazırlamak. Ancak yeni tam kaynak C++ regresyonu ve gerçek 8/16MHz AVR derlemesi olmadan `.ino` commit yok. Gerçek kart/HW_BENCH/HW_PUMP/USER_ACCEPTANCE NOT_RUN, G01–G10, endüstriyel **RELEASE_BLOCKED**; Actions/Work/Codex/force-push yok.


## 2026-10-31 — P0-02 / R0 USB bloklamayan aday ve gerçek native negatif testler

**Canlı baz `main`:** `49b2470ba3d7ab719712953f76595462b8479d65`. **Aktif firmware blob SHA:** `04185aaa799b0af0bfa663b8acc8f5013a515066` **değişmedi**. v1.04 arşivi, `AGENTS.md`, `REVISION_MANIFEST.md`, HW Setup profil/runtime dosyaları korunur. Önceden kabul edilmiş STEP/Remote `8/8 ×2`, derleyici uyarısı `7/7 ×2`, HW-SETUP-01D `68/68 ×2` host ve AVR `2/2` PASS tarihsel kanıttır; bu oturumda yeniden koşulmadı.

**Kaynak somut kusuru:** USB `case 9` içinde `p` için iki, `d` için üç, `w` için bir `Serial.parseInt()` çağrısı ile eksik/bozuk veri beklemesi ana döngüyü bloklayıp STOP kontrolünü geciktirebilir. R0/P0-02 kusuru kaynak-audit olarak kanıtlandı; gerçek fiziksel tepki zamanı ölçülmedi.

**Yalnız geçici yeni bileşen / aday:**
- `p0-02-usb-parser.h` (blob `20527923030414d7d385b5102b9307446ce7f5c7`): saf AVR uyumlu C++11, heap ve Arduino IO bağımlılığı yok; `p(rate,cal)`, `d(volume,rate,cal)`, `c`, `w(cal)`, `x` olayları; en fazla 48 payload byte / 10 sayısal basamak, `int32_t` overflow koruması, 40 ms sessizliğe göre son alan bitirme; x gelince hemen `kStop`, eksik pakette `kReject`, ilk komut baytında `kPending`. 40ms sessiz sonlandırma eski satırsonusuz biçime uyumluluk için adaydır, **kısmi ama sayısal olarak geçerli son alanın erken kabul edilmesi riski REVIEW**.
- `tests/p0-02-usb-candidate.cjs` (blob `3a82a13a0a07c15bdcb32c16073d9a0275f2bc7f`): kabul edilmiş aktif firmware blob SHA'ya kilitli geçici `.ino` üreteci; `case 9` içinde `Serial.parseInt()` sıfır; en fazla 32 UART byte/`loop`, aşırı buffer'da motor STOP + kalan veri discard, `usbApplyEvent` ayrı olay adaptörü, `exit_action_menu()` parser reset; Local/Remote `pump/dose/hwMotorAllowed` kaynakları korunur; çalışmakta olan motor yeni eksik p/d/w başlangıcında fail-closed durur. `w` yalnız tamamlanan ve `0 < cal <= menu[7].lim` durumda eski 11 ayarı yazar; yazma hâlâ senkron, STOP gecikmesi ayrıca REVIEW.
- `tests/p0-02-usb-parser-native.cpp`, `tests/p0-02-usb-parser-native-gate.cjs`: özgün yeni parser header doğrudan include edilerek **16MHz 36/36 PASS**, **8MHz 36/36 PASS**, yerel `g++ -std=c++11 -Wall -Wextra -Werror -pedantic -fsanitize=undefined`. Eski `p1000 5000`, `d10 1000 5000\n`, `w4200\r`, c/x, eski satırsonusuz p, eksik p/d/w, bozuk ayraç/negatif giriş/32-bit overflow, 48 byte sınırı, millis taşması, x preemption ve geri kazanım kapsamı. Güvenlik mutantında `x` tanıma değiştirilince gerçek C++ test `P002_PARSER_FAIL x immediate` / exit 1 verdi; mutant repo dışı.
- `tests/p0-02-usb-dispatch-gate.cjs`: geçici adayın *aynen kullandığı* `usbApplyEvent` fonksiyonunu ve kabul edilen firmware'deki **byte-özdeş** `pump()/dose()/handle_remote_input()` gövdelerini hedefleyen g++/UBSan host excerpt denetimi; yerel **16MHz 19/19 PASS**, **8MHz 19/19 PASS**. Bekleyen yeni komut eski RUN/RNG/STEP'i durduruyor; USB x anlık STOP olayı, hatalı w EEPROM yazmıyor; HW izni olmadığında STEP yok; Local ve Remote STOP korunuyor. Mock hesap fonksiyonları kullanılır; bütün `setup()/loop()` veya gerçek AVR kart testi **değildir**.
- `tests/p0-02-usb-candidate-source-gate.cjs`: GitHub'daki gerçek kabul edilmiş `.ino`ye karşı aday yama ankrajları/safety marker'lar ve korunan STEP/Remote fonksiyon gövdelerinin kaynak karşılaştırması **26/26 PASS**, bilerek eksik STOP/STEP/UART read korumalı üç negatif mutant **3/3 BLOCKED**. Geçici aday **28.671 karakter**, aktif `.ino` değişmez.

**Sınır ve bilinçli farklar:** Eskiden `parseInt()` kabul ettiği negatif/bozuk ayracı yeni parser reddeder (fail-closed); `p/d/w` son alanı isteğe bağlı satır sonu olmadan **40ms sessizlik** ile kapanır, bu iletişim protokolü zamanlama farkı ayrıca cihaz testi gerektirir. UART paketleri çok yoğun gelirse 32 byte/loop bütçe aşılırsa yazılım motoru durdurup kalan kuyruğu boşaltır; x STOP yalnız byte **işlendiğinde** etkilidir. Komut `w` kayıt süresi, STEP beklemeleri ve Remote debounce hâlâ fiziksel P0-02 REVIEW. `c` yalnız motor güvenlik izin kapısı ile STEP verir.

**Ayrı statüler:** `PARSER_HEADER_NATIVE_CPP=36/36 ×2 PASS`; `SOURCE_EXCERPT_USB_DISPATCH_NATIVE_CPP=19/19 ×2 PASS`; `CANDIDATE_SOURCE_CHECK=26/26 PASS, MUTANTS=3/3 BLOCKED`. **Yeni birleşik tam `setup()/loop()` C++ QA = NOT_RUN; yeni geçici aday için gerçek Arduino Pro Mini 16/8 MHz AVR derlemesi = NOT_RUN; HW_BENCH/HW_PUMP/STOP latency/USER_ACCEPTANCE = NOT_RUN/REVIEW**. Bu nedenle **aktif firmware teslimi HOLD**, endüstriyel `RELEASE_BLOCKED`; gerçek cihazda kullanıma onay verilmedi.

**Sonraki tek aktif iş:** Geçici adayın gerçek bütün `setup()/loop()` gövdesini kullanan mock Serial/EEPROM/GPIO/encoder/LCD negatif/pozitif regresyonu ve gerçek Arduino CLI 16/8 MHz derlemesi. Özellikle `p/d/w` sessiz bitiş, `x` önceliği, Local/Remote STOP, RNG/STEP, UART backlog/overflow, `w` EEPROM gecikmesi ve 40 ms sessizlik sınırı. Tüm yazılım kapıları PASS olsa bile fiziksel STOP frekansı/jitter ve brownout/industrial kabulleri REVIEW kalır. Actions/Work/Codex/force-push yok; [skip ci] main yalnız uygun kanıtla.


## 2026-10-31 — P0-02 nonblocking USB: gerçek kaynak tam loop / çift AVR QA geçidi hazır

**Canlı baz:** `main 03db8c6d56ebede8827353dfd4e0602c19427e81`; kilitli aktif firmware Git blob **`04185aaa799b0af0bfa663b8acc8f5013a515066`**; v1.04 arşivi, `AGENTS.md`, `REVISION_MANIFEST.md`, HW Setup profil/runtime blob kilitleri değişmedi. Önceden gerçek g++ ile geçilmiş bağımsız parser `36/36 ×2`, event-dispatch/source excerpt `19/19 ×2`, kaynak 26/26 + 3/3 mutant PASS kanıtları korunur; tekrar PASS olarak sayılmadı.

**Yeni geçici dosyalar:** `tests/p0-02-usb-full-loop-avr-qa.cjs` Git blob **`02d1a0684466f105678b3e1d552c30646e52400e`** ve `tests/p0-02-usb-one-shot-qa.cjs` blob **`83a1e03920968323d080564c4928faa58a232b23`**. Bunlar aktif `.ino`yu değiştirmez, karta yükleme yapmaz, yalnız daha önce kaynak SHA'ya kilitlenmiş `tests/p0-02-usb-candidate.cjs` üzerinden TEMP çalışma dizininde 28.671 karakterlik aday üretir.

**Yeni test kapsamı:** Önceki kabul edilmiş gerçek `setup()/loop()` C++ mock 01D test gövdeleri temel alınarak **22 ilave negatif/pozitif doğrulama** oluşturuldu. Geçici p/d satırsonusuz değerler için 40 ms sessizlikten önce ve sonra karar verme, eksik p/d/w, tam sayı taşması, bozuk ayraç, UART 32-byte/loop backlog fail-closed, `x` ile çalışan veya henüz tamamlanmamış p/d/c işlemlerini durdurma, `w0` reddi, geçerli w kayıt yolu, Local STOP, Remote Hold START/STOP ve STEP/RNG güvenli durumları hedeflenir. Eski p/d testlerinin beklentileri 40 ms sessizlik sonlandırmasına göre yeniden uyarlanmış, host `fresh()` sahte yeniden başlatma sırasında parser durumu sıfırlanmıştır. Geçici gerçek kodun `pump/dose/hwMotorAllowed/EEPROM/LCD/encoder` akışı aynı kaynak gövdesiyle test edilir.

**Gözlenen/doğrulanan:** Birleşik test aracının **Node sözdizimi PASS**; SHA kilitli adayın 28.671 karakter üretimi ve 43.547 karakterlik özgün `setup()/loop()` mock C++ oluşturma aşaması kontrollü JS dry-run **PASS**. Yeni kaynakta `Serial.parseInt()` yok; `hwMotorAllowed()` STEP kontrolleri iki yerde kalıyor, USB parser başlığı C++ test aramasına giriyor. Test aracında kasıtlı C++ derleyici hatası simülasyonu **exit 2 BLOCKED**. Bu sadece test altyapısı/ön doğrulama, gerçek yeni tam C++ PASS değildir.

**Zorunlu yeni kabul:** `P002_USB_FULL_SOURCE_FCPU_16000000` ve `8000000` gerçek g++/clang++ çalıştırmaları, kaynak negatif testleri ve `P002_USB_AVR_16MHzatmega328=PASS` ile `P002_USB_AVR_8MHzatmega328=PASS`. Bunlar bu oturumda **NOT_RUN**: çalışma ortamında Arduino CLI/avr-g++ bulunmuyor. Kullanıcı Windows ortamı daha önce Arduino CLI + clang++ ile çalıştırıldı. Kökten tek komut: `node firmware/Peristaltic_Pump_Software_v1.05/tests/p0-02-usb-one-shot-qa.cjs`. CLI veya C++ yoksa ya da koşu başarısızsa **çıkış 2/ BLOCKED**. PASS çıktısı gelmeden **aktif firmware ana dala commit yok**.

**Açık R0 sınırı:** 40 ms sessizlikle biten *son sayısal alan* aktarım kesintisinde kendisi geçerli bir sayı görünebilir; protokol sonlandırıcısı olmayan eski biçimde eksik ama sentaktik olarak geçerli son alanı her zaman güvenilir biçimde ayırmak mümkün olmayabilir. 32 byte UART budget aşımı STOP + drain; USB x fiziksel ISR önceliği değil, döngüde işlendiği anda STOP. Eski `w` kaydı senkron EEPROM beklemesi yapıyor; STEP (4–16000µs yarım-periyot), EEPROM izni (25 okuma/STEP) ve Remote 30ms debounce ile gerçek **P0-02 stop latency / jitter** ayrıca REVIEW. `HW_BENCH/HW_PUMP/USER_ACCEPTANCE=NOT_RUN`, G01–G10 fiziksel kilitleri ve endüstriyel `RELEASE_BLOCKED` devam eder.

**Sonraki iş:** Windows üzerinde yeni tek-komut QA'nın gerçek 16/8MHz host C++ ve AVR çıktısını al; kanıtlı kusur çıkarsa yalnız geçici adayı minimal onar ve negatif/pozitif testleri tekrar et. Adayın tüm yeni yazılım kapıları PASS olmadan aktif `.ino` değişikliği yok, fiziksel kabul kanıtsız PASS değildir.


## 2026-10-31 - P0-02 USB QA header/prototype native repair

Verified base main: ecdabed95788d5e99fac1af9b283f6f7a9b32418. Accepted firmware SHA: 04185aaa799b0af0bfa663b8acc8f5013a515066 (UNCHANGED). v1.04 archive, AGENTS, manifest, HW headers preserved.

Root issue: tests/p0-02-usb-full-loop-avr-qa.cjs generated a host C++ file with stub + prototypes + sketch. Its forward declaration for usbApplyEvent(const perpompa_usb::Event&) appeared before inclusion of p0-02-usb-parser.h, so a real C++ compiler could not resolve the namespace. This is a QA fixture bug, not an active firmware defect.

Minimal repair: generated host C++ now inserts parser header after the Arduino stub and BEFORE the generated function prototypes. The active .ino, parser, driver logic and Arduino AVR sketch candidate did not change.

New tests/p0-02-usb-prototype-order-gate.cjs Git blob: 778a72f8a3bd05f0cb95165c3641e6da26735745. Real g++ -std=c++11 -Wall -Wextra -Werror -pedantic -fsyntax-only at both F_CPU=16000000UL and 8000000UL: incorrect order correctly FAIL, corrected order PASS => 2/2 native PASS. SHA locked accepted firmware and parser, and verified exact new QA source ordering. Three changed Node scripts pass JavaScript syntax.

One-shot QA now requires this new native gate before full C++ and true 8/16 MHz AVR compiles; it fails closed with exit 2 on any failure. Previously accepted parser 36/36x2, event dispatch 19/19x2 and source 26/26 + 3 mutants preserved, NOT rerun and NOT proof of new full-loop.

Open gates: new full setup()/loop() real C++ NOT_RUN; real Arduino Pro Mini 16/8 MHz AVR candidate compiles NOT_RUN. Physical STOP latency, jitter, 40 ms framing REVIEW; HW_BENCH, HW_PUMP, user acceptance NOT_RUN; industrial RELEASE_BLOCKED. Production firmware HOLD.

Next: From fresh GitHub main ZIP on Windows with clang++ and Arduino CLI, run node firmware/Peristaltic_Pump_Software_v1.05/tests/p0-02-usb-one-shot-qa.cjs. Fix any further proven source/test bug in temporary candidate only; no active firmware commit until all new host and AVR gates PASS.

## 2026-10-31 — P0-02: UART kuyruğu/sessizlik sıralaması güvenlik düzeltmesi (yalnız geçici aday)

Canlı temel main 0533c0940e55766233330c426f99b94977ffb190. Kabul edilmiş aktif firmware blob 04185aaa799b0af0bfa663b8acc8f5013a515066, v1.04 arşivi, AGENTS.md, REVISION_MANIFEST.md, HW profile/runtime ve parser header kaynakları DEĞİŞMEDİ. Eski parser 36/36×2, dispatch 19/19×2, 26/26+3/3 mutant, prototip sırası 2/2 geçmiş PASS kanıtları korunur, tekrar çalıştırılmadı.

**Kaynakta kanıtlı yeni kusur:** Geçici p0-02-usb-candidate.cjs, USB case 9 içinde `usbParser.tick(millis())` çağrısını mevcut UART kuyruğunu okumadan ÖNCE yapıyordu. Örneğin `p1000 500` alınıp 41ms sonra son `0` UART kuyruğunda olduğunda, eski sıra 500 değerli erken `kPump` üretiyor; ardından gelen rakamı ayrı geçersiz komut kabul ediyor. Aynı sıralama kuyruğa gelmiş `x` STOP olayından önce geçici erken `kPump` olayı üretebiliyor. Bunlar kaynak/model kaynaklı risk; fiziksel STEP üretildiği iddia edilmedi.

**Minimal aday onarımı:** 32-bayt/loop sınırlı UART okuma ve backlog fail-closed işleminden SONRA, yalnız `!usbDropUntilEmpty && !Serial.available()` durumunda `usbParser.tick(millis())` çağrılır. Böylece kuyruğa gelmiş rakam/STOP baytı sessizlik kararından önce işlenir. `Serial.parseInt()` geri gelmedi; `pump()/dose()/Remote` gövdeleri, STEP izinleri, EEPROM, 16x2 LCD ve aktif .ino korunur.

**Gerçek hedefli native kanıt:** `tests/p0-02-usb-queue-order-native.cpp`, aynı Git blob SHA b1d1bbb8f3dc89f736c01a72cbb1934016622328 ile yerel `g++ -std=c++11 -O0 -Wall -Wextra -Werror -pedantic -fsanitize=undefined -fno-sanitize-recover=all` altında F_CPU=16000000UL **13/13 PASS**, F_CPU=8000000UL **13/13 PASS**. Eski tick-first mantığı erken kPump olayını yeniden üretiyor; feed-first son rakamı işler, kuyruktaki x'i STOP yapar, tam 40ms sessizlik ve millis wrap kontrolleri geçer. Bu parser + olay sıralaması model testidir; tam özgün setup/loop veya AVR zamanlaması değildir.

**Ek kaynak denetimi:** Güncel geçici aday üretimi 28.885 karakter. `tests/p0-02-usb-candidate-source-gate.cjs` **28/28 PASS**; kaynakta bozulmuş STOP, STEP, UART read veya sessizlik kuyruk kapısı için **4/4 mutant BLOCKED**. `tests/p0-02-usb-queue-order-gate.cjs` gerçek header, cpp ve aktif firmware Git blob SHA'larını doğrular; kaynak sıralamasını ve F_CPU 8/16MHz native 13/13 marker'larını zorunlu kılar. Tek komut `tests/p0-02-usb-one-shot-qa.cjs` mevcut prototip 2/2 kontrolünün yanına yeni sıra 2/2 kontrolünü ekler.

**Açık kabul:** Bu yeni geçici adayın tam `setup()/loop()` C++ pozitif-negatif koşusu **NOT_RUN**; gerçek Arduino Pro Mini 8 ve 16MHz AVR derlemesi **NOT_RUN** (bu oturumda arduino-cli/avr-g++ yok). Dolayısıyla yeni USB adayının aktif firmware teslimi **HOLD**; fiziksel P0-02 STOP latency/jitter, 40ms protokol ve brownout **REVIEW**, HW_BENCH/HW_PUMP/USER_ACCEPTANCE **NOT_RUN**, endüstriyel **RELEASE_BLOCKED**.

**Sıradaki kanıt geçidi:** Windows'ta mevcut clang++ ve Arduino CLI ile güncel GitHub main ZIP'inden `node firmware/Peristaltic_Pump_Software_v1.05/tests/p0-02-usb-one-shot-qa.cjs` koş; sonuçlar FAIL ise gerçek host/AVR kusurlarını test-öncelikli düzelt. Bütün yeni kapılar PASS olmadan aktif .ino değişmez. Work/Codex/Actions/force-push yok, [skip ci] main dışında teslim yok.