#include <M5StickCPlus2.h>
#include "pager_protocol.h"
#include "pager_client.h"

PagerClient client;
RTC_DATA_ATTR uint8_t lastReceiverIP[4] = {0, 0, 0, 0};

void showDiagnostic(const char* line1, const char* line2, const char* line3, int color);
bool connectAndSendWithDiagnostics(CallType type);
void goToDeepSleep();
void readButtonsAndSend();

// Откуда пришла ошибка — для финального экрана
static const char* failReason = "";

void setup() {
    // POWER_HOLD (GPIO 4): фиксируем чтобы AXP2101 не отключал питание при отключении USB
    pinMode(GPIO_NUM_4, OUTPUT);
    digitalWrite(GPIO_NUM_4, HIGH);
    gpio_hold_en(GPIO_NUM_4);  // сохраняется даже в deep sleep

    M5.begin();

    // Включаем подсветку и громкость
    M5.Lcd.setBrightness(100);
    M5.Speaker.setVolume(255);
    M5.Lcd.setRotation(1);
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextSize(1);

    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

    if (cause == ESP_SLEEP_WAKEUP_EXT0) {
        readButtonsAndSend();
    } else {
        // Первый запуск (power-on, не из deep sleep)
        M5.Lcd.setTextSize(2);
        M5.Lcd.drawString("Pager Ready", 10, 30);
        M5.Lcd.setTextSize(1);
        M5.Lcd.drawString("Press center button", 10, 60);
        M5.Lcd.drawString("to send a call", 10, 75);

        // Ждём 2 сек чтобы пользователь увидел
        delay(2000);
    }

    goToDeepSleep();
}

void loop() {
    goToDeepSleep();
}

void readButtonsAndSend() {
    // Даём время на стабилизацию после пробуждения
    delay(100);

    // Обновляем состояние кнопок (M5Unified сам читает GPIO.in1.data как в строке 2731 M5Unified.cpp)
    M5.update();

    // BtnB (GPIO 39) — обед (нажата сразу при пробуждении = удерживалась вместе с A)
    if (M5.BtnB.isPressed()) {
        showDiagnostic("Dinner call!", "Connecting...", NULL, TFT_YELLOW);
        bool ok = connectAndSendWithDiagnostics(CALL_DINNER);
        if (!ok) {
            showDiagnostic("Error!", "No connection", NULL, TFT_RED);
            M5.Speaker.tone(200, 500);
        } else {
            showDiagnostic("Sent!", "Dinner call sent", NULL, TFT_GREEN);
            M5.Speaker.tone(1000, 200);
        }
        delay(4000);
        return;
    }

    // Кнопка A (GPIO 37) — ждём отпускания для замера длительности удержания
    // После ext0-пробуждения кнопка A всё ещё нажата, ждём когда отпустят
    unsigned long pressStart = millis();
    bool isUrgent = false;
    bool wasReleased = false;

    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Normal call...", 10, 20);
    M5.Lcd.drawString("(hold for urgent)", 10, 40);

    while (true) {
        M5.update();  // обновляем состояние кнопок
        bool btnA_held = M5.BtnA.isPressed();

        if (!btnA_held && !wasReleased) {
            // Первый раз когда кнопку отпустили — фиксируем
            wasReleased = true;
        }

        unsigned long held = millis() - pressStart;

        // Проверка на urgent (>2 сек удержания и кнопка всё ещё нажата)
        if (held > 2000 && btnA_held && !isUrgent) {
            isUrgent = true;
            M5.Lcd.fillScreen(TFT_RED);
            M5.Lcd.setTextColor(TFT_WHITE, TFT_RED);
            M5.Lcd.setTextSize(2);
            M5.Lcd.drawString("URGENT", 10, 10);
            M5.Lcd.setTextSize(1);
            M5.Lcd.drawString("Hold to confirm...", 10, 40);
        }

        if (isUrgent && btnA_held) {
            int progress = min((int)((millis() - pressStart - 2000) * 100 / 1000), 100);
            M5.Lcd.drawRect(10, 60, 100, 15, TFT_WHITE);
            M5.Lcd.fillRect(10, 60, progress, 15, TFT_WHITE);
            M5.Lcd.setCursor(10, 85);
            M5.Lcd.printf("%d%%", progress);
        }

        // Выходим когда кнопка отпущена (быстрое нажатие) ИЛИ urgent + отпущена
        if (wasReleased && (!isUrgent || !btnA_held)) break;
        // Лимит 6 секунд чтобы не зависнуть
        if (held > 6000) break;

        delay(10);
    }

    CallType callType = isUrgent ? CALL_URGENT : CALL_NORMAL;

    // --- ПОШАГОВАЯ ДИАГНОСТИКА ---
    bool ok = connectAndSendWithDiagnostics(callType);

    if (ok) {
        M5.Lcd.fillScreen(TFT_GREEN);
        M5.Lcd.setTextColor(TFT_WHITE, TFT_GREEN);
        M5.Lcd.setTextSize(2);
        M5.Lcd.drawString("Sent!", 10, 40);
        M5.Speaker.tone(1000, 200);
    } else {
        M5.Lcd.fillScreen(TFT_RED);
        M5.Lcd.setTextColor(TFT_WHITE, TFT_RED);
        M5.Lcd.setTextSize(2);
        M5.Lcd.drawString("Error!", 10, 20);
        M5.Lcd.setTextSize(1);
        M5.Lcd.drawString(failReason, 10, 50);
        M5.Speaker.tone(200, 500);
    }

    delay(5000);  // Даём 5 секунд на прочтение ошибки
}

