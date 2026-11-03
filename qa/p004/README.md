# P0-04 — Birleşik v1.06 kurtarma adayı ve stack/heap QA kaynakları

**Durum: REVIEW ONLY · NOT FOR PRODUCTION · RELEASE_BLOCKED**

Bu klasör, 3 Kasım 2026 tarihli P0-04 çalışmasının **üretim firmware'ine aktarılmamış** geçici kodunu ve simülasyon test aracını kaynaklarıyla saklar.

## İçerik

- `read_only_candidate.tar.gz`: v1.06 geçici MergedGates / EEPROM recovery adayı; tam `.ino` ve 7 header dosyası. SHA256 `b9d04926776ce3703043c507fa804c6cc6ff6ca494f84df6b6926b37ff3d38f2`.
- Arşivin `Peristaltic_Pump_Software_v1.06/Peristaltic_Pump_Software_v1.06.ino` dosyası SHA256: `e0d852757f74fedc4c00dd9da05b9cf02bf660c72bf7dcaa4afc52c8a7710e21`.
- `stack_probe.c`: simAVR ile AVR SRAM/stack/heap **sınırlı izleme** için kullanılan test aracı. Fiziksel pompaya yükleme, seri port programlama veya dağıtım yapmaz.
- `verify-review.sh`: arşiv bütünlüğünü ve çıkarılmış `.ino` SHA256 değerini kontrol eder.

## Kanıt / kabul

- [AVR 8/16 MHz Actions kabul raporu](../../firmware/Peristaltic_Pump_Software_v1.06/P004_ACTIONS_AVR_REVIEW_20261103.md)
- [Stack/heap 16/8 MHz simülasyon ve sınır raporu](../../firmware/Peristaltic_Pump_Software_v1.06/P004_STACK_HEAP_ACTIONS_REVIEW_20261103.md)
- Başarılı yoğun simAVR koşusu: https://github.com/uysalayhan23/PerPompa/actions/runs/41642141415
- Gerçek AVR: 2/2 derleme PASS. Statik SRAM 1436 bayt; 612 bayt boş statik alan. 70 milyon adımlı iki simülasyonda gözlenen en düşük SP `0x0713`, heap üst sınırı `0x06B4`, **en dar stack–heap aralığı 95 bayt**.
- **Bu 95 bayt gerçek cihazın tüm olası işletim koşullarındaki garantili minimum boşluk değildir.** 8 MHz simülasyonunda HW Setup yürütümü gözlenmedi; tam yol/ISR/stack kabulü REVIEW. Fiziksel cihaz NOT_RUN, ticari sürüm RELEASE_BLOCKED.
- EEPROM kurtarma politikası: yalnız 17/48 tutucu aday; 31/48 SERVICE_HOLD. İzin verilmemiş otomatik kurtarma / veri kaybı kabul edilemez.

## Üretim kaynaklarının durumu

Üretim kodu **değiştirilmedi**. GitHub main üzerinde `firmware/Peristaltic_Pump_Software_v1.06/Peristaltic_Pump_Software_v1.06.ino` Git blob SHA `957f963c1d4f59a04f18a896e9f6c92890061a49` korunur. v1.04 arşivi, v1.05 manifestosu ve saatlik P0-04 otomasyonu değiştirilmez.

**Bu klasörün varlığı firmware onayı değildir.** Açık kalan kabul olmadan `read_only_candidate.tar.gz` üretim klasörüne kopyalanmamalı, Arduino'ya yüklenmemeli veya endüstriyel kullanımda kullanılmamalıdır.

Bir kez çalıştırılan GitHub Actions workflow'ları aktif `.github/workflows/` klasöründen kaldırılmıştır; tarihsel çalıştırmalar ve commit'ler kanıt olarak kalır.
