#include "AlarmSettings.h"

// ===================== Имена NVS namespace и ключей =====================

const char* AlarmSettings::NVS_NAMESPACE = "alarm";
const char* AlarmSettings::KEY_ENABLED   = "enabled";
const char* AlarmSettings::KEY_HOUR      = "hour";
const char* AlarmSettings::KEY_MIN       = "min";
const char* AlarmSettings::KEY_TZ_OFFSET = "tz_offset";
const char* AlarmSettings::KEY_COMPILE_TS = "compile_ts";

// Значения по умолчанию
static const bool     DEFAULT_ENABLED  = false;
static const uint8_t  DEFAULT_HOUR     = 7;
static const uint8_t  DEFAULT_MIN      = 0;
static const int32_t  DEFAULT_TZ_OFFSET = 0;     // UTC+0
static const uint32_t DEFAULT_COMPILE_TS = 0;

// ===================== Конструктор =====================

AlarmSettings::AlarmSettings()
    : _enabled(DEFAULT_ENABLED)
    , _hour(DEFAULT_HOUR)
    , _alarmMin(DEFAULT_MIN)
    , _tzOffset(DEFAULT_TZ_OFFSET)
    , _compileTs(DEFAULT_COMPILE_TS)
{
}

// ===================== begin() =====================

void AlarmSettings::begin()
{
    // Открываем NVS namespace в режиме чтения/записи
    _prefs.begin(NVS_NAMESPACE, false);

    // Читаем сохранённые значения; если ключа нет — используем дефолт
    _enabled   = _prefs.getBool(KEY_ENABLED, DEFAULT_ENABLED);
    _hour      = _prefs.getUChar(KEY_HOUR, DEFAULT_HOUR);
    _alarmMin       = _prefs.getUChar(KEY_MIN, DEFAULT_MIN);
    _tzOffset  = _prefs.getInt(KEY_TZ_OFFSET, DEFAULT_TZ_OFFSET);
    _compileTs = _prefs.getUInt(KEY_COMPILE_TS, DEFAULT_COMPILE_TS);

    Serial.printf("[AlarmSettings] Loaded: enabled=%d, time=%02d:%02d, tz=%+ld, compile_ts=%lu\n",
                  _enabled, _hour, _alarmMin, (long)_tzOffset, (unsigned long)_compileTs);
}

// ===================== Время будильника =====================

void AlarmSettings::saveAlarmTime(int hour, int min)
{
    // Валидация
    if (hour < 0)   hour = 0;
    if (hour > 23)  hour = 23;
    if (min < 0)    min  = 0;
    if (min > 59)   min  = 59;

    _hour = (uint8_t)hour;
    _alarmMin  = (uint8_t)min;

    _prefs.putUChar(KEY_HOUR, _hour);
    _prefs.putUChar(KEY_MIN,  _alarmMin);

    Serial.printf("[AlarmSettings] Time saved: %02d:%02d\n", _hour, _alarmMin);
}

int AlarmSettings::getAlarmHour() const
{
    return _hour;
}

int AlarmSettings::getAlarmMin() const
{
    return _alarmMin;
}

// ===================== Флаг активности =====================

bool AlarmSettings::isAlarmSet() const
{
    return _enabled;
}

void AlarmSettings::setAlarmEnabled(bool enabled)
{
    _enabled = enabled;
    _prefs.putBool(KEY_ENABLED, _enabled);
    Serial.printf("[AlarmSettings] Alarm %s\n", _enabled ? "ENABLED" : "DISABLED");
}

bool AlarmSettings::getAlarmEnabled() const
{
    return _enabled;
}

// ===================== Сброс =====================

void AlarmSettings::clearAlarm()
{
    _enabled = DEFAULT_ENABLED;
    _hour    = DEFAULT_HOUR;
    _alarmMin     = DEFAULT_MIN;

    _prefs.putBool(KEY_ENABLED, _enabled);
    _prefs.putUChar(KEY_HOUR, _hour);
    _prefs.putUChar(KEY_MIN,  _alarmMin);

    Serial.printf("[AlarmSettings] Cleared. Default: %02d:%02d, disabled\n", _hour, _alarmMin);
}

// ===================== Часовой пояс =====================

void AlarmSettings::saveTzOffset(int32_t offsetSec)
{
    _tzOffset = offsetSec;
    _prefs.putInt(KEY_TZ_OFFSET, _tzOffset);
    Serial.printf("[AlarmSettings] TZ offset saved: %+ld sec\n", (long)_tzOffset);
}

int32_t AlarmSettings::getTzOffset() const
{
    return _tzOffset;
}

// ===================== Timestamp компиляции =====================

void AlarmSettings::saveCompileTimestamp(uint32_t ts)
{
    _compileTs = ts;
    _prefs.putUInt(KEY_COMPILE_TS, _compileTs);
    // Не логируем каждое сохранение — слишком часто
}

uint32_t AlarmSettings::getCompileTimestamp() const
{
    return _compileTs;
}

// ===================== commit() =====================

void AlarmSettings::commit()
{
    // Preferences уже пишет при каждом put, но для явной синхронизации:
    _prefs.end();
    _prefs.begin(NVS_NAMESPACE, false);
    Serial.println("[AlarmSettings] Committed to NVS");
}
