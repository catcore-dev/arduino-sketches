#ifndef PAGER_PROTOCOL_H
#define PAGER_PROTOCOL_H

#include <Arduino.h>

// --- Типы вызовов ---
enum CallType : uint8_t {
    CALL_NORMAL = 0,   // Обычный вызов
    CALL_DINNER = 1,   // Обед
    CALL_URGENT = 2,   // Срочный вызов
    CALL_UNKNOWN = 255
};

// --- Порт ---
#define PAGER_TCP_PORT     8888

// --- UDP discovery ---
#define PAGER_UDP_PORT     8889
#define PAGER_BEACON_MSG   "PAGER-BEACON"
#define PAGER_ACK_MSG      "PAGER-ACK"

// --- Формат TCP-сообщения ---
#define PAGER_ACK  0x06
#define PAGER_NAK  0x15

// --- Таймауты (мс) ---
#define PAGER_TCP_TIMEOUT_MS  5000
#define PAGER_SEND_RETRIES    3
#define PAGER_RETRY_DELAY_MS  2000

// --- Приоритеты (выше = важнее) ---
inline int callPriority(CallType t) {
    switch(t) {
        case CALL_URGENT: return 3;
        case CALL_DINNER: return 2;
        case CALL_NORMAL: return 1;
        default: return 0;
    }
}

// --- Максимум вызовов в очереди ---
#define PAGER_MAX_QUEUE  5

#endif
