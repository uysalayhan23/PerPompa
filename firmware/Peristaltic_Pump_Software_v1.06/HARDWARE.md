# PerPompa v1.06 — I²C LCD bağlantıları

| İşlev | Arduino Pro Mini |
|---|---|
| LCD PCF8574 SDA | A4 |
| LCD PCF8574 SCL | A5 |
| Enkoder | D2/D3/D4 |
| Remote giriş | D5 |
| Motor DIR | D6 |
| Motor STEP | D7 |
| RNG komutlu çalışma göstergesi | A0 |
| Mevcut USB / seri programlama ve v1.05 USB işlevi | D0/D1 |

Eski paralel LCD D8–D13 sinyalleri kullanılmaz. 5 V/16 MHz ve 3,3 V/8 MHz Pro Mini hedeflenir. PCF8574 backpack beslemesi ve SDA/SCL pull-up gerilimi anakartla uyumlu olmalı; gerekirse uygun seviye dönüştürücü kullanılmalıdır. Fiziksel devre kabulü henüz NOT_RUN; RNG gerçek sıvı akış ölçümü değildir.
