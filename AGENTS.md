# PerPompa — Repository-wide agent guardrails

Bu deponun **tek aktif firmware geliştirme sürümü v1.05**. Her görevde önce aşağıdaki canlı GitHub dosyalarını oku:

1. `firmware/Peristaltic_Pump_Software_v1.05/REVISION_MANIFEST.md` (**yetkili teknik kabul politikası**)
2. `firmware/Peristaltic_Pump_Software_v1.05/NEXT_TASK.md` (**tek aktif görev**)
3. `firmware/Peristaltic_Pump_Software_v1.05/TEST_MATRIX.md` (**regresyon kanıtları**)
4. Aktif `Peristaltic_Pump_Software_v1.05.ino`, canlı `main` HEAD, arşivlenen v1.04 SHA.

**KESİN KİLİTLER:** `firmware/archive/` değiştirme. No force-push, no GitHub Actions, no Work/Codex (aksi açıkça yetkilendirilmedikçe). `[skip ci]`, beklenen HEAD kontrolü, uzak commit/blob SHA teyidi. Kanıtsız PASS/COMPLETE yazma; host/model testi AVR/hardware PASS değildir. Kaynak değişikliği yalnız kanıtlı P0-01 ve sonraki tek aktif manifest görevi için, en küçük yama ve pozitif-negatif test geçildikten sonra.

Yeni sohbet, görev veya ajan başlaması bu kuralları kaldırmaz. Manifest/görev dosyaları okunamıyorsa **kod yazmayı durdur, READ_BLOCKED bildir.** Kullanıcının açık talebi olmadan kapsam genişletme.
