# PerPompa v1.05 — NEXT_TASK / Çalışma devri

**Aktif manifest:** [REVISION_MANIFEST.md](REVISION_MANIFEST.md) v1.0  
**Başlangıç main HEAD:** `8624b2b78b0bcf4f83dd3388b5533b46e7b7d667` (tarihsel; her zaman canlı HEAD oku)  
**Arşiv v1.04 kaynak blob kilidi:** `0d965c08947ba0b3170d22fbff806fc2e6f1e83a`  
**Aktif görev:** `P0-01 — sınır değer ve motor zamanlama referans testleri`  
**Aşama:** TEST_FIRST / firmware düzeltmesi henüz yapılmadı  
**Kod kabulü:** PENDING; AVR_COMPILE NOT_RUN; HW_BENCH NOT_RUN; HW_PUMP NOT_RUN; USER_ACCEPTANCE NOT_RUN.

## Endüstriyel ürün kontrol ekibi (2026-10-31)

**Takım:** [QUALITY_TEAM.md](QUALITY_TEAM.md) A01–A09 protokolü oluşturuldu; bu, bağımsız canlı 9 ajan veya otomatik arka plan görevi olduğu iddiası değildir.
**Endüstriyel serbest bırakma:** [INDUSTRIAL_RELEASE_GATE.md](INDUSTRIAL_RELEASE_GATE.md) G01–G10 = NOT_RUN; **RELEASE_BLOCKED**. [RISK_REGISTER.md](RISK_REGISTER.md) R-001–R-012 açık, risk sayıları kalibrasyon değil.
**Özgün kaynak host C++ test hazırlığı:** [tests/native-math-harness.cjs](tests/native-math-harness.cjs) eklendi. Çalıştırılana dek HOST_CPP=NOT_RUN; testi yalnız referans kaynak kodundan ayıklanan gerçek C++ hesap fonksiyonları için kapsar; tam AVR/firmware değildir.
**Kilit:** `AGENTS.md`, `REVISION_MANIFEST.md`, v1.04 arşivi ve aktif v1.05 .ino değiştirilmedi.

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
