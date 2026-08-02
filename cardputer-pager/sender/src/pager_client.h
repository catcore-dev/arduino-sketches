#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "pager_protocol.h"

// По умолчанию: раскомментируй и заполни для статической конфигурации WiFi
#define WIFI_SSID "ANV"
#define WIFI_PASS "9115554644"

// RTC-сохраняемый IP последнего известного ресивера (определён в main.cpp)
extern RTC_DATA_ATTR uint8_t lastReceiverIP[4];

class PagerClient {
public:
    PagerClient() : _receiverIP(IPAddress(0,0,0,0)) {}

    void begin() {
        WiFi.mode(WIFI_STA);
#ifdef WIFI_SSID
        WiFi.begin(WIFI_SSID, WIFI_PASS);
#else
        WiFi.begin();
#endif
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 30) {
            delay(500);
            attempts++;
        }
        if (WiFi.status() == WL_CONNECTED) {
            discoverReceiver();
        }
    }

    bool discoverReceiver() {
        bool found = false;

        // Попытка из RTC-памяти (lastReceiverIP — RTC_DATA_ATTR, сохраняется при deep sleep)
        IPAddress rtcIP(lastReceiverIP[0], lastReceiverIP[1], lastReceiverIP[2], lastReceiverIP[3]);
        if (lastReceiverIP[0] != 0 && testIP(rtcIP)) {
            _receiverIP = rtcIP;
            return true;
        }

        // UDP broadcast discovery
        WiFiUDP udp;
        if (udp.begin(PAGER_UDP_PORT)) {
            udp.beginPacket(IPAddress(255,255,255,255), PAGER_UDP_PORT);
            udp.write((const uint8_t*)PAGER_BEACON_MSG, strlen(PAGER_BEACON_MSG));
            udp.endPacket();

            unsigned long start = millis();
            while (millis() - start < 2000) {
                int packetSize = udp.parsePacket();
                if (packetSize > 0) {
                    char buf[32];
                    int len = udp.read(buf, sizeof(buf) - 1);
                    if (len > 0) {
                        buf[len] = 0;
                        if (strcmp(buf, PAGER_ACK_MSG) == 0) {
                            _receiverIP = udp.remoteIP();
                            // Сохраняем IP в RTC-память
                            lastReceiverIP[0] = _receiverIP[0];
                            lastReceiverIP[1] = _receiverIP[1];
                            lastReceiverIP[2] = _receiverIP[2];
                            lastReceiverIP[3] = _receiverIP[3];
                            found = true;
                            break;
                        }
                    }
                }
                delay(10);
            }
            udp.stop();
        }

        return found;
    }

    bool sendCall(CallType type) {
        if (_receiverIP[0] == 0) {
            if (!discoverReceiver()) return false;
        }

        for (int retry = 0; retry < PAGER_SEND_RETRIES; retry++) {
            WiFiClient client;
            if (client.connect(_receiverIP, PAGER_TCP_PORT, PAGER_TCP_TIMEOUT_MS)) {
                uint8_t callByte = (uint8_t)type;
                client.write(callByte);

                unsigned long start = millis();
                while (client.connected() && millis() - start < PAGER_TCP_TIMEOUT_MS) {
                    if (client.available() >= 1) {
                        uint8_t response = client.read();
                        client.stop();
                        return response == PAGER_ACK;
                    }
                    delay(10);
                }
                client.stop();
            }
            delay(PAGER_RETRY_DELAY_MS);
        }
        return false;
    }

private:
    IPAddress _receiverIP;

    bool testIP(IPAddress ip) {
        WiFiClient client;
        bool result = client.connect(ip, PAGER_TCP_PORT, 1000);
        if (result) client.stop();
        return result;
    }
};
