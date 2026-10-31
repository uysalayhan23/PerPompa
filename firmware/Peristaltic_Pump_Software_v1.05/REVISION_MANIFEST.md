# PerPompa v1.05 — Sabit Revizyon ve Kabul Manifestosu
**Manifest sürümü:** 1.1 | **Tarih:** 2026-10-31 | **Yetkili çalışma dalı:** `main`
**Kapsam:** Yalnız `firmware/Peristaltic_Pump_Software_v1.05/` ve ona ait test/dokümanlar.
**Durum:** ACTIVE POLICY; bu dosya kendi başına hiçbir firmware fonksiyonunu PASS yapmaz.

## 0. Yeni sohbet / görev devralma zorunluluğu

Her yeni oturum ilk işlem olarak **canlı** `main` HEAD'i, bu `REVISION_MANIFEST.md`, `NEXT_TASK.md`, `TEST_MATRIX.md`, kök `AGENTS.md` ve v1.05 kaynak dosyasını okumalıdır. Daha önceki sohbetin SHA'sını güncel sayma. Bu kontrol yapılmadan kod düzeltmesi, commit veya "PASS/COMPLETE" yok. Araçlar dosyaları otomatik okumuyorsa elle okut; yalnızca belleğe güvenme. Yeni sohbet açılması otomatik veya arka planda çalışma başlatmaz.

## Politika dosyalarının sabitlenmesi

**REVISION_MANIFEST.md ve kök AGENTS.md politika kilidindedir.** Kullanıcının bu iki dosyada açıkça onayladığı bir değişiklik olmadan kurallar gevşetilemez, kaldırılamaz, taşınamaz veya yeniden yazılamaz. Normal geliştirme sırasında sadece kanıta göre `NEXT_TASK.md` ve `TEST_MATRIX.md` güncellenir; bu güncellemeler politika istisnası oluşturamaz. GitHub `main` şu anda sunucu tarafı korumalı dal değildir; bu politika sürece ve kontrol kapısına dayanır, GitHub yöneticilerinin dosyaları teknik olarak değiştirmesini engelleyen bir yetki kısıtlaması değildir.

## Ana çalışma çizgisi — kesintisiz kanıtlı revizyon döngüsü (kullanıcı onayı, 2026-10-31)

**Tespit et → kaynakta en küçük güvenli onarımı yap → özgün kaynakla pozitif/negatif + regresyon test et → yalnız ilgili kabul kapıları PASS ise [skip ci] ile main'e teslim et, uzak HEAD/blob SHA doğrula → sıradaki en yüksek öncelikli açık göreve geç.** Bu tüm yeni sohbetlerde ve uzman incelemelerinde esas iş yapma sırasıdır.

- Çalışma çıktı odaklıdır: her ara bulgu için ayrı uzun rapor yok; bir oturumda mümkün olan ardışık güvenli iş paketleri bitirilir. Nihai teslim yalnız değiştirilen gerçek dosya, ölçülen test türü/sonucu, uzak SHA, açık engel ve sonraki görevi kısaca bildirir. Yanıt vermek arka planda iş başlatmaz.
- Kaynakta düzeltilmemiş risk `PASS` ilan edilemez. Kusurun gerçek kaynağını kanıtlayamıyorsan yalnız kayıt aç. Güvenli ve test edilmiş gerçek kaynak yaması olmadan salt raporu "düzeltme" sayma.
- **Kalite kapısı vazgeçilmez:** Test yapılamıyorsa veya derleme kanıtı eksikse firmware yamasını main'e gönderme. Sorunu açıkça `BLOCKED/NOT_RUN` olarak kaydet; gerekli test altyapısını/kanıtlarını veya bağımsız güvenli sonraki işi ilerlet. Kontrolsüz kod aktarımıyla "devam" hedefi gerçekleştirilemez.
- İş sırasını A02 emniyet ve A09 sürüm incelemesi denetler; risk önceliği `R0 kontrolsüz start/stop/safe-state > R1 yanlış dozaj/hız/yön > R2 kullanılabilirlik > R3 kozmetik`. Tek seferde bir kaynak revizyonu; bağımsız test ve belgeler paralel hazırlanabilir.
- Ticari ürün için `RELEASE_BLOCKED` kilidi, [INDUSTRIAL_RELEASE_GATE.md](INDUSTRIAL_RELEASE_GATE.md) G01–G10 fiziksel/uygunluk kanıtları gelene kadar korunur. Host C++ test sonucu gerçek kart ya da ticari uygunluk değildir.
- Arşiv, geçerli sürüm davranışı, pinler, encoder/LCD, Remote/RNG kilitleri ve politika belgelerini gevşetme. Bu bölümü değiştirmek için kullanıcıdan yeni açık izin gerekir.

