# PerPompa v1.06 — EEPROM kurtarma karşı-örneği ve güvenli sınır (2026-11-03)

**ÖNCELİK R0 — REVIEW / NOT FOR FLASH.** Kullanıcı hedefi: ilk enerjilendirmede veri kaybına veya izinsiz motor hareketine yol açabilecek kurtarma kusurlarını fiziksel teste geçmeden yakalamak.

## Tespit: önceki kurtarma adayı güvenli değil

Orijinal aday `recoverInterruptedFirstSave()`, önceki kayıt başarıyla `NEW_READY` olduğunda Slot A'daki yalnız commit marker byte'ı `0xa5`'ten `0x00`'a bozulursa, durumu ilk yarım kayıtla karıştırıp ayar verilerini silebiliyordu. Yeni negatif native test ilk kaynakta **FAIL** üretti ("damaged previously committed marker MUST NOT erase data"). Bu nedenle önceki "48/48 kurtarıldı" sonucu **çekildi**. Orijinal prototip firmware'e eklenmemiştir.

## Korumacı onarım adayı

Kurtarma ancak ilk EEPROM kaydının *tanık/witness yazma aşamasında* durduğu, Slot A ile Slot B'nin tüm 30'ar baytının boş olduğu ve tanık alanlarının ilk yazma sırasına uygun kısmi durumda olduğu kanıtlanabiliyorsa denenebilir. İki tanık tamamen geçerliyse veya ayar yazımı başladıysa, geçerli bir eski kaydın hasarlanmış olması ihtimali sebebiyle **LOCKED / SERVICE_HOLD** devam eder. Açık fiziksel yerel HOLD, HW Setup ekranı, boş menü, LCD hazır, motor/Remote/USB idle ve `migrationBoot==kLocked` koşulları zorunlu. Kurtarma başarılı olursa yalnız `kLegacyNeedsReview` durumuna geçilir, motor otomatik çalışmaz.

## Kaynak testleri

- Güncel kaynak: `main a55d30b4637f6522946c15c92e6e08f043e878f6`, v1.06 .ino blob `55b19a8b1017bf1924dc568439f920ec9c90064f`; v1.05 ve v1.04 değişmedi.
- İlk karşı-örnek: orijinal adayda commit marker hasarlı durumda **NEGATIVE_FAIL_REPRODUCED**, geçerli veri silinmesi tespit edildi.
- Yeni tutucu başlık + yerel UI kapısı: **g++ C++11 40/40 PASS**, **clang++ C++11 40/40 PASS**, UBSan ve `-Wall -Wextra -Werror -pedantic`.
- İlk kaydetme için 50 kesinti konumu: **48 LOCKED**; bunların **17'si RECOVERABLE**, **31'i SERVICE_HOLD**; diğer **1 LEGACY_REVIEW**, **1 NEW_READY**. GCC ve Clang aynı sonuçları verdi.
- Negatifler: eksik yerel onay, başka menü, LCD arızası, aktif motor, Remote, USB, çalışan HW menüsü, bilinmeyen kayıt, daha önce onaylanmış veya bozulmuş commit kaydı; kurtarma girişimi EEPROM'u değiştirmiyor.
- Testli kurtarma başlığı (Git object, yalnız lokal): `9962766dfa150888d77dbf23857216f52d0a48b5`; UI yardımcı başlığı `036978c0f7bf5631180dcf96dac329a4dbd9d3c9`. Güncel kaynak adayları GitHub `main` referanslarına eklenmemiştir.
- Host UI güvenlik kapısı PASS **yalnız izole kaynak başlıkları + EEPROM mock** kapsamındadır. Tam Arduino setup/loop entegrasyonu / AVR / fiziksel kullanıcı onayı **NOT_RUN**.

## Açık engeller ve karar

- **İlk kesinti sonrası 31 LOCKED** senaryoda servis müdahalesi gerekebilir; onları otomatik açmak veya ayarları silmek güvenli değil.
- İki tanığın tamamen geçerli olduğu kaydın marker'ı bozulmuşsa yazılım geçmişteki işlemle yarım kaydı ayırt edemez. Belirsizlikte LOCKED korunur.
- Daha fazla EEPROM sıfırlama yöntemi ve motor izni otomatik eklenmeyecek. Onarım yalnız bağımsız doğrulama + cihazda açık onay/görünür UI + 8/16 MHz gerçek AVR kanıtıyla değerlendirilir.
- RAM-only adayı `957f963c1d4f59a04f18a896e9f6c92890061a49`: `String inputString` ve `reserve(200)` iki satırının silinmesi; **AVR_NOT_RUN**, `main` firmware değişmedi.

**Durum:** NEGATIVE_FAIL_REPRODUCED / CONSERVATIVE_HOST_CPP_PASS / FULL_FIRMWARE_NOT_RUN / AVR_NOT_RUN / PHYSICAL_NOT_RUN / RELEASE_BLOCKED. Bu not veya eski PASS kayıtları kart üzerine yükleme onayı değildir.
