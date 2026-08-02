#ifndef CLOCK_DISPLAY_H
#define CLOCK_DISPLAY_H

#include <M5Cardputer.h>

/// Класс для отрисовки часов, даты и статуса будильника на дисплее M5Cardputer.
/// Экран 240x135 (landscape), чёрный фон, зелёные/циановые цифры.
class ClockDisplay
{
public:
    ClockDisplay();

    /// Инициализация: очистка экрана, настройка поворота
    void begin();

    /// Отрисовка времени "ЧЧ:ММ" крупным шрифтом (~3/4 высоты экрана)
    /// и ":СС" мелким шрифтом под ними
    void drawTime(int hour, int min, int sec);

    /// Отрисовка даты и дня недели мелким шрифтом в верхней части экрана
    void drawDate(int year, int month, int day, int weekday);

    /// Индикатор будильника в правом нижнем углу
    /// Если isSet == false — очищает область индикатора
    void drawAlarmStatus(bool isSet, int alarmHour, int alarmMin);

    /// Затемнение экрана для режима сна (рисует полупрозрачную маску)
    void drawSleepOverlay();

    /// Полная очистка экрана (чёрный фон)
    void clearScreen();

private:
    static constexpr uint16_t COLOR_BG    = TFT_BLACK;
    static constexpr uint16_t COLOR_TIME  = 0x07FF;   // Cyan
    static constexpr uint16_t COLOR_DATE  = TFT_GREEN;
    static constexpr uint16_t COLOR_ALARM = TFT_GREEN;
    static constexpr uint16_t COLOR_SEP   = 0x7BEF;   // Серый для разделителя

    int _w;  // ширина дисплея
    int _h;  // высота дисплея

    /// Возвращает короткое название дня недели (рус/англ)
    static const char* _weekdayName(uint8_t wday);
};

#endif // CLOCK_DISPLAY_H
