#include <M5Cardputer.h>

#define COLS 20
#define ROWS 30
#define COLW 16

int colY[COLS];
int colDelay[COLS];
char colChar[COLS];
unsigned long colTimer[COLS];

char randHex() {
  const char h[] = "0123456789ABCDEF";
  return h[random(16)];
}

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg);
  M5Cardputer.Lcd.fillScreen(TFT_BLACK);
  M5Cardputer.Lcd.setTextColor(TFT_GREEN);
  M5Cardputer.Lcd.setTextSize(1);

  for (int i = 0; i < COLS; i++) {
    colY[i] = random(ROWS);
    colDelay[i] = random(40, 151);
    colChar[i] = randHex();
    colTimer[i] = 0;
    M5Cardputer.Lcd.setCursor(i * COLW, colY[i] * 8);
    M5Cardputer.Lcd.print(colChar[i]);
  }
}

void loop() {
  M5Cardputer.update();

  if (M5Cardputer.Keyboard.isPressed() > 0) {
    M5Cardputer.Lcd.fillScreen(TFT_BLACK);
    for (int i = 0; i < COLS; i++) {
      colY[i] = random(ROWS);
      colDelay[i] = random(40, 151);
      colChar[i] = randHex();
      M5Cardputer.Lcd.setCursor(i * COLW, colY[i] * 8);
      M5Cardputer.Lcd.print(colChar[i]);
    }
    delay(100);
    return;
  }

  unsigned long now = millis();

  for (int i = 0; i < COLS; i++) {
    if (now - colTimer[i] >= colDelay[i]) {
      M5Cardputer.Lcd.setTextColor(TFT_BLACK);
      M5Cardputer.Lcd.setCursor(i * COLW, colY[i] * 8);
      M5Cardputer.Lcd.print(colChar[i]);

      colY[i]++;
      if (colY[i] >= ROWS) colY[i] = 0;

      colChar[i] = randHex();
      M5Cardputer.Lcd.setTextColor(TFT_GREEN);
      M5Cardputer.Lcd.setCursor(i * COLW, colY[i] * 8);
      M5Cardputer.Lcd.print(colChar[i]);

      colTimer[i] = now;
    }
  }

  delay(5);
}
