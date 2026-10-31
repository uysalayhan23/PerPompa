# PerPompa v1.05 — NEXT_TASK / Çalışma devri

**Aktif manifest:** [REVISION_MANIFEST.md](REVISION_MANIFEST.md) v1.1 — kullanıcı onaylı tespit → onar → test → teslim → sıradaki iş döngüsü  
**Başlangıç main HEAD:** `8624b2b78b0bcf4f83dd3388b5533b46e7b7d667` (tarihsel; her zaman canlı HEAD oku)  
**Arşiv v1.04 kaynak blob kilidi:** `0d965c08947ba0b3170d22fbff806fc2e6f1e83a`  
**Aktif görev:** `HW-SETUP-01C/P0-02 — 52/52 ×2 ve AVR 2/2 PASS; ancak yetkili STEP başına 50 EEPROM okuması timing REVIEW; .ino HOLD`  
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
