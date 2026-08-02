#include "AlarmBuzzer.h"

AlarmBuzzer::AlarmBuzzer()
    : _state(IDLE)
    , _stateStart(0)
    , _beepCount(0)
{
}

void AlarmBuzzer::begin()
{
    M5Cardputer.Speaker.setVolume(VOLUME);
    _state = IDLE;
}

void AlarmBuzzer::playAlarm()
{
    if (_state != IDLE) return;  // уже играет

    _state      = BEEP;
    _stateStart = millis();
    _beepCount  = 0;
    M5Cardputer.Speaker.tone(FREQ, BEEP_MS);
}

void AlarmBuzzer::stopAlarm()
{
    _state      = IDLE;
    _stateStart = 0;
    _beepCount  = 0;
    M5Cardputer.Speaker.stop();
}

bool AlarmBuzzer::isPlaying() const
{
    return _state != IDLE;
}

void AlarmBuzzer::update()
{
    if (_state == IDLE) return;

    unsigned long now     = millis();
    unsigned long elapsed = now - _stateStart;

    switch (_state)
    {
    case BEEP:
        // Ждём окончания бипа
        if (elapsed >= BEEP_MS)
        {
            _beepCount++;
            if (_beepCount >= BEEPS_PER_CYCLE)
            {
                _state = LONG_PAUSE;
            }
            else
            {
                _state = SHORT_PAUSE;
            }
            _stateStart = now;
        }
        break;

    case SHORT_PAUSE:
        // Короткая пауза между бипами
        if (elapsed >= SHORT_PAUSE_MS)
        {
            _state      = BEEP;
            _stateStart = now;
            M5Cardputer.Speaker.tone(FREQ, BEEP_MS);
        }
        break;

    case LONG_PAUSE:
        // Длинная пауза между циклами
        if (elapsed >= LONG_PAUSE_MS)
        {
            _beepCount  = 0;
            _state      = BEEP;
            _stateStart = now;
            M5Cardputer.Speaker.tone(FREQ, BEEP_MS);
        }
        break;

    default:
        break;
    }
}
