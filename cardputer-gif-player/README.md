# GIF-плеер для M5Cardputer

GIF-плеер для M5Cardputer с поддержкой настоящего декодирования GIF (библиотека AnimatedGIF).

## Требования

- PlatformIO
- M5Cardputer (аппаратно)
- SD-карта (FAT32)

## Сборка

```
cd projects/cardputer-gif-player
pio run
```

## Прошивка

- Подключить Cardputer по USB
- `pio run --target upload`

## Подготовка SD-карты

1. Отформатировать в FAT32
2. Скопировать .gif файлы в корень карты
3. Вставить в Cardputer

## Управление

- W — вверх по списку
- S — вниз по списку
- ENTER/SPACE — выбрать и проиграть
- BACKSPACE — выйти из просмотра в меню

## Зависимости

- M5Cardputer (M5Stack)
- AnimatedGIF (bitbank2)