## 1. Değişmez referans ve kapsam kilitleri

- **Korunmuş v1.04:** `firmware/archive/Peristaltic_Pump_Software_v1.04/Peristaltic_Pump_Software_v1.04.ino`; başlangıç Git blob SHA `0d965c08947ba0b3170d22fbff806fc2e6f1e83a`. Arşiv kaynak ve README'sini yeniden yazma, silme, yerini değiştirme.
- **v1.05 başlangıcı:** `firmware/Peristaltic_Pump_Software_v1.05/Peristaltic_Pump_Software_v1.05.ino`; başlangıç blob SHA aynı. Sonraki v1.05 değişiklikleri ayrı Git geçmişiyle izlenir.
- **Başlangıç HEAD:** `8624b2b78b0bcf4f83dd3388b5533b46e7b7d667` (sadece referans; canlı HEAD yerine geçmez).
- D5 Remote giriş: `INPUT_PULLUP`, 30 ms debounce; Hold/Pulse mevcut anlamları. A0 RNG çıkışı: motor adım **komutu** bilgisidir, fiziksel akış geri bildirimi değildir.
- Motor STEP D7 / DIR D6; 200 tam adım/devir, `STEP_MODE=4`; Cal 30 dönüş hedefi. Kart frekansı/gerilimi ve motor sürücüsü fiziksel olarak doğrulanmadan zamanlama veya pinleri değiştirme.
- Dose hedefi dolunca durma, Pump stop verilene dek çalışma, Cal dönüş sayısı, mevcut encoder/LCD iyileştirmeleri, menü, kaydedilmiş uyumlu ayarlar ve Remote/USB komut biçimi korunmalıdır. Farklı davranış amaçlanıyorsa açık risk analizi ve kontrollü kabul gerekir.
- Gizli bağımlılık veya fiili donanım testi yapıldığı iddia edilemez. Aktif kaynak dışındaki Golden/arsiv/ürün belgelerine yan etki yok.

## 2. Her değişiklik için zorunlu kalite kapıları

1. **Canlı temel:** GitHub `main` HEAD ve hedef blob SHA oku; çalışma ağacının bazını sabitle, eşzamanlı değişikliği kontrol et.
2. **Somut kusur:** Kesin kod bölgesi, tetikleyen giriş, beklenen/mevcut sonuç ve etkilenen modülleri yaz. Hipotez = REVIEW; önce başarısız karşı-örnek üret.
3. **Fayda ve risk:** "Hiç düzeltmesek daha mı iyi?" sorusunu yanıtla; ölçülebilir fayda, regresyon ihtimali, geri alma yöntemi belirt. Fayda kanıtlanmıyorsa **HOLD / DO_NOT_CHANGE**.
4. **En küçük yama:** v1.05 dışına dokunma; paketleri birleştirme, temizleme/refactor veya menü/encoder/Remote yeniden tasarlama. Mevcut geçerli değerlerdeki sonuçlar eşdeğer olmalı.
5. **Hedefli pozitif-negatif:** Özgün kaynak/gerçek derleyiciyle derleme ve ilgili modülün iki yönlü testleri; ardından ortak regresyon matrisi. Salt JS modeli, regex ve matematiksel tahmin **native PASS değildir**.
6. **Ayrı statüler:** SOURCE_CHECK, MODEL_CHECK, HOST_CPP, AVR_COMPILE, HW_BENCH, HW_PUMP, USER_ACCEPTANCE. Koşulmayan = `NOT_RUN`; kanıt eksikliği = `REVIEW`. `FAIL` veya `NOT_RUN` otomatik PASS/COMPLETE'a çevrilmez.
7. **Teslim kapısı:** En az hedefli gerçek kaynak pozitif-negatif testleri ve gerekli derleme kanıtı yoksa **firmware yaması main'e gönderilmez**. Donanım gerektiren kabul alanları ayrı REVIEW kalır. Dokümantasyon/test iskeleti, kodu değiştirmediği açıkça belirtilerek ayrı commit edilebilir.
8. **Git disiplini:** Actions yok, Work/Codex yok (kullanıcı tekrar izin vermedikçe), force-push/koruma atlatma yok, her commit `[skip ci]`; yalnız `main`, önce güncel HEAD, sonra fast-forward ve HEAD eşleşmesini gözeten `expected_sha`, en sonda uzak HEAD + değişen blob SHA kontrolü. HEAD kayarsa yeniden incele, kör tekrar yok.
9. **Geri dönüş:** Başarısız yama ayrı kaydedilir; v1.04 hiçbir koşulda rollback ile değiştirilmez. Geri dönüş, yalnız onaylı v1.05 değişikliklerinin temiz ve denetlenebilir biçimde geri alınmasıdır.
10. **Kayıt:** Her inceleme/teslimde `NEXT_TASK.md` içindeki tek aktif görevi, test sayılarını ve gerçek statüleri güncelle. Bir oturum sadece rapor ürettiyse "onarım bitti" deme.

