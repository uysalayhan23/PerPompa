# Peristaltic Pump Software v1.04

Bu sürüm, v1.03 üzerindeki encoder ve menü kullanımını sağlamlaştırır. Dose, Pump, Cal., Remote Pins ve RNG çalışma fonksiyonlarının içeriği değiştirilmedi.

## Encoder ve menü düzenlemeleri

- Encoder hızlandırması sayı ayarlarında da kapalıdır; kabul edilen her dönüş adımı bir menü/değer adımıdır.
- Dönüşler en az 5 ms aralıkla kabul edilir. Bir adımın hemen ardından gelen ters yöndeki 25 ms içindeki hareket, kontak sekmesi olabileceği için yok sayılır.
- Encoder düğmesine basılırken oluşan dönüşler ve düğme olayı çevresindeki hareketler menüye uygulanmaz.
- Motor çalışırken encoder dönüşleri okunup tüketilir; stop sonrasında bekleyen dönüş menüyü kendiliğinden kaydırmaz.
- Basılı tutma ve çift basış, yalnızca sayısal ayarın içine girilmişken değeri değiştirir. Basılı tutma tek kez işlenir; çift basış üst sınırı geçemez.
- Ana menüde üst ve alt satır her adımda birlikte hesaplanır; uçtan başa dönüşte komşu menü satırları tutarlı kalır.
- LCD her menü hareketinde temizlenmez. Yalnız değişen karakterler yazılır; titreme ve gereksiz yazma azalır.
- Sayı gösterimi için tampon büyütüldü. EEPROM'dan gelen geçersiz menü değerleri başlangıç değerine döndürülür.
- Cihaz açılırken `uL/min` hız biriminin ondalık gösterimi doğru kurulur.

## Sınırlar ve doğrulama

Bu sürümde encoderin bir mekanik tıkı için `ENCODER_STEPS_PER_NOTCH = 4` korunmuştur. Fiziksel encoderin tık/elektriksel geçiş oranı farklıysa bu sabitin gerçek parça ile ayarlanması gerekebilir. Yazılım, kopuk bağlantıyı veya sınırsız elektriksel gürültüyü tek başına düzeltemez.

Kart ve Arduino derleyicisi bu çalışma ortamında bulunmadığından fiziksel çalışma ve gerçek Arduino derlemesi henüz doğrulanmadı. Yerel C++ sözdizimi kontrolü ve simüle menü denemeleri geçti. Statik karşılaştırmada Dose, Pump, Remote Pins, RNG ve hesap fonksiyonlarının v1.03 ile aynı kaldığı da doğrulandı.

Kart hazır olduğunda şu kısa denemeler yapılmalıdır: her tıkta tek menü ilerlemesi; hızlı çevirme; yönü aniden ters çevirme; encoder'a basarken çevirme; sayısal ayarda basılı tutma/çift basış; Pump çalışırken çevirip stop verme; Hold/Pulse uzaktan start/stop.
