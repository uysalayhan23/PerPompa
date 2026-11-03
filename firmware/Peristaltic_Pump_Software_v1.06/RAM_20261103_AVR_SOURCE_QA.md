# PerPompa v1.06 — RAM-only gerçek AVR kabul kaydı (2026-11-03)

**Görev:** Kullanılmayan `String inputString` ve `inputString.reserve(200)` kodunun kaldırılması; motor, EEPROM, LCD, USB ve Remote işlevleri aynen korunur.

**Kaynak/derleme bilgileri:** GitHub main başlangıç HEAD `21c7136f1d5d6fe6ecd753e2220c7db1ce56efb8`. Değişmeyen üretim v1.06 kaynak blob `55b19a8b1017bf1924dc568439f920ec9c90064f`; RAM-only iki satırlık aday Git blob `957f963c1d4f59a04f18a896e9f6c92890061a49`.

## Kullanıcı Windows / Arduino CLI kanıtı

`Run-AVR-RAM-Only.ps1` betiği, SHA-kilitli orijinal kaynaktan iki eşleşme ile RAM adayını üretti. Kullanıcının 3 Kasım 2026 PowerShell çıktısı:

- `BASE=55b19a8b1017bf1924dc568439f920ec9c90064f`
- `RAM_ONLY_CANDIDATE=957f963c1d4f59a04f18a896e9f6c92890061a49`
- `DIFF=2 EXACT ANCHORS; MODBUS=NONE; NO_UPLOAD; NO_GITHUB_COMMIT`
- **16 MHz AVR PASS:** Flash **26760/30720**, statik SRAM **1362/2048**, kullanılmayan statik alan **686** bayt.
- **8 MHz AVR PASS:** Flash **26756/30720**, statik SRAM **1362/2048**, kullanılmayan statik alan **686** bayt.
- `RAM_ONLY_AVR=2/2`, `AVR_TEST_EXIT=0`.

Önceki v1.06 AVR: 16 MHz **27540 Flash**, 8 MHz **27536 Flash**, statik SRAM **1368**. Karşılaştırmalı sonuç **780 bayt Flash** ve **6 bayt statik SRAM azalışı**; kullanılmayan 200 baytlık heap rezervasyon girişimi kaldırıldı. **686 bayt statik boş SRAM, gerçek yığın tepe güvenlik payı olarak ölçülmedi.**

## Kaynak eşitliği / test ayrımı

GitHub kaynak ve aday içeriği bağımsız karşılaştırıldı: **tam iki kullanılmayan satır çıkarılmış**, `setup()/loop()` fonksiyon envanteri ve 13 kritik kaynak güvenlik deseni korunmuş (**SOURCE_CHECK 13/13**), üç statik negatif mutasyon denetimi (**3/3 SOURCE_MUTANT**) geçti. Bu **çalıştırılan host tam-döngü testi değildir**. Depodaki eski `p0-01-full-loop-host.cjs` daha eski v1.05 başlangıcı/yama dizisine SHA-kilitli; doğrudan güncel v1.06 için HOST_CPP PASS kanıtı olarak kullanılamaz.

**Kabul durumu:** `RAM_ONLY_SOURCE_CHECK=PASS`, `AVR_COMPILE=2/2 PASS`, `FULL_SETUP_LOOP_HOST_CPP=NOT_RUN`, `STACK_PEAK=NOT_RUN`, `PHYSICAL=NOT_RUN`. Kullanıcının şartına uygun olarak eksik tam native regresyon yüzünden **firmware main commit yapılmadı / HOLD**.

## Ayrı EEPROM kurtarma işi

Yalnız tutucu 17/48 ilk kayıt kurtarma adayı değerlendirilebilir; 31/48 servis kilidi korunur. Eski 48/48 kurtarma adayı **GÜVENSİZ / YASAK**. Kurtarma sırasında 161 kesinti, GCC ve Clang ayrı ayrı 315371/315371 native kontrol PASS; yeni `setup()/loop()` UI entegrasyonu ve gerçek AVR **NOT_RUN**. Onaysız kayıt silme veya motor izinlerini gevşetme yapılmayacak.

**Kilitler:** `main` yalnız, `[skip ci]`, 50 Actions çalışma izni mevcut fakat bu tur kullanılmadı; v1.05, v1.04, P0-04 otomasyonu ve Modbus'suz I²C-only kapsam korunur. `RELEASE_BLOCKED`.
