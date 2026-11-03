> **GÜVENLİK DÜZELTMESİ — 2026-11-03:** Bu raporun aşağıdaki 48/48 kurtarma PASS sonucu, sonraki negatif testteki *önceden geçerli kaydın commit işaretinin bozulması* senaryosunu kapsamıyor. İlk kurtarma adayı bu kaydı yanlışlıkla silebildi. Bu aday **GÜVENLİ DEĞİL / KULLANMAYIN**. Daha tutucu aday ve güncel kabul: [EEPROM_RECOVERY_SAFETY_REVIEW.md](EEPROM_RECOVERY_SAFETY_REVIEW.md). Eski sayılar yalnız tarihsel test sonucu, uygulanabilir kabul değildir.

# PerPompa v1.06 — Operatör onaylı ilk EEPROM kaydı kurtarma, RAM incelemesi (2026-11-03)

**Sadece inceleme ve QA kaydı.** Hiçbir aktif firmware, EEPROM yerleşimi, motor veya P0-04 otomasyonu değiştirilmedi. Temel `main`: `d3e65e6cc72d32663892a2f3f315d0abc2695835`; v1.06 aktif firmware blob: `55b19a8b1017bf1924dc568439f920ec9c90064f`.

## İlk kayıt elektrik kesintisi — kurtarma adayı

- Önceden gözlenen **50** ilk kaydetme kesintisi: 48 `LOCKED`, 1 `LEGACY_NEEDS_REVIEW`, 1 `NEW_READY`.
- Sadece test klasöründeki gerçek `p004-migration-guard-candidate.h` başlığının kopyasına **`recoverInterruptedFirstSave(...)`** API adayı eklendi; mevcut firmware başlığı veya kullanıcı arayüzü değiştirilmedi.
- **Dört şart:** açık yerel fiziksel onay, motor idle, Remote idle, USB idle. `kLocked`, `kMissing` kayıt, hiçbir committed marker yok, Slot B tamamen boş, Slot A ilk kayıtla uyumlu olmalı. Aksi durumda EEPROM'a yazmaz.
- Önce yarım Slot A **sondan başa**, sonra iki witness alanı temizlenir. Legacy 0..21, HW profil 32..59 ve HW inhibit 64 korunur. Başarılı sonuç **yalnız** `kLegacyNeedsReview`; motor çalışması veya yeni kayıt izni otomatik açılmaz.
- Simüle edilen **48/48 LOCKED** ilk-kayıt durumu kurtarılabildi. Kurtarmanın kendisinde **1.153** güç kesintisi noktası kontrol edildi; açık onay olmadan `kNewReady` durumuna geçilmedi.
- Gerçek C++ başlıkları + EEPROM mock: Linux **g++ 6.579/6.579 PASS**, **clang++ 6.579/6.579 PASS**, UBSan etkin.
- İki negatif mutant **2/2 yakalandı**: onay şartı kaldırma ve committed kayıt korumasını bypass etme. Kayıtlı/CRC-bozuk committed veriler kurtarma ile silinemiyor.
- Kaynak paketi SHA256: `bd8381b56b39a4545ac33119416d01e79e33cc624f77a767fc0e4f2c8ef6a930`, çıkarılıp **18/18** dosya SHA kontrolü ve native tekrar koşusu PASS.
- Testte kullanılan kurtarma başlığı SHA256: `633386d95403a75c604487b6a0ebdd37930d5308186832a99d1f359ef4d811b8`; host testi SHA256: `fa4f5fd9b82171b6794e89be4ad5895ffad4d8d59cafb07ebf974a9577135ff9`.

**Açık riskler:** Operatör onayını alan gerçek cihaz ekranı/menüsü entegre değil. EEPROM fiziksel brownout ve yazma atomikliği doğrulanmadı. Kurtarma işlevi kartta derlenmedi; **AVR_NOT_RUN**, `PHYSICAL_NOT_RUN`, `RECOVERY_UI_HOLD`.

## RAM / heap

- Aktif `.ino` içinde kullanılmayan `String inputString = "";` ve `inputString.reserve(200);` dışında `inputString` geçmiyor.
- Yalnız bu iki satırı çıkaran aday Git blob: `957f963c1d4f59a04f18a896e9f6c92890061a49` (**referanslanmayan Git objesi**, `main` firmware değiştirilmedi).
- Windows/Arduino CLI için ZIP içindeki `Run-AVR-RAM-Only.ps1` SHA-pinli canlı kaynakları indirir, **yalnız 2 anchor** temizler, aday Git SHA'yı doğrular ve **iki gerçek AVR derlemesi** yapar. `AVR_8_16_RAM=NOT_RUN` (kullanıcı çıktısı henüz yok).
- Statik SRAM: son doğrulanmış v1.06 için 1.368/2.048 bayt; 680 bayt raporlanan boşluk. Dinamik heap ve stack tepe kullanımı **NOT_RUN**. 200 bayt rezervasyon girişiminin kaldırılması ölçülen stack güvenliği değildir.

**Kilitler:** v1.05, v1.04, `P0-04` saatlik otomasyon, Modbus'suz yalnız I²C LCD kapsamı değişmez. Fiziksel güvenlik/üretim **RELEASE_BLOCKED**. GitHub Actions izinli 50 çalıştırmadan bu tur **0** tüketildi. Firmware değişikliği yalnız gerçek 8/16 MHz AVR PASS ve hedefli birleşik regresyonla değerlendirilecek.
