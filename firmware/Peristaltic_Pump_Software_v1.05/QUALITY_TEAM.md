# PerPompa v1.05 — Endüstriyel Mühendislik ve QA Ajan Takımı
**Sürüm:** 1.0 • 2026-10-31 • Durum: Team Protocol ACTIVE / Ticari ürün kabulü NOT_READY

## Takımın gerçek çalışma modeli
Bu dosya dokuz **uzman sorumluluğunu ve birbirinden ayrı kalite imzalarını** tanımlar; dokuz ayrı çalışan yapay zekâ örneğinin otomatik açıldığını veya işin arka planda yürütüldüğünü iddia etmez. Tek oturumlu asistan birden fazla rolün analizini hazırlayabilir, ancak **aynı asistanın iki ayrı görüşü gerçek bağımsız doğrulama sayılmaz**. Kritik donanım, fonksiyonel emniyet ve mevzuat için uygun nitelikte gerçek uzman/kuruluşun bağımsız kanıtı gerekir.

| ID | Rol | Sorumluluk | Gerekli çıktı |
|---|---|---|---|
| A01 | Sistem mimarı / ürün gereksinimleri | Kullanım amacı, arayüz ve sistem sınırları, gereksinim–test izlenebilirliği | REQUIREMENTS.md, risk bağlantıları |
| A02 | Emniyet ve risk sorumlusu (veto) | Tehlike, arıza modları, güvenli duruma geçiş, reset ve beklenmeyen yeniden başlatma | RISK_REGISTER.md, risk azaltma kanıtları |
| A03 | AVR firmware mühendisi | v1.05 minimal yama; ISR, zamanlama, tip taşması, bellek ve EEPROM kontrolü | Kod diff'i ve kanıt matrisi |
| A04 | Bağımsız doğrulama / adversarial QA | Pozitif–negatif, sınır, durum geçişi, regresyon, mutation/fault injection planı | Hedefli native testler, hata kanıtı |
| A05 | Donanım / HIL doğrulama | Gerçek kart, STEP/DIR/RNG/Remote, enerji kesme, giriş/çıkış, watchdog | İzlenebilir cihaz/test raporu |
| A06 | Metroloji / proses kalite | Debi, dozaj, kalibrasyon, tolerans, belirsizlik, hortum ve koşul etkisi | Ölçüm verileri, kalibrasyon planı |
| A07 | EMC / çevresel dayanıklılık | ESD, EFT, gerilim dalgalanması, titreşim, sıcaklık, bağışıklık ve emisyon | Laboratuvar protokolü ve sonuçları |
| A08 | Düzenleyici uygunluk / doküman | Pazar, ürün sınıfı, uygulanabilir standartlar, kullanım kılavuzu, teknik dosya | Uygunluk kapsamı ve kanıtlar |
| A09 | Bağımsız sürüm kapısı (veto) | Eksik kanıt, test bağımsızlığı, değişiklik izinleri, sürüm/geri alma | RELEASE_GO / RELEASE_BLOCKED kararı |

## Görev dağıtımı ve dört göz kuralı
1. A01 ürün gereksinimini ve kabul ölçütünü tanımlar; A02 tehlike kapsamını belirler.
2. A04 önce özgün v1.05 üzerinde kusuru yeniden üretir: kritik vakada kaynağa dokunmadan negatif test.
3. A03 yalnız gerekli küçük değişikliği önerir; A04 ayrı test yürütür. Geliştirici kendi yamasının tek onaylayıcısı olamaz.
4. A02 güvenli başlangıç/stop ve olası zarar açısından veto hakkına sahiptir.
5. A05/A06/A07 fiziksel ve çevresel kanıtları sunar; yoksa ilgili kategori NOT_RUN/REVIEW kalır.
6. A08 ürün için gerçekten geçerli düzenlemeleri belirler. Belirsiz klasifikasyonla CE veya standart uyumu iddiası yapılamaz.
7. A09 belgeleri, karşı-örnekleri, kaynak SHA ve bütün kapıları denetleyip yalnız kanıtla durum verir.
8. A01–A09 çıktılarını aynı yapay zekâ hazırlıyorsa bu "bağımsız insan doğrulaması" sayılmaz; karar verilmeden önce tarafsız gerçek uzman teyidi ayrıca istenir.

## Ajanların gözlem hedefleri
**Yazılım:** hesaplama/taşma, adım süresi, ISR, buton sıçraması, LCD, EEPROM veri bütünlüğü/aşınması, seri veri bozulması, SRAM, reset, uzun dönem uptime, saat taşması, motor driver başlangıcı, RNG yanlış pozitif, tam/eksik USB komutları. 
**Sistem:** enerji geri gelmesi ve yeniden çalışma, stop latency, motor sıkışması, akış olmaması, aşırı basınç ve sızıntı, hortum yaşlanması, kalibrasyon sapması, beklenen/istenmeyen yön, bakım ve insan erişimi. 
**Çevre/üretim:** gürültü/EMI/ESD, güç kalitesi, ısı-nem, kablo yerleşimi, varyant uyumluluğu, seri numarası, firmware binary SHA, son hat testi, hata oranı ve geri çağırma izi.

## Öncelik ve tek aktif firmware görevi
Öncelik kuralı: önce insan/tesis güvenliği; sonra kontrol dışı çalışma ve yanlış dozaj; sonra veri bütünlüğü; sonra kullanım kolaylığı ve yeni özellikler.
- **R0 / BLOCKER:** kontrolsüz start, stop başarısızlığı, güvenli durum, elektriksel/sıvı proses tehlikesi, güvenlik fonksiyonu doğrulama yokluğu.
- **R1 / HIGH:** yanlış dozaj/hız, kalibrasyon, yön, ölçüm belirsizliği, seri komut karışması.
- **R2 / MEDIUM:** kullanılabilirlik, ekran, kayıt tutarlılığı.
- **R3 / LOW:** kozmetik ve yeni özellikler.
Takım paralel inceleme notu tutabilir ama **tek aktif yazılım revizyonu P0-01**; kilitli REVISION_MANIFEST ve AGENTS.md öncelik/kapsamına aykırı kaynak değişikliği yapamaz.

## Zorunlu kalite kontrol kapısı
Her değişiklikte gereksinim ID → risk ID → negatif karşı-örnek → minimum kaynak diff → pozitif native C++/AVR kanıtı → negatif native test → ortak regresyon → gerçek cihaz gerektiğinde HIL/metroloji → bağımsız review → uzak HEAD/blob SHA kaydı.
- Test sınırına erişilemediyse **BLOCKED/NOT_RUN**: sahte PASS veya "kusursuz" ifadesi yok.
- Safety/uygunluk ve ticari ürün serbest bırakma şartları [INDUSTRIAL_RELEASE_GATE.md](INDUSTRIAL_RELEASE_GATE.md) içindedir.
- Bu belge kök AGENTS.md ve REVISION_MANIFEST.md politika kilitlerini değiştirmez.

## Yeni sohbet devralma
Canlı main HEAD → AGENTS.md → REVISION_MANIFEST.md → NEXT_TASK.md → TEST_MATRIX.md → bu TEAM dosyası → INDUSTRIAL_RELEASE_GATE.md → RISK_REGISTER.md → v1.05 gerçek kaynak. A01–A09 görev ve imza ayrımını uygula; tek asistanın incelemesi ile bağımsız fiziksel/doğrulama onayını birleştirme. Belgeler eksikse READ_BLOCKED bildir.
