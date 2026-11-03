# PerPompa P0-04 — AVR SPH/SPL stack ölçüm düzeltmesi (2026-11-03)

**Karar:** `MEASUREMENT_BUG_CONFIRMED` · `ATOMIC_SP_BOUNDED_2/2_PASS` · `ALL_PATHS_ISR_STACK_REVIEW` · `PHYSICAL_NOT_RUN` · `RELEASE_BLOCKED`.

## Somut bulgu: kaynak firmware değil, gözlem aracının SP okuması

Geçici birleşik Flash-LCD v1.06 QA kaynağında, `perpompa_p004::Store<EEPROMClass>::decode()` fonksiyonunun **30 baytlık yerel alan ayıran** AVR prologunda, iki baytlı stack pointer'ın yüksek baytı (SPH) yazıldıktan sonra alçak baytı (SPL) henüz güncellenmemiştir. Önceki simAVR aracı her işlemden sonra SPH/SPL'yi birlikte okuyup **geçici SPH+yeni, SPL+eski** birleşimini gerçek stack tepe değeri sanmıştır.

- Optimize 16 MHz disassembly: `1c8e sbiw r28,0x1e`, `1c92 cli`, `1c94 out 0x3e,r29` (SPH), `1c96 out 0x3f,r0` (SREG), `1c98 out 0x3d,r28` (SPL). Eski yüksek-water PC `0x1c96`, yani tam SPH ve SPL yazmaları arası.
- Optimize 8 MHz: `1c8a sbiw r28,0x1e`, `1c8e cli`, `1c90 out 0x3e,r29`, `1c92 out 0x3f,r0`, `1c94 out 0x3d,r28`. Eski yüksek-water PC `0x1c92`, yine aradadır.
- Bu kayıt anındaki **256 B anlık düşüş, yeni gerçek 256 B stack ayırma değildir**. Yarım güncellenmiş bir 16 bit yazma sırasının yanlış örneklenmesidir.
- [İlk PC kökeni Actions #41665520374](https://github.com/uysalayhan23/PerPompa/actions/runs/41665520374): tam SHA kaynak 16/8 MHz AVR, 2/2 optimize PC sembol ataması PASS, 8/8 kasıtlı hatalı iz reddi.

## Düzeltilmiş ve bağımsız gerçek AVR ölçümü

Yalnız **QA ölçüm aracında** SPH→SPL yazma dizisi tanınarak ara ölçüm dışlandı. Aynı optimize AVR firmware `.ino` SHA256 `5c7009477ba56cb70850f22ece9860fcb222bd4c0a0c3a10622c21b66c79bfa5` kullanıldı. [İkinci Actions #41666679685](https://github.com/uysalayhan23/PerPompa/actions/runs/41666679685) gerçek AVR 16/8 MHz 2/2 derleme PASS, atomic SP pozitif 2/2 PASS ve kasıtlı yarım/eksik/uyumsuz SP izleriyle 10/10 negatif yakalama.

| Tekrar hesaplanan sınırlandırılmış açılış/LCD/enkoder/Remote simAVR ölçümü | 16 MHz | 8 MHz |
|---|---:|---:|
| Optimize AVR Flash (B) | 28.010 | 28.006 |
| Statik SRAM (B) | 1.326 | 1.326 |
| Statik ayrılmamış SRAM (B) | 722 | 722 |
| **Eski yanlış SP, SPH yazımı arası** | `0x0713` | `0x0713` |
| **Güncellenmesi tamamlanmış en düşük SP** | **`0x07C5`** | **`0x07BF`** |
| **Tamamlanmış SP'ye göre gözlenen stack derinliği** | **314 B** | **320 B** |
| Eski yanlış stack derinliği | 492 B | 492 B |
| **Tamamlanmış SP'ye göre gözlenen heap açıklığı** | **383 B** | **377 B** |
| Eski yanlış açıklık | 205 B | 205 B |
| Simülasyonda OUT SPH / OUT SPL gözlemleri | 36 / 36 | 30 / 30 |
| Yarım SP ara örnekleri (hesaptan dışlandı) | 71 | 59 |
| Eşleşmemiş SP güncellemesi | 0 | 0 |
| Tamamlanmış minimumu kaydeden PC | `0x6A72` | `0x03A0` |
| PC'nin en yakın optimize sembolü | `fputc+8` | `turnOffPWM+0` |
| Simülatör uyaran aşaması | 0 (açılış) | 0 (açılış) |

**Dikkat:** Sembol, SP'nin yeni minimuma indiği anda yürütülen fonksiyondur; bütün çağrı zincirinin veya Timer1 ISR katkısının kesin nedeni değildir. Bu sınırlı test gerçek EEPROM yazma kesintisini, motor dozajını, tüm UART akışlarını ve farklı ISR iç içe geçmelerini kapsamaz.

## Eski raporların düzeltilmiş yorumu

- Önceki `95 B` ve `205 B` **her işlemde okunan ham SP** örnekleridir; 16 bit SP yazımı yarımken örneklenmiş olabilirler. Bunları **gerçek minimum çalışma payı** veya gerçek A/B stack kazanımı olarak kabul etmeyin.
- Flash-LCD değişikliğindeki gerçek **110 B statik SRAM kazancı** ve 8/16 MHz AVR derleme PASS geçerlidir; yalnız eski ham SP üzerinden ilan edilen mutlak stack payı geçerli kabul kanıtı değildir.
- EEPROM streaming adayı (`200→201 B` ham), menü tamponu (`205→196 B` ham) ve satır tamponu (`205→189 B` ham) karşılaştırmalarındaki farklar **tamamlanmış SP tepe farkı olarak kanıtlanmamıştır**. Bu QA adaylarının `DO_NOT_PROMOTE` kararları korunur; ama önceki ham SP farkını gerçek regresyon olarak yorumlamayın ve eski testleri otomatik tekrarlamayın.
- İlk ve ikinci Actions çalışmaları yalnız **ölçüm aracının tanısı ve düzeltilmesi** içindir; üretim firmware'inde hiçbir değişiklik yapılmamıştır.

## Kabul sınırı ve korunacak kurallar

Yeni QA dosyaları: `qa/p004/stack_origin_probe.c` (önceki ham iz), `qa/p004/stack_atomic_probe.c` (SPH/SPL tamamlanma izlemesi), `qa/p004/trace_pc_origin_review.py` (2/2 + 8/8), `qa/p004/check_atomic_sp_review.py` (2/2 + 10/10), ve SHA-kilitli tek çalıştırma betikleri. Tek kullanımlık Actions workflow'u test sonrasında `main`den kaldırılır.

Üretim v1.06 `.ino` Git blob `957f963c1d4f59a04f18a896e9f6c92890061a49` değişmedi; v1.05 manifestosu, v1.04 arşivi ve saatlik P0-04 otomasyonu korunur. **Tüm yolların ISR/heap/stack tepe güvenliği: REVIEW. Gerçek cihaz: NOT_RUN. Ticari sürüm: RELEASE_BLOCKED.** Yeni ölçümden hareketle firmware otomatik olarak onaylanmaz.
