#ifndef ALARM_MENU_H
#define ALARM_MENU_H

#include <M5Cardputer.h>
#include "AlarmSettings.h"
#include "ClockDisplay.h"

/// Класс настройки будильника через клавиатуру M5Cardputer.
///
/// Навигация (когда меню активно):
///   W/S      — переключение полей (часы → минуты → вкл/выкл → выход)
///   A/D      — изменение значения выбранного поля
///   Enter    — подтвердить и сохранить через AlarmSettings
///   C / #    — выход без сохранения
///
/// Вызов меню из основного цикла:
///   Enter    — открыть меню настроек
class AlarmMenu
{
public:
    AlarmMenu(AlarmSettings& alarm, ClockDisplay& display);

    /// Инициализация меню
    void begin();

    /// Открыть меню настроек
    void open();

    /// Активно ли меню в данный момент
    bool isActive() const;

    /// Обработка клавиш — вызывать из loop(), когда menu.isActive() == true
    void update();

private:
    enum Field : uint8_t {
        FIELD_HOUR = 0,
        FIELD_MIN,
        FIELD_ENABLED,
        FIELD_EXIT,
        FIELD_COUNT
    };

    AlarmSettings& _alarm;
    ClockDisplay&  _display;
    bool           _active;
    bool           _justOpened;   ///< флаг первого кадра после open()
    Field          _currentField;

    // Рабочая копия редактируемых значений
    int  _editHour;
    int  _editMin;
    bool _editEnabled;

    // Приватные методы
    void _draw();
    void _drawField(int x, int y, const char* label, const char* value, bool isFocused);
    void _commitAndExit();
    void _cancelAndExit();
    void _nextField();
    void _prevField();
    void _increaseValue();
    void _decreaseValue();
};

#endif // ALARM_MENU_H
