# PerPompa v1.05 — Endüstriyel Ürün Serbest Bırakma Kapısı
**Durum:** RELEASE_BLOCKED • **Hedef:** Doğrulanabilir endüstriyel ürün, sertifikalı olduğu henüz iddia edilemez.

## Ürün sınırı / eksik ürün bilgileri
Satış bölgesi; hedef sektör ve pompalanacak sıvı sınıfı (tehlikesiz/uygulamaya özel); çalışma döngüsü; pompa başına debi/hacim aralığı; izin verilen hata ve belirsizlik; güç/kart varyantları; basınç; çalışma sıcaklık/nem aralığı; PLC arayüzü ve makinenin bütünündeki güvenlik fonksiyonları netleştirilmelidir. Bunlar tanımlanmadan "endüstriyel uyumlu" denemez. Tehlikeli, tıbbi, gıda veya patlayıcı atmosfer uygulamaları ayrıca sınıflandırılmalı ve ayrı uygunluk süreci gerektirir.

## Sürüm serbest bırakma için 10 zorunlu kapı
| Kapı | Gerekli delil | Başlangıç |
|---|---|---|
| G01 Ürün gereksinimleri | Kullanım amacı, sınırları, onaylı fonksiyon/performans gereksinimleri ve test eşlemesi | NOT_RUN |
| G02 Tehlike analizi | ISO 12100 yaklaşımıyla tehlike/risk azaltma; güvenlik fonksiyonu gerekiyorsa bağımsız donanım/yetkin uzman değerlendirmesi | NOT_RUN |
| G03 Yazılım kanıtı | Gerçek kaynak host CPP + hedef AVR derleme, tip/taşma, negatif sınırlar, deterministik testler, hafıza/zamanlama | NOT_RUN |
| G04 Güvenli durum ve restart | Güç kesintisi/geri gelişi, stop/arıza, kontrol kaybı, yanlış yön; gerekiyorsa donanımdan bağımsız güvenlik yolu | NOT_RUN |
| G05 Elektrik ve motor | Kart/sürücü varyantı, güç, giriş/çıkış, sıcaklık, arıza, izolasyon ve koruma test planı | NOT_RUN |
| G06 Proses metrolojisi | İstenen debi/volüm, belirsizlik, doz tekrar edilebilirliği, hortum aşınması ve proses koşulları | NOT_RUN |
| G07 Dayanıklılık/EMC | Sıcaklık, nem, uzun süreli çalışma, ESD/EFT/EMC uygunluk ölçümleri | NOT_RUN |
| G08 Seri üretim / izlenebilirlik | Versiyon+bellek/binary hash, seri no, üretim testi, kalibrasyon kayıtları, geri dönüş prosedürü | NOT_RUN |
| G09 Düzenleyici değerlendirme | Satış pazarı, ürün sınıfı, uygunluk değerlendirme dosyası ve gerekirse kuruluş/laboratuvar raporları | NOT_RUN |
| G10 Bağımsız serbest bırakma | A02/A04/A05/A06/A07/A08 kanıt incelemesi + A09 karar kaydı; açık kritik risk sıfır | NOT_RUN |

**RELEASE_GO** yalnız G01–G10'un tüm gerekli öğeleri ilgili ürün konfigürasyonu için delilli PASS ise verilir. Her eksik kritik kanıt **RELEASE_BLOCKED**. Yazılım bir tehlikeli hareketi güvenle durdurmak için tek başına yeterli kabul edilmez; donanımsal bağımsız koruma gerekliliğine risk analizi karar verir.

## Uygulanabilirlik adayı standart ve mevzuat (sertifika değildir)
- ISO 12100:2010 — makine tehlike analizi ve risk azaltma.
- IEC 60204-1:2016+A1:2021 — makine elektrik ekipmanı (ürün makine kapsamındaysa).
- IEC 61010-1:2010+A1:2016 — ölçme/kontrol/laboratuvar ve endüstriyel proses kontrol ekipmanı; ürün kapsamına göre.
- IEC 61326-1:2020 — ölçme ve kontrol ekipmanı EMC gereksinimleri; ürün kapsamına göre.
- IEC 62061:2021+A1:2024 / ISO 13849-1 — güvenlikle ilgili kontrol işlevi gerekiyorsa, risk sınıflandırmasıyla.
- AB EMC 2014/30/EU ve uygunsa Makine mevzuatı (AB) 2023/1230: 20 Ocak 2027 uygulama tarihi; Türkiye'deki güncel ilgili düzenlemeler ayrıca kontrol edilmeli.
Resmî kapsam notları: https://www.iso.org/standard/51528.html ; https://webstore.iec.ch/en/publication/71256 ; https://webstore.iec.ch/en/publication/59769 ; https://webstore.iec.ch/en/publication/62793 ; https://webstore.iec.ch/en/publication/59927 ; https://single-market-economy.ec.europa.eu/sectors/mechanical-engineering/machinery_en
Bu liste koşulsuz zorunlu standart beyanı değildir. Güncel harmonize standart, ürün kapsamı ve piyasa giriş tarihi yetkili uzman tarafından doğrulanır.

## Kullanıcıya / piyasaya verilemeyecek iddialar
"Kesinlikle kusursuz", "CE onaylı", "SIL/PL sertifikalı", "endüstriyel güvenlik PASS", "akış sensörlü", "24/7 kesintisiz güvenilirlik" ve "ürün piyasaya hazır" ibareleri, kendilerine karşılık gelen gerçek bağımsız delil olmadan kullanılamaz.

## Test kayıtlarının asgari bilgileri
Cihaz seri numarası, PCB ve işlemci/clock varyantı, kaynak commit SHA, derlenmiş binary SHA, araç ve kütüphane sürümleri, güç/sıvı koşulları, ölçüm cihazı kalibrasyonu, planlanan tolerans, tarih, uzman/onaylayan, normal ve hata senaryosu sonuçları, sapma kaydı ve ham veri. Sensör ölçümünün yokluğunda RNG sadece komut edilen hareketi bildirir.

## Karar
**2026-10-31: ENDÜSTRİYEL ÜRÜN G01–G10 = NOT_RUN, RELEASE_BLOCKED.** Dosyanın eklenmesi kabul kanıtı değildir.