## 3. Sabit test/koruma matrisi

Bkz. [TEST_MATRIX.md](TEST_MATRIX.md). Özellikle:
- Sıfır/negatif/aşırı hız, sıfır/çok küçük kalibrasyon, maksimum hacim, 16-bit gecikme ve 32-bit adım sınırı.
- Dose/Pump/Cal normal ve hata girişleri, başlangıç/stop, yön ve RNG çıkışında yanlış-pozitif yok.
- USB p/d/c/w/x: doğru/hatalı/eksik komut, işlem sürerken yabancı karakter, zaman aşımı, stop; bir hata motora yanlış adım veya sahte RNG üretmemeli.
- EEPROM boş/bozuk/geçerli Hold/Pulse, kayıtlı CW/CCW, güç açılışında Remote davranışı. **Hold otomatik başlangıcı mevcut davranış**; güvenlik kararı alınmadan otomatik değiştirme.
- Encoder hızlı/ters adım, tıklama, stop sonrası kuyruk, LCD iki satır, 16 sütun, kritik menü geçişleri.
- Kart 8/16 MHz ve gerçek kontrolcü varyantları ayrı doğrulanmalı.

## 4. Risklere dayalı iş sırası

| ID | Durum | Risk / beklenen fayda | Kabul kapısı |
|---|---|---|---|
| P0-01 | REVIEW | Sıfır kalibrasyon/hız, aritmetik/16-bit zamanlama, aşırı adım; yanlış motor parametrelerini engelleme | Önce referans negatif + geçerli aralık pozitif, sonra gerçek kaynak testleri |
| P0-02 | REVIEW | Step üretimi ve gecikme yüzünden stop gecikmesi; daha öngörülebilir durma | Gerçek zaman ölçümü, tüm durdurma yolları |
| P0-03 | REVIEW | USB parse/son karakter durumu, RNG uyumu | Geçerli ve hatalı komut/stop regresyonları |
| P1-01 | REVIEW | Geçersiz EEPROM değeri ve açılışta Remote Hold davranışı | Güç açılış senaryoları, kayıtlı geçerli Hold/Pulse korunması |
| P1-02 | REVIEW | Kayıtlı motor yönünün açılışta uygulanmaması | CW/CCW EEPROM ve menü geçişleri |
| P1-03 | HOLD / DO_NOT_CHANGE | Encoder ve LCD v1.04 iyileştirmeleri | Somut yeni kusur olmadan yeniden yazım yok |
| P2-01 | NOT_RUN | Arduino gerçek derleme/kütüphane bağımlılıkları | Derleyici çıktısı, kart tanımı ve sürümler |
| P2-02 | REVIEW | Teknik belge/kılavuz güncelliği | Aktif firmware doğrulaması sonrası belge |

**Sıra tek aktif görev:** P0-01. P0-01 kaynak/test kapısı geçmeden diğer modüllere dokunma; güvenlikle ilgili yeni kritik bulgu olursa kayda al ve önceliği gerekçeyle değiştir.

## 5. Kabul kayıt biçimi

Her revizyon için şu alanlar zorunlu: `Task ID | Base HEAD/blob | Observed FAIL | Minimal diff | Positive native proof | Negative native proof | Regression proof | AVR compile | Hardware status | Benefit/regression | New HEAD/blob | Remaining REVIEW/HOLD`. Duruma "PASS" yazılması yalnız ilgili test kategorisini ifade eder, bütün ürün kabulü değildir.

## 6. Devralma talimatı (yeni sohbet için)

`uysalayhan23/PerPompa` güncel `main` üzerinden devam et. Önce `AGENTS.md`, `firmware/Peristaltic_Pump_Software_v1.05/REVISION_MANIFEST.md`, `NEXT_TASK.md` ve `TEST_MATRIX.md` dosyalarını, ardından aktif v1.05 kaynak kodunu oku. v1.04 arşiv blob `0d965c08947ba0b3170d22fbff806fc2e6f1e83a` kilidini kontrol et. Tek aktif P0-01 görevinden tespit → pozitif-negatif özgün test → en küçük doğrulanmış revizyon → [skip ci] main → uzak HEAD/blob SHA sırasıyla ilerle; kanıtsız PASS/COMPLETE, Work/Codex/Actions, force-push yok. Donanım NOT_RUN olarak ayrı kalsın. Her teslimde NEXT_TASK'ı güncelle.

> Bu dosyalar gelecekteki oturumlarda kalıcı kaynaktır; sohbetin bunları otomatik okuduğunu varsayma. Her oturumda gerçekten GitHub'dan tekrar yükle.
