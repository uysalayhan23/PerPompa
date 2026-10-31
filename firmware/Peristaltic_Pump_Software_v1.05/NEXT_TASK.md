# PerPompa v1.05 — NEXT_TASK / Çalışma devri

**Aktif manifest:** [REVISION_MANIFEST.md](REVISION_MANIFEST.md) v1.1 — kullanıcı onaylı tespit → onar → test → teslim → sıradaki iş döngüsü  
**Başlangıç main HEAD:** `8624b2b78b0bcf4f83dd3388b5533b46e7b7d667` (tarihsel; her zaman canlı HEAD oku)  
**Arşiv v1.04 kaynak blob kilidi:** `0d965c08947ba0b3170d22fbff806fc2e6f1e83a`  
**Aktif görev:** `P0-01 — aday düzeltme çift AVR derlemesi ve tam döngü güvenlik kabulü`  
**Aşama:** TEST_FIRST → geçici P0-01 aday 8/16MHz AVR gerçek derleme PASS (2/2); FULL_LOOP kabul henüz BLOCKED. [P0_01_CANDIDATE.md](P0_01_CANDIDATE.md) kaydı oluştu.  
**Kod kabulü:** PENDING; HOST_CPP_SOURCE_EXTRACTED_BRANCH=42/42 PASS; **AVR_COMPILE(CANDIDATE)=2/2 PASS (kullanıcı CLI)**; FULL_LOOP_CPP NOT_RUN; HW_BENCH NOT_RUN; HW_PUMP NOT_RUN; USER_ACCEPTANCE NOT_RUN.

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
