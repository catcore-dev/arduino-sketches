#ifndef RTC_CLOCK_H
#define RTC_CLOCK_H

#include <Arduino.h>
#include <Wire.h>
#include <time.h>

class RTCClock
{
public:
    /// Конструктор: сохраняет адрес RTC (0x51 по умолчанию)
    RTCClock(uint8_t addr = 0x51);

    /// Инициализация: проверяет наличие RTC на шине Wire
    /// Возвращает true, если RTC обнаружен
    bool begin(TwoWire &wire = Wire);

    /// Установить дату и время
    /// Все параметры в десятичном формате (не BCD)
    void setDateTime(uint16_t year, uint8_t month, uint8_t day,
                     uint8_t hour, uint8_t min, uint8_t sec);

    /// Получить дату и время в виде struct tm
    /// Возвращает заполненную структуру или нулевую при ошибке
    struct tm getDateTime();

    /// Проверить, доступен ли RTC
    bool isPresent() const { return _present; }

    /// Проверить, был ли сброс питания (VL bit в секундах)
    bool isVoltageLow();

private:
    uint8_t _addr;
    bool _present;
    TwoWire *_wire;

    /// Записать байт в регистр RTC
    void _writeReg(uint8_t reg, uint8_t data);
    /// Прочитать байт из регистра RTC
    uint8_t _readReg(uint8_t reg);
    /// Прочитать несколько байт из регистра RTC
    void _readRegs(uint8_t reg, uint8_t *buf, size_t len);

    /// Преобразовать BCD в десятичное число
    static uint8_t _bcd2dec(uint8_t bcd);
    /// Преобразовать десятичное число в BCD
    static uint8_t _dec2bcd(uint8_t dec);
};

#endif // RTC_CLOCK_H
