#include <SPI.h>
#include <TFT_eSPI.h>
#include <SD.h>

// Инициализация дисплея (используем настройки для вашей платы)
TFT_eSPI tft = TFT_eSPI();

// Для SD-карты используем стандартные пины
const int SD_CS = 5;  // Для вашей платы может быть другой пин!
File myFile;

void setup() {
    Serial.begin(115200);
    
    // Инициализация дисплея
    tft.begin();
    tft.setRotation(3);  // Настройка ориентации (может потребовать изменения)
    tft.fillScreen(TFT_BLACK);
    
    // Инициализация SD-карты
    if (!SD.begin(SD_CS)) {
        tft.println("Ошибка инициализации SD-карты!");
        return;
    }
    
    // Проверяем наличие файла
    if (!SD.exists("text.txt")) {
        createDefaultFile();
    }
}

void loop() {
    displayTextFromSD();
    delay(5000);  // Обновление каждые 5 секунд
}

void displayTextFromSD() {
    myFile = SD.open("text.txt");
    if (myFile) {
        tft.fillScreen(TFT_BLACK);  // Очищаем экран
        tft.setTextColor(TFT_WHITE);
        tft.setCursor(10, 10);
        tft.setTextSize(2);
        
        while(myFile.available()) {
            String line = myFile.readStringUntil('\n');
            tft.println(line);
        }
        
        myFile.close();
    }
}

void createDefaultFile() {
    myFile = SD.open("text.txt", FILE_WRITE);
    if (myFile) {
        myFile.println("Добро пожаловать!");
        myFile.println("Это пример текста");
        myFile.println("с SD-карты");
        myFile.close();
    }
}
