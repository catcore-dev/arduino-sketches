#ifndef ALARM_BUZZER_H
#define ALARM_BUZZER_H

#include <M5Unified.h>
#include <M5Cardputer.h>

class AlarmBuzzer
{
public:
    AlarmBuzzer();

    /// Настройка громкости спикера
    void begin();

    /// Запустить циклический звуковой сигнал будильника
    void playAlarm();

    /// Немедленно остановить звук
    void stopAlarm();

    /// Играет ли сейчас будильник
    bool isPlaying() const;

    /// Вызывать в loop() — управляет таймингом звукового паттерна
    void update();

private:
    enum State : uint8_t
    {
        IDLE        = 0,
        BEEP        = 1,
        SHORT_PAUSE = 2,
        LONG_PAUSE  = 3
    };

    State        _state;
    unsigned long _stateStart;  ///< millis() когда началось текущее состояние
    int          _beepCount;    ///< сколько бипов сделано в текущем цикле

    // Параметры звукового паттерна
    static constexpr int    FREQ             = 2000;   // частота тона (Hz)
    static constexpr int    BEEP_MS          = 200;    // длительность одного бипа
    static constexpr int    SHORT_PAUSE_MS   = 100;    // пауза между бипами
    static constexpr int    LONG_PAUSE_MS    = 1000;   // пауза между циклами
    static constexpr int    BEEPS_PER_CYCLE  = 3;      // бипов в одном цикле
    static constexpr uint8_t VOLUME          = 80;     // умеренная громкость (0-255)
};

#endif // ALARM_BUZZER_H
