# Remote Pins ve RNG: devre bağlantı notları

**İlgili yazılım:** `Peristaltic_Pump_Software_v1.03.ino`  
**Kart:** Arduino Pro Mini  
**Ürün dış bağlantıları:** `REMOTE` için iki uçlu kuru kontak girişi; `RNG` için iki uçlu, normalde açık (NO) kuru kontak çıkışı.

Bu belge devre tasarımı ve montajı içindir. Kullanıcı kılavuzunda kart pinleri, bobin sürücüsü ve koruma diyodu anlatılmaz. Bağlantıların çizimi ayrıca `Remote_Pins_RNG_Devre_Semasi.pdf` dosyasındadır.

## 1. Yazılımdaki pinler

| Görev | Yazılım tanımı | Pro Mini bağlantısı | Elektriksel işlev |
|---|---|---|---|
| Uzaktan start/stop girişi | `REMOTE_PIN` | **D5** | `INPUT_PULLUP`: açık kontakta HIGH, D5 ile GND kapanınca LOW |
| Çalışma durum sinyali | `RNG_PIN` | **A0** | Dijital çıkış: motor adım komutu sırasında HIGH, durunca LOW |

**Önemli ayrım:** A0 pininin kendisi kuru kontak değildir. A0, ürün içindeki röle sürücüsüne verilen mantık sinyalidir. Ürün dışındaki `RNG` uçları, iç rölenin **COM ve NO kontakları** olmalıdır. Röle takılmadan kullanıcı kılavuzundaki “RNG kuru kontak çıkışı” tanımı fiziksel ürün için geçerli olmaz.

## 2. Remote Pins girişini oluşturma

1. Ürün dışına `REMOTE-1` ve `REMOTE-2` olarak iki klemens çıkarın.
2. `REMOTE-1` ucunu Pro Mini **D5** pinine bağlayın.
3. `REMOTE-2` ucunu kartın **GND** hattına bağlayın.
4. PLC'nin **potansiyelsiz röle kontağını** bu iki uç arasına bağlayın. Kontak kapanınca D5, GND'ye iner ve yazılım bunu uzaktan kumanda olayı olarak okur.

Yazılım D5 için iç pull-up direncini açar; temel kuru kontak bağlantısına ayrıca pull-up direnci gerekmez. Yazılım kontak sıçramasını **30 ms** boyunca filtreler. PLC komutunda açık ve kapalı durumları bu süreden uzun ve kararlı tutun.

**PLC'nin 24 V transistor çıkışını D5'e doğrudan bağlamayın.** Böyle bir çıkış kullanılacaksa uygun izolasyonlu bir arayüz tasarlayın; arayüzün ürün tarafında D5 ile GND'yi anahtarlayan kuru kontak veya eşdeğer izole anahtar bulunmalıdır. Kablo uzunluğu ve elektriksel gürültü yüksekse giriş korumasını ve izolasyonu kurulum koşullarına göre ayrıca değerlendirin.

### Güç açılışı davranışı

- **Hold** kayıtlıysa ve cihaz açılırken kontak kapalıysa yazılım pompayı hemen başlatabilir. Bu, kodda açıkça tanımlıdır.
- **Pulse** kayıtlıysa cihaz açılırken zaten kapalı olan kontak yeni darbe sayılmaz. Yeni start için önce kontağı açıp yeniden kapatın.

## 3. RNG için iç röle devresi

Hedef, A0 aktif HIGH olduğunda 5 V bobinli röleyi çektirmek; ürün dışına yalnız **NO kuru kontak** vermektir.

1. Kart **A0** çıkışını bir **100–220 Ω** seri direnç üzerinden, uygun lojik seviyeli N-kanal MOSFET'in gate ucuna bağlayın.
2. Gate ile GND arasına yaklaşık **100 kΩ pull-down** direnç koyun. Kart açılırken gate'in boşta kalmasını önler.
3. MOSFET source ucunu iç devre GND hattına bağlayın.
4. Röle bobininin bir ucunu düzenlenmiş **+5 V** beslemeye, diğer ucunu MOSFET drain ucuna bağlayın.
5. Bobine paralel koruma diyodu koyun: **katot (çizgili uç) +5 V tarafına**, anot MOSFET drain tarafına bağlanır. Diyodun akım/gerilim değerini bobine göre seçin.
6. Rölenin **COM ve NO** kontaklarını ürün dışındaki `RNG-1` ve `RNG-2` klemenslerine götürün. NC kullanılmaz.
7. İç 5 V besleme GND hattı ile Pro Mini GND hattını ortaklayın. Rölenin dış COM/NO kontaklarını bu GND hattına bağlamayın; dış kontak potansiyelsiz kalmalıdır.

**A0'a çıplak bobin bağlamayın.** 5 V, bobinin çalışma gerilimidir; A0'ın beslemesi veya röleyi doğrudan sürebileceği anlamına gelmez. Pro Mini'nin 3,3 V sürümünde A0 HIGH yaklaşık 3,3 V; 5 V sürümünde yaklaşık 5 V'tur. MOSFET'in ilgili gate geriliminde, seçtiğiniz rölenin bobin akımını güvenle anahtarlayabildiğini veri sayfasından doğrulayın. 5 V kaynağının bobin akımı için yeterli payı olmalıdır.

### Dış RNG kontağının anlamı

| Yazılım durumu | A0 | İç röle | Dış `RNG-1`–`RNG-2` |
|---|---|---|---|
| Motor adım komutları üretiliyor | HIGH | Çekili | Kapalı |
| Komutlar durdu | LOW | Bırakmış | Açık |

RNG, motorun fiziksel olarak döndüğünü ya da sıvının aktığını **ölçmez**. PLC'ye verilen bilgi yazılımın çalışma komutudur. PLC girişinin besleme ve algılama bağlantısını PLC üreticisinin giriş şemasına göre kurun; röle kontak gerilim/akım sınırını aşmayın.

## 4. Montaj sonrası kontrol

1. Cihaz enerjisizken `REMOTE` uçlarıyla `RNG` uçları arasında istenmeyen bağlantı bulunmadığını ölçün.
2. `REMOTE` kontağı açıkken D5'in HIGH, kapalıyken LOW okunduğunu doğrulayın. PLC'nin 24 V hattının D5'e ulaşmadığını ölçün.
3. Röleyi bağlamadan önce test sırasında A0'ın dururken LOW, motor adım komutları sırasında aktif HIGH olduğunu doğrulayın.
4. Röleyi sürücüye bağlayın. Stop durumunda `RNG-1`–`RNG-2` açık, çalışma sırasında kapalı olmalıdır.
5. Hold ve Pulse davranışlarını Dose, Pump ve Cal. modlarında ayrı ayrı deneyin. Hold'da güç açılırken kapalı kontağın start verebileceğini de kontrollü olarak sınayın.

## Dayanaklar

- [Arduino `INPUT_PULLUP` açıklaması](https://docs.arduino.cc/language-reference/en/variables/constants/inputOutputPullup/): açık/kapalı kontak seviyeleri ve giriş sınırları.
- [Arduino `digitalWrite` açıklaması](https://docs.arduino.cc/language-reference/en/functions/digital-io/digitalwrite/): A0'ın dijital çıkış olarak kullanımı ve 3,3/5 V HIGH seviyesi.
- [Arduino röle sürücü şeması](https://docs.arduino.cc/resources/schematics/TSX00003-schematics.pdf): transistor/MOSFET sürücüsü ve bobin koruma diyodu örneği.