// --- ПОШАГОВАЯ ОТПРАВКА С ДИАГНОСТИКОЙ НА ЭКРАНЕ ---
bool connectAndSendWithDiagnostics(CallType type) {
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 10);

    // Шаг 1: WiFi
    M5.Lcd.println("1) WiFi ANV...");
    WiFi.mode(WIFI_STA);
    WiFi.begin("ANV", "9115554644");

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        attempts++;
        M5.Lcd.setCursor(10, 25);
        M5.Lcd.printf("   Attempt %d/30", attempts);
    }

    if (WiFi.status() != WL_CONNECTED) {
        M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
        M5.Lcd.setCursor(10, 40);
        M5.Lcd.println("   WiFi FAILED!");
        M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Lcd.println("   Check SSID/password");
        M5.Lcd.println("   Network: ANV");
        failReason = "WiFi FAILED";
        return false;
    }

    M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    M5.Lcd.println("   WiFi OK!");
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.print("   IP: ");
    M5.Lcd.println(WiFi.localIP().toString());

    // Шаг 2: Поиск Cardputer
    M5.Lcd.println("2) Find Cardputer...");

    // Попытка из RTC-памяти
    IPAddress rtcIP(lastReceiverIP[0], lastReceiverIP[1], lastReceiverIP[2], lastReceiverIP[3]);
    bool found = false;

    if (lastReceiverIP[0] != 0) {
        M5.Lcd.print("   Trying saved IP: ");
        M5.Lcd.println(rtcIP.toString());
        WiFiClient test;
        if (test.connect(rtcIP, PAGER_TCP_PORT, 1000)) {
            test.stop();
            found = true;
            M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
            M5.Lcd.println("   Saved IP works!");
            M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        } else {
            M5.Lcd.println("   Saved IP failed");
        }
    }

    if (!found) {
        // UDP broadcast discovery
        M5.Lcd.println("   UDP broadcast...");
        WiFiUDP udp;
        if (udp.begin(PAGER_UDP_PORT)) {
            udp.beginPacket(IPAddress(255, 255, 255, 255), PAGER_UDP_PORT);
            udp.write((const uint8_t*)PAGER_BEACON_MSG, strlen(PAGER_BEACON_MSG));
            udp.endPacket();

            unsigned long start = millis();
            while (millis() - start < 3000) {
                int packetSize = udp.parsePacket();
                if (packetSize > 0) {
                    char buf[32];
                    int len = udp.read(buf, sizeof(buf) - 1);
                    if (len > 0) {
                        buf[len] = 0;
                        if (strcmp(buf, PAGER_ACK_MSG) == 0) {
                            lastReceiverIP[0] = udp.remoteIP()[0];
                            lastReceiverIP[1] = udp.remoteIP()[1];
                            lastReceiverIP[2] = udp.remoteIP()[2];
                            lastReceiverIP[3] = udp.remoteIP()[3];
                            found = true;
                            M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
                            M5.Lcd.print("   Found! IP: ");
                            M5.Lcd.println(udp.remoteIP().toString());
                            M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
                            break;
                        }
                    }
                }
                delay(10);

                if ((millis() - start) % 1000 < 10) {
                    M5.Lcd.print(".");
                }
            }
            udp.stop();
        }
    }

    if (!found) {
        M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
        M5.Lcd.println("   Cardputer NOT found!");
        M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Lcd.println("   Is Cardputer on?");
        M5.Lcd.println("   Same WiFi network?");
        failReason = "Cardputer NOT found";
        return false;
    }

    // Шаг 3: TCP отправка
    M5.Lcd.println("3) Sending call...");

    IPAddress receiverIP(lastReceiverIP[0], lastReceiverIP[1], lastReceiverIP[2], lastReceiverIP[3]);

    for (int retry = 0; retry < PAGER_SEND_RETRIES; retry++) {
        M5.Lcd.printf("   Try %d/%d...\n", retry + 1, PAGER_SEND_RETRIES);
        WiFiClient tcp;
        if (tcp.connect(receiverIP, PAGER_TCP_PORT, PAGER_TCP_TIMEOUT_MS)) {
            uint8_t callByte = (uint8_t)type;
            tcp.write(callByte);

            unsigned long start = millis();
            while (tcp.connected() && millis() - start < PAGER_TCP_TIMEOUT_MS) {
                if (tcp.available() >= 1) {
                    uint8_t response = tcp.read();
                    tcp.stop();
                    if (response == PAGER_ACK) {
                        M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
                        M5.Lcd.println("   ACK received!");
                        return true;
                    } else {
                        M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
                        M5.Lcd.println("   Wrong response!");
                        failReason = "wrong ACK";
                        return false;
                    }
                }
                delay(10);
            }
            tcp.stop();
            M5.Lcd.println("   TCP timeout");
        } else {
            M5.Lcd.println("   TCP connect failed");
        }
        delay(PAGER_RETRY_DELAY_MS);
    }

    M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
    M5.Lcd.println("   All retries failed");
    failReason = "TCP send failed";
    return false;
}

void showDiagnostic(const char* line1, const char* line2, const char* line3, int color) {
    M5.Lcd.fillScreen(color == TFT_RED ? TFT_RED : TFT_BLACK);
    M5.Lcd.setTextColor(color == TFT_RED ? TFT_WHITE : TFT_WHITE, color == TFT_RED ? TFT_RED : TFT_BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 30);
    M5.Lcd.println(line1);
    M5.Lcd.setTextSize(1);
    if (line2) { M5.Lcd.setCursor(10, 60); M5.Lcd.println(line2); }
    if (line3) { M5.Lcd.setCursor(10, 80); M5.Lcd.println(line3); }
}

void goToDeepSleep() {
    M5.Lcd.setBrightness(0);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    btStop();
    delay(20);

    // Фиксируем POWER_HOLD (GPIO 4) чтобы AXP2101 не отключал питание в deep sleep
    gpio_hold_en(GPIO_NUM_4);

    esp_sleep_enable_ext0_wakeup(GPIO_NUM_37, LOW);
    esp_deep_sleep_start();
}
