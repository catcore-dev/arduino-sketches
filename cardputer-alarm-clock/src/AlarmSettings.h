#ifndef ALARM_SETTINGS_H
#define ALARM_SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>

/// Класс для хранения настроек будильника в энергонезависимой памяти (NVS).
///
/// Использует встроенную библиотеку Arduino Preferences (обёртка над ESP32 NVS).
/// Данные сохраняются между перезагрузками.
///
/// NVS namespace: "alarm"
/// Ключи:
///   "enabled"    — bool,  флаг активности будильника
///   "hour"       — uint8_t, час срабатывания (0-23)
///   "min"        — uint8_t, минута срабатывания (0-59)
///   "tz_offset"  — int32_t, смещение часового пояса в секундах
///   "compile_ts" — uint32_t, timestamp последней компиляции/настройки
class AlarmSettings
{
public:
    AlarmSettings();

    /// Инициализация: открыть NVS namespace, прочитать все сохранённые значения.
    /// Если данных нет — устанавливаются значения по умолчанию (07:00, выключен).
    void begin();

    /// Сохранить время срабатывания будильника.
    void saveAlarmTime(int hour, int min);

    /// Получить час срабатывания (0-23).
    int getAlarmHour() const;

    /// Получить минуту срабатывания (0-59).
    int getAlarmMin() const;

    /// Проверить, активирован ли будильник.
    bool isAlarmSet() const;

    /// Включить/выключить будильник.
    void setAlarmEnabled(bool enabled);

    /// Получить флаг активности будильника.
    bool getAlarmEnabled() const;

    /// Сбросить будильник: выключить, установить 07:00.
    void clearAlarm();

    /// Сохранить смещение часового пояса (в секундах от UTC).
    void saveTzOffset(int32_t offsetSec);

    /// Получить сохранённое смещение часового пояса.
    int32_t getTzOffset() const;

    /// Сохранить timestamp (например, время компиляции или последней настройки).
    void saveCompileTimestamp(uint32_t ts);

    /// Получить сохранённый timestamp.
    uint32_t getCompileTimestamp() const;

    /// Принудительно записать все текущие значения в NVS.
    void commit();

private:
    Preferences _prefs;

    // Кэшированные значения
    bool     _enabled;
    uint8_t  _hour;
    uint8_t  _alarmMin;
    int32_t  _tzOffset;
    uint32_t _compileTs;

    static const char* NVS_NAMESPACE;
    static const char* KEY_ENABLED;
    static const char* KEY_HOUR;
    static const char* KEY_MIN;
    static const char* KEY_TZ_OFFSET;
    static const char* KEY_COMPILE_TS;
};

#endif // ALARM_SETTINGS_H
