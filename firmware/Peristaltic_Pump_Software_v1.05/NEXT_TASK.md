# PerPompa v1.05 — NEXT_TASK / Çalışma devri

**Aktif manifest:** [REVISION_MANIFEST.md](REVISION_MANIFEST.md) v1.1 — kullanıcı onaylı tespit → onar → test → teslim → sıradaki iş döngüsü  
**Başlangıç main HEAD:** `8624b2b78b0bcf4f83dd3388b5533b46e7b7d667` (tarihsel; her zaman canlı HEAD oku)  
**Arşiv v1.04 kaynak blob kilidi:** `0d965c08947ba0b3170d22fbff806fc2e6f1e83a`  
**Aktif görev:** `P0-01 — gerçek Arduino derleme + hata anında motor çıkış kapısı kabulü`  
**Aşama:** TEST_FIRST → aday geçici host denemesi; firmware düzeltmesi AVR derlemesi/test kilidi nedeniyle hâlâ `BLOCKED`. [P0_01_CANDIDATE.md](P0_01_CANDIDATE.md) kaydı oluştu.  
**Kod kabulü:** PENDING; HOST_CPP 4 normal PASS + 2 geçersiz dönüşüm reproduce; AVR_COMPILE NOT_RUN; HW_BENCH NOT_RUN; HW_PUMP NOT_RUN; USER_ACCEPTANCE NOT_RUN.

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
