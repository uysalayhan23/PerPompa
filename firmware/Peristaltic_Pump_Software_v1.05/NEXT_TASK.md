# PerPompa v1.05 — NEXT_TASK / Çalışma devri

**Aktif manifest:** [REVISION_MANIFEST.md](REVISION_MANIFEST.md) v1.1 — kullanıcı onaylı tespit → onar → test → teslim → sıradaki iş döngüsü  
**Başlangıç main HEAD:** `8624b2b78b0bcf4f83dd3388b5533b46e7b7d667` (tarihsel; her zaman canlı HEAD oku)  
**Arşiv v1.04 kaynak blob kilidi:** `0d965c08947ba0b3170d22fbff806fc2e6f1e83a`  
**Aktif görev:** `P0-01 — sınır değer ve motor zamanlama referans testleri`  
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
