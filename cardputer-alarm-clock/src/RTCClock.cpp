#include "RTCClock.h"

// Регистры BM8563 / PCF8563
#define REG_CONTROL_STATUS_1 0x00
#define REG_CONTROL_STATUS_2 0x01
#define REG_SECONDS          0x02
#define REG_MINUTES          0x03
#define REG_HOURS            0x04
#define REG_DAYS             0x05
#define REG_WEEKDAYS         0x06
#define REG_MONTHS           0x07
#define REG_YEARS            0x08

// Биты
#define VL_BIT               0x80   // Voltage Low в регистре секунд

// ============================================================

RTCClock::RTCClock(uint8_t addr)
    : _addr(addr)
    , _present(false)
    , _wire(nullptr)
{
}

bool RTCClock::begin(TwoWire &wire)
{
    _wire = &wire;

    // Проверяем наличие RTC: отправляем адрес и смотрим ответ
    _wire->beginTransmission(_addr);
    uint8_t err = _wire->endTransmission();
    _present = (err == 0);

    if (_present)
    {
        // Инициализация: включаем секундный режим на CLKOUT (тихо),
        // выключаем тестовые режимы
        _writeReg(REG_CONTROL_STATUS_1, 0x00);
        _writeReg(REG_CONTROL_STATUS_2, 0x00);
    }

    return _present;
}

void RTCClock::setDateTime(uint16_t year, uint8_t month, uint8_t day,
                           uint8_t hour, uint8_t min, uint8_t sec)
{
    if (!_present || !_wire) return;

    // Корректируем год в формат 0-99 (от 2000)
    uint8_t yearShort = (year >= 2000) ? (year - 2000) : year;

    // BCD-преобразование
    uint8_t data[7];
    data[0] = _dec2bcd(sec);         // секунды
    data[1] = _dec2bcd(min);         // минуты
    data[2] = _dec2bcd(hour);        // часы
    data[3] = _dec2bcd(day);         // дни
    data[4] = 0x00;                  // день недели (не задаём)
    data[5] = _dec2bcd(month);       // месяц
    data[6] = _dec2bcd(yearShort);   // год

    // Записываем начиная с регистра секунд
    _wire->beginTransmission(_addr);
    _wire->write(REG_SECONDS);
    for (int i = 0; i < 7; i++)
    {
        _wire->write(data[i]);
    }
    _wire->endTransmission();
}

struct tm RTCClock::getDateTime()
{
    struct tm t = {0};

    if (!_present || !_wire) return t;

    // Читаем 7 байт времени, начиная с регистра секунд
    uint8_t data[7];
    _readRegs(REG_SECONDS, data, 7);

    // Распаковываем BCD
    t.tm_sec  = _bcd2dec(data[0] & 0x7F);  // убираем VL бит
    t.tm_min  = _bcd2dec(data[1] & 0x7F);
    t.tm_hour = _bcd2dec(data[2] & 0x3F);
    t.tm_mday = _bcd2dec(data[3] & 0x3F);
    t.tm_wday = data[4] & 0x07;
    t.tm_mon  = _bcd2dec(data[5] & 0x1F) - 1;  // struct tm: 0-11
    t.tm_year = _bcd2dec(data[6]) + 100;        // struct tm: годы от 1900

    // Проверка столетия (бит 7 в регистре месяцев)
    if (data[5] & 0x80)
    {
        t.tm_year += 100;  // 2100+
    }

    return t;
}

bool RTCClock::isVoltageLow()
{
    if (!_present || !_wire) return false;
    uint8_t sec = _readReg(REG_SECONDS);
    return (sec & VL_BIT) != 0;
}

// ===================== Приватные методы =====================

void RTCClock::_writeReg(uint8_t reg, uint8_t data)
{
    _wire->beginTransmission(_addr);
    _wire->write(reg);
    _wire->write(data);
    _wire->endTransmission();
}

uint8_t RTCClock::_readReg(uint8_t reg)
{
    _wire->beginTransmission(_addr);
    _wire->write(reg);
    _wire->endTransmission(false);

    _wire->requestFrom(_addr, (uint8_t)1);
    if (_wire->available())
    {
        return _wire->read();
    }
    return 0;
}

void RTCClock::_readRegs(uint8_t reg, uint8_t *buf, size_t len)
{
    if (!buf || len == 0) return;

    _wire->beginTransmission(_addr);
    _wire->write(reg);
    _wire->endTransmission(false);

    _wire->requestFrom(_addr, (uint8_t)len);
    size_t i = 0;
    while (_wire->available() && i < len)
    {
        buf[i++] = _wire->read();
    }
    // Если прочитали меньше, заполняем нулями
    while (i < len)
    {
        buf[i++] = 0;
    }
}

uint8_t RTCClock::_bcd2dec(uint8_t bcd)
{
    return (bcd >> 4) * 10 + (bcd & 0x0F);
}

uint8_t RTCClock::_dec2bcd(uint8_t dec)
{
    return ((dec / 10) << 4) | (dec % 10);
}
