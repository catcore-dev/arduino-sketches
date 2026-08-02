# arduino-sketches

**Мои проекты на Arduino и ESP32. TFT-дисплеи, датчики, реле, умный дом.**

---

Начинал с мигания светодиодом, как и все. Потом захотелось чего-то полезного — чтобы экран показывал текст с SD-карты, датчик температуры управлял реле, дисплей реагировал на кнопки. Собрал пару работающих штук.

## Проекты

### 1. TFT SD Reader

Читает текстовый файл с SD-карты и выводит на TFT-дисплей. Обновляется каждые 5 секунд.

**Железо:** TFT_eSPI-совместимый дисплей, SPI, SD-карта

**Файлы:** `tft-sd-reader/tft-sd-reader.ino`

### 2. Smart Home Sensor

Датчик температуры и влажности (DHT22), датчик движения (PIR), реле для управления светом. Всё выводится на LCD 16×2 с меню навигации по кнопке. Если движения нет 30 секунд — свет выключается.

**Железо:** Arduino, DHT22, PIR, реле, LCD 16×2, кнопка

**Файлы:** `smart-home-sensor/smart-home-sensor.ino`

## Структура

```
├── tft-sd-reader/
│   ├── tft-sd-reader.ino
│   └── README.md         # схема подключения, список компонентов
├── smart-home-sensor/
│   ├── smart-home-sensor.ino
│   └── README.md
├── assets/                # фото и схемы
└── README.md              # этот файл
```

## Почему так мало

Я больше занимаюсь серверной частью и DevOps. Arduino для меня — хобби. Но эти скетчи реально работают, проверены на железе.

---

## English

Arduino and ESP32 hobby projects. TFT display with SD card reader, smart home sensor with DHT22 + PIR + relay + LCD menu. All tested on real hardware.

---

MIT License.
