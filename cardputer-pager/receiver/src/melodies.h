#pragma once
#include <Arduino.h>
#include "pager_protocol.h"

struct Note {
    uint16_t frequency;  // 0 = пауза
    uint16_t duration;   // мс
};

volatile bool melodyPlaying = false;

// --- Обычный вызов: приятная последовательность (повышенные частоты) ---
const Note normalMelody[] = {
    {2093, 150}, // C7
    {2637, 150}, // E7
    {3136, 150}, // G7
    {2093, 300}, // C7
    {2093, 150}, // C7
    {2637, 150}, // E7
    {3136, 150}, // G7
    {2093, 300}, // C7
    {0, 0}
};

// --- Обед: весёлая восходящая последовательность (повышенные частоты) ---
const Note dinnerMelody[] = {
    {2093, 300}, // C7
    {2349, 150}, // D7
    {2637, 150}, // E7
    {2794, 150}, // F7
    {3136, 300}, // G7
    {4186, 400}, // C8
    {0, 0}
};

// --- Срочный вызов: резкий прерывистый сигнал ---
const Note urgentMelody[] = {
    {3136, 100}, // G7
    {0, 80},
    {3136, 100},
    {0, 80},
    {3136, 100},
    {0, 250},
    {4186, 100}, // C8
    {0, 80},
    {4186, 100},
    {0, 80},
    {4186, 100},
    {0, 0}
};

const Note* getMelody(CallType type) {
    switch(type) {
        case CALL_NORMAL: return normalMelody;
        case CALL_DINNER: return dinnerMelody;
        case CALL_URGENT: return urgentMelody;
        default: return nullptr;
    }
}

void playMelody(const Note* melody) {
    if (!melody) return;
    melodyPlaying = true;
    for (int i = 0; melody[i].duration > 0 && melodyPlaying; i++) {
        if (melody[i].frequency == 0) {
            M5Cardputer.Speaker.end();
            delay(melody[i].duration);
        } else {
            M5Cardputer.Speaker.tone(melody[i].frequency, melody[i].duration);
            delay(melody[i].duration);
        }
    }
    if (melodyPlaying) {
        M5Cardputer.Speaker.end();
    }
    melodyPlaying = false;
}

void stopMelodyNow() {
    melodyPlaying = false;
    M5Cardputer.Speaker.end();
}
