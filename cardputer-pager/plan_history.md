# Plan History: FIX: Add UDP beacon listener to receiver + rebuild + flash

## Step 01-build-receiver (ok, 14.4s)
Сборка receiver (Cardputer) с UDP-слушателем

## Step 02-build-sender (ok, 15.0s)
Сборка sender (M5StickC) — фикс forward declaration

## Step 03-flash-receiver (ok, 22.1s)
Прошивка Cardputer на COM3 с UDP-фиксом

## Step 04-flash-sender (ok, 26.9s)
Прошивка M5StickC на COM11 с диагностикой

## Step 05-verify (ok, 0.3s)
Проверка firmware.bin — оба свежие
