#include "ClockDisplay.h"

// ============================================================
// Размеры дисплея M5Cardputer (ST7789): 240 x 135 (landscape)
// Поворот 1: ширина 240, высота 135
//
// Компоновка:
//   ┌─────────────────────────────────────┐
//   │  Дата, день недели     (textSize 2) │  y = 2
//   │                                     │
//   │     ЧЧ:ММ              (textSize 8) │  y = 28
//   │       :СС              (textSize 5) │  y = 92
//   │                                     │
//   │                     [Будильник 7:30]│  y = 118
//   └─────────────────────────────────────┘
// ============================================================

// Константы позиционирования
static constexpr int DATE_Y        = 2;
static constexpr int DATE_TEXT_SIZE = 2;

static constexpr int TIME_Y        = 28;
static constexpr int TIME_TEXT_SIZE = 8;

static constexpr int SEC_Y         = 92;
static constexpr int SEC_TEXT_SIZE  = 5;

static constexpr int ALARM_Y       = 114;
static constexpr int ALARM_X       = 140;   // правая половина

// Размеры символов для default font (6x8 в масштабе 1)
#define CHAR_W(ts)  (6 * (ts))
#define CHAR_H(ts)  (8 * (ts))

// ============================================================

ClockDisplay::ClockDisplay()
    : _w(240)
    , _h(135)
{
}

void ClockDisplay::begin()
{
    M5Cardputer.Display.setRotation(1);    // landscape: 240x135
    // datum остается TL_DATUM (по умолчанию) — все расчёты под top-left
    clearScreen();
}

void ClockDisplay::clearScreen()
{
    M5Cardputer.Display.fillScreen(COLOR_BG);
}

// ============================================================
// drawTime
// ============================================================
void ClockDisplay::drawTime(int hour, int min, int sec)
{
    // 1. Стираем старую область времени (большой прямоугольник)
    //    от верха цифр до низа секунд
    M5Cardputer.Display.fillRect(0, TIME_Y - 4,
                                 _w, SEC_Y + CHAR_H(SEC_TEXT_SIZE) - TIME_Y + 8,
                                 COLOR_BG);

    // 2. "ЧЧ:ММ" — крупный шрифт, центрированный по ширине
    //    строка "ЧЧ:ММ" = 5 символов
    int charW8 = CHAR_W(TIME_TEXT_SIZE);
    int timeStrW = charW8 * 5;  // HH:MM
    int timeX = (_w - timeStrW) / 2;
    int timeY = TIME_Y;

    // Часы (2 цифры)
    M5Cardputer.Display.setTextSize(TIME_TEXT_SIZE);
    M5Cardputer.Display.setTextColor(COLOR_TIME);

    char buf[3];
    snprintf(buf, sizeof(buf), "%02d", hour);
    M5Cardputer.Display.setCursor(timeX, timeY);
    M5Cardputer.Display.print(buf);

    // Двоеточие (мигает каждую секунду — показываем всегда, но можно
    // сделать мигание по sec % 2)
    if (sec % 2 == 0)
    {
        M5Cardputer.Display.setTextColor(COLOR_SEP);
    }
    else
    {
        M5Cardputer.Display.setTextColor(COLOR_BG); // прячем на нечётных секундах
    }
    M5Cardputer.Display.setCursor(timeX + charW8 * 2, timeY);
    M5Cardputer.Display.print(":");
    M5Cardputer.Display.setTextColor(COLOR_TIME);

    // Минуты (2 цифры)
    snprintf(buf, sizeof(buf), "%02d", min);
    M5Cardputer.Display.setCursor(timeX + charW8 * 3, timeY);
    M5Cardputer.Display.print(buf);

    // 3. ":СС" — мелкий шрифт, по центру под ЧЧ:ММ
    int charW5 = CHAR_W(SEC_TEXT_SIZE);
    int secStrW = charW5 * 3;  // :SS
    int secX = (_w - secStrW) / 2;
    int secY = SEC_Y;

    M5Cardputer.Display.setTextSize(SEC_TEXT_SIZE);
    M5Cardputer.Display.setTextColor(COLOR_TIME);
    snprintf(buf, sizeof(buf), ":%02d", sec);
    M5Cardputer.Display.setCursor(secX, secY);
    M5Cardputer.Display.print(buf);
}

// ============================================================
// drawDate
// ============================================================
void ClockDisplay::drawDate(int year, int month, int day, int weekday)
{
    // Стираем область даты (верхняя строка)
    int dateH = CHAR_H(DATE_TEXT_SIZE);
    M5Cardputer.Display.fillRect(0, DATE_Y, _w, dateH + 2, COLOR_BG);

    M5Cardputer.Display.setTextSize(DATE_TEXT_SIZE);
    M5Cardputer.Display.setTextColor(COLOR_DATE);

    char buf[32];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %s",
             year, month, day, _weekdayName(weekday));

    int strW = strlen(buf) * CHAR_W(DATE_TEXT_SIZE);
    int x = (_w - strW) / 2;
    if (x < 0) x = 0;

    M5Cardputer.Display.setCursor(x, DATE_Y);
    M5Cardputer.Display.print(buf);
}

// ============================================================
// drawAlarmStatus
// ============================================================
void ClockDisplay::drawAlarmStatus(bool isSet, int alarmHour, int alarmMin)
{
    // Стираем старую область индикатора
    M5Cardputer.Display.fillRect(ALARM_X, ALARM_Y,
                                 _w - ALARM_X, CHAR_H(DATE_TEXT_SIZE) + 2,
                                 COLOR_BG);

    if (!isSet) return;

    M5Cardputer.Display.setTextSize(DATE_TEXT_SIZE);
    M5Cardputer.Display.setTextColor(COLOR_ALARM);

    char buf[24];
    snprintf(buf, sizeof(buf), "ALARM %02d:%02d", alarmHour, alarmMin);

    M5Cardputer.Display.setCursor(ALARM_X, ALARM_Y);
    M5Cardputer.Display.print(buf);
}

// ============================================================
// drawSleepOverlay
// ============================================================
void ClockDisplay::drawSleepOverlay()
{
    // Рисуем разреженную сетку полупрозрачных точек/линий,
    // имитирующих затемнение. M5GFX не поддерживает alpha blending,
    // поэтому используем тёмно-серые полосы.

    // Затемняющие полосы через каждые 4 пикселя
    M5Cardputer.Display.fillRect(0, 0, _w, _h, 0x0001); // чуть темнее чёрного

    // Рисуем тонкие диагональные линии для эффекта "выключенного экрана"
    // или просто затемняющую маску
    for (int y = 0; y < _h; y += 6)
    {
        M5Cardputer.Display.drawFastHLine(0, y, _w, 0x0841);
    }
}

// ============================================================
// Приватные методы
// ============================================================

const char* ClockDisplay::_weekdayName(uint8_t wday)
{
    // wday: 0=воскресенье, 1=понедельник, ... 6=суббота
    // (struct tm возвращает 0..6, где 0 = воскресенье)
    static const char* names[7] = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };
    if (wday > 6) wday = 0;
    return names[wday];
}
