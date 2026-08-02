#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "pager_protocol.h"

class PagerServer {
public:
    PagerServer() : _hasNew(false), _lastCall(CALL_UNKNOWN), _server(PAGER_TCP_PORT) {}

    void run() {
        WiFi.mode(WIFI_STA);
        WiFi.begin("ANV", "9115554644");
        _server.begin();
        _udp.begin(PAGER_UDP_PORT);
    }

    void listen() {
        // --- TCP: приём вызовов ---
        WiFiClient client = _server.available();
        if (client) {
            unsigned long start = millis();
            while (client.connected() && millis() - start < PAGER_TCP_TIMEOUT_MS) {
                if (client.available() >= 1) {
                    uint8_t byte = client.read();
                    _lastCall = decodeCallType(byte);
                    _senderIP = client.remoteIP().toString();
                    _hasNew = true;
                    client.write(PAGER_ACK);
                    delay(50);
                    break;
                }
                delay(10);
            }
            client.stop();
        }

        // --- UDP: ответ на beacon (discovery) ---
        int packetSize = _udp.parsePacket();
        if (packetSize > 0) {
            char buf[32];
            int len = _udp.read(buf, sizeof(buf) - 1);
            if (len > 0) {
                buf[len] = 0;
                if (strcmp(buf, PAGER_BEACON_MSG) == 0) {
                    // Отвечаем ACK отправителю
                    _udp.beginPacket(_udp.remoteIP(), _udp.remotePort());
                    _udp.write((const uint8_t*)PAGER_ACK_MSG, strlen(PAGER_ACK_MSG));
                    _udp.endPacket();
                }
            }
        }
    }

    bool hasNewCall() {
        bool ret = _hasNew;
        _hasNew = false;
        return ret;
    }

    CallType getLastCall() {
        return _lastCall;
    }

    String getSenderIP() {
        return _senderIP;
    }

    CallType decodeCallType(uint8_t byte) {
        switch(byte) {
            case 0: return CALL_NORMAL;
            case 1: return CALL_DINNER;
            case 2: return CALL_URGENT;
            default: return CALL_UNKNOWN;
        }
    }

private:
    bool _hasNew;
    CallType _lastCall;
    String _senderIP;
    WiFiServer _server;
    WiFiUDP _udp;
};
