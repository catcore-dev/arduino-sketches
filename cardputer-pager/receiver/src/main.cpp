#include <M5Cardputer.h>
#include "pager_protocol.h"
#include "pager_server.h"
#include "melodies.h"

PagerServer server;
CallType currentCall = CALL_UNKNOWN;

void showNotification(CallType type);
void showIdle();
void startMelody(CallType type);
void stopMelody();

void setup() {
    M5Cardputer.begin();

    // Максимальная громкость спикера
    M5Cardputer.Speaker.setVolume(255);

    auto& display = M5Cardputer.Display;
    display.setRotation(1);
    display.fillScreen(TFT_BLACK);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextDatum(MC_DATUM);

    display.drawString("Connecting...", 120, 60);

    server.run();

    display.fillScreen(TFT_BLACK);
    display.drawString("WiFi: ANV", 120, 40);
    display.drawString("Connecting...", 120, 60);
    
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    
    int wait = 0;
    while (WiFi.status() != WL_CONNECTED && wait < 60) {
        delay(500);
        wait++;
        // Показываем точки каждые 500мс
        display.setTextSize(1);
        String dots;
        int dotCount = (wait / 2) % 4;
        for (int di = 0; di < dotCount; di++) dots += ".";
        display.fillRect(80, 80, 80, 20, TFT_BLACK);
        display.drawString(dots, 120, 80);
    }

    display.fillScreen(TFT_BLACK);

    if (WiFi.status() == WL_CONNECTED) {
        display.drawString("Ready", 120, 50);
        display.drawString(WiFi.localIP().toString(), 120, 70);
        display.setTextSize(1);
        display.drawString("Waiting for calls...", 120, 100);
    } else {
        display.drawString("WiFi FAILED", 120, 60);
        display.setTextSize(1);
        display.drawString("Restart device", 120, 80);
    }
}

void loop() {
    M5Cardputer.update();

    server.listen();

    if (server.hasNewCall()) {
        CallType newCall = server.getLastCall();

        if (currentCall == CALL_UNKNOWN || callPriority(newCall) > callPriority(currentCall)) {
            currentCall = newCall;
            showNotification(newCall);
            startMelody(newCall);
        }
    }

    if (currentCall != CALL_UNKNOWN && M5Cardputer.Keyboard.isPressed()) {
        stopMelody();
        currentCall = CALL_UNKNOWN;
        showIdle();
        delay(500);
    }
}

void showNotification(CallType type) {
    auto& display = M5Cardputer.Display;
    display.fillScreen(TFT_BLACK);
    display.setTextSize(2);
    display.setTextDatum(MC_DATUM);

    switch(type) {
        case CALL_NORMAL:
            display.setTextColor(TFT_CYAN, TFT_BLACK);
            display.drawString("CALL", 120, 40);
            display.setTextSize(1);
            display.drawString("Please come here", 120, 80);
            break;
        case CALL_DINNER:
            display.setTextColor(TFT_YELLOW, TFT_BLACK);
            display.drawString("DINNER", 120, 40);
            display.setTextSize(2);
            display.drawString("Time to eat!", 120, 80);
            break;
        case CALL_URGENT:
            display.fillScreen(TFT_RED);
            display.setTextColor(TFT_WHITE, TFT_RED);
            display.setTextSize(4);
            display.drawString("URGENT", 120, 50);
            display.setTextSize(2);
            display.drawString("Come now!", 120, 90);
            break;
    }

    display.setTextSize(1);
    display.setTextColor(TFT_DARKGREY, TFT_BLACK);
    display.drawString("Press any key", 120, 120);
}

void showIdle() {
    auto& display = M5Cardputer.Display;
    display.fillScreen(TFT_BLACK);
    display.setTextColor(TFT_DARKGREEN, TFT_BLACK);
    display.setTextSize(1);
    display.setTextDatum(MC_DATUM);
    display.drawString("Ready", 120, 50);
    display.drawString(WiFi.localIP().toString(), 120, 70);
    display.drawString("Waiting for calls...", 120, 100);
}

void startMelody(CallType type) {
    const Note* melody = getMelody(type);
    if (melody) {
        playMelody(melody);
    }
}

void stopMelody() {
    stopMelodyNow();
}
