// Умный дом на Arduino
// Контролирует температуру, освещение, движение, управление реле и дисплеем
// Использует датчики температуры, датчик движения, светодиоды, кнопки, LCD дисплей

#include <LiquidCrystal.h>      // библиотека для LCD 16x2
#include <DHT.h>               // библиотека для датчика температуры и влажности DHT

// Пины подключения
#define DHTPIN 2               // датчик DHT
#define DHTTYPE DHT22          // тип датчика DHT22

#define MOTION_SENSOR_PIN 3    // датчик движения PIR
#define RELAY_PIN 4            // реле для управления лампой
#define LED_PIN 13             // встроенный светодиод

#define BUTTON_PIN 5           // кнопка управления

  // Настройки дисплея
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// Создаем объект DHT
DHT dht(DHTPIN, DHTTYPE);

// Переменные состояния
bool relayState = false;
bool motionDetected = false;
unsigned long lastMotionTime = 0;
const unsigned long motionTimeout = 30000; // выключить свет через 30 секунд отсутствия движения

// Переменная для состояния кнопки
bool buttonState = false;
bool lastButtonState = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// Переменные для хранения температуры и влажности
float temperature = 0.0;
float humidity = 0.0;

// Переменные для меню и настроек
int menuIndex = 0;
const int menuItemsCount = 3;

// Таймеры для обновления экрана и сенсоров
unsigned long lastSensorRead = 0;
const unsigned long sensorInterval = 2000;

unsigned long lastDisplayUpdate = 0;
const unsigned long displayInterval = 1000;

// Функции прототипы
void readSensors();
void updateDisplay();
void checkMotion();
void handleButton();
void toggleRelay();
void showMenu();
void menuNavigation();
void controlRelayWithMenu();

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // выключаем реле
  pinMode(MOTION_SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // кнопка с подтяжкой к питанию

  Serial.begin(9600);
  dht.begin();
  lcd.begin(16, 2);

  lcd.print("Умный дом");
  delay(2000);
  lcd.clear();
}

void loop() {
  unsigned long currentMillis = millis();

  handleButton();

  if (currentMillis - lastSensorRead >= sensorInterval) {
    lastSensorRead = currentMillis;
    readSensors();
  }

  if (currentMillis - lastDisplayUpdate >= displayInterval) {
    lastDisplayUpdate = currentMillis;
    updateDisplay();
  }

  checkMotion();

  // В режиме меню управление реле через меню
  if(menuIndex > 0) {
    controlRelayWithMenu();
  }
}

// Чтение датчиков температуры и влажности
void readSensors() {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Ошибка чтения с DHT");
  } else {
    Serial.print("Темп: "); Serial.print(temperature);
    Serial.print(" °C Влаж:"); Serial.print(humidity);
    Serial.println("%");
  }
}

// Обновление информации на LCD дисплее
void updateDisplay() {
  lcd.clear();
  
  if(menuIndex == 0) { // главный экран
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(temperature, 1);
    lcd.print((char)223);
    lcd.print("C H:");
    lcd.print(humidity, 0);
    lcd.print("%");

    lcd.setCursor(0, 1);
    lcd.print("Движен: ");
    lcd.print(motionDetected ? "Да" : "Нет");
  } else {
    showMenu();
  }
}

// Обработка движения
void checkMotion() {
  int val = digitalRead(MOTION_SENSOR_PIN);
  if(val == HIGH) {
    motionDetected = true;
    lastMotionTime = millis();
    digitalWrite(LED_PIN, HIGH);
    if(!relayState) {
      relayState = true;
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("Движение - Свет вкл");
    }
  } else {
    // Если прошло более motionTimeout, выключаем свет и светодиод
    if(motionDetected && (millis() - lastMotionTime > motionTimeout)) {
      motionDetected = false;
      digitalWrite(LED_PIN, LOW);
      if(relayState) {
        relayState = false;
        digitalWrite(RELAY_PIN, LOW);
        Serial.println("Нет движения - Свет выкл");
      }
    }
  }
}

// Обработка нажатия кнопки с антидребезгом и меню
void handleButton() {
  int reading = digitalRead(BUTTON_PIN);

  if(reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if((millis() - lastDebounceTime) > debounceDelay) {
    if(reading != buttonState) {
      buttonState = reading;
      if(buttonState == LOW) { // кнопка нажата (замыкается на землю)
        menuNavigation();
      }
    }
  }

  lastButtonState = reading;
}

// Навигация по меню по нажатию кнопки
void menuNavigation() {
  menuIndex++;
  if(menuIndex > menuItemsCount) {
    menuIndex = 0;
  }
  lcd.clear();
}

// Показ меню настроек
void showMenu() {
  switch(menuIndex) {
    case 1:
      lcd.setCursor(0, 0);
      lcd.print(">Свет:");
      lcd.setCursor(0, 1);
      lcd.print(relayState ? "Включен " : "Выключен");
      break;
    case 2:
      lcd.setCursor(0, 0);
      lcd.print(">Темп:");
      lcd.setCursor(0, 1);
      lcd.print(temperature, 1);
      lcd.print((char)223);
      lcd.print("C");
      break;
    case 3:
      lcd.setCursor(0, 0);
      lcd.print(">Влажность:");
      lcd.setCursor(0, 1);
      lcd.print(humidity, 0);
      lcd.print("%");
      break;
    default:
      lcd.print("Ошибка меню");
      break;
  }
}

// Управление реле из меню при положении меню 1
void controlRelayWithMenu() {
  // Если нажать кнопку долго — переключить реле
  // Для примера переключаем на каждое нажатие (не удержание)
  static unsigned long lastPressTime = 0;
  if(buttonState == LOW && millis() - lastPressTime > 500) {
    if(menuIndex == 1) {
      toggleRelay();
      lastPressTime = millis();
    }
  }
}

// Переключение реле
void toggleRelay(){
  relayState = !relayState;
  digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
  Serial.print("Реле ");
  Serial.println(relayState ? "Включено" : "Выключено");
}