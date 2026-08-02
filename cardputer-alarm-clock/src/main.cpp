#include <M5Cardputer.h>
#include "RTCClock.h"
#include "ClockDisplay.h"
#include "AlarmSettings.h"
#include "AlarmMenu.h"
#include "AlarmBuzzer.h"

// ============================================================
// Global objects / Глобальные объекты
// ============================================================
RTCClock      rtc;              ///< RTC BM8563 (PCF8563) I2C
ClockDisplay  clkDisplay;       ///< Display rendering (часы + дата + статус)
AlarmSettings alarmSettings;    ///< Alarm settings in NVS (настройки в NVS)
AlarmMenu     menu(alarmSettings, clkDisplay);  ///< Alarm settings menu (меню настройки)
AlarmBuzzer   buzzer;           ///< Buzzer sound signal (звуковой сигнал)

// ============================================================
// Cached alarm state — updated when menu closes
// Кэш состояния будильника — обновляется при закрытии меню
// ============================================================
static bool  s_alarmSet   = false;   ///< Alarm enabled? (будильник включён?)
static int   s_alarmHour  = 7;       ///< Alarm hour (час срабатывания)
static int   s_alarmMin   = 0;       ///< Alarm minute (минута срабатывания)

/// Day of month when alarm last fired.
/// Used to prevent repeated triggering within the same day.
/// День месяца последнего срабатывания — чтобы не звонить повторно в тот же день.
static int s_alarmTriggerDay = -1;

// ============================================================
// Screen sleep state / Режим сна экрана
// ============================================================
static bool          s_screenSleep    = false;        ///< Screen is dimmed/sleeping?
static unsigned long s_lastActivity   = 0;            ///< millis() of last keypress
static constexpr unsigned long SLEEP_TIMEOUT_MS = 30000;  ///< 30 sec idle → sleep

// ============================================================
// setup() — hardware init, load settings, initial draw
// ============================================================
void setup()
{
    // --- M5Cardputer initialization ---
    M5Cardputer.begin();
    Serial.begin(115200);

    // --- Display ---
    clkDisplay.begin();

    // --- Serial debug ---
    Serial.println("\n=== Cardputer Alarm Clock ===");

    // --- RTC BM8563 ---
    if (rtc.begin())
    {
        Serial.println("RTC BM8563 detected.");
        if (rtc.isVoltageLow())
        {
            // Power loss detected — set a default time
            // Обнаружен сброс питания — устанавливаем время по умолчанию
            Serial.println("RTC voltage low — setting default time (2026-06-12 12:00)");
            rtc.setDateTime(2026, 6, 12, 12, 0, 0);
        }
    }
    else
    {
        Serial.println("ERROR: RTC BM8563 not found!");
        M5Cardputer.Display.setTextSize(2);
        M5Cardputer.Display.setTextColor(TFT_RED);
        M5Cardputer.Display.setCursor(10, 60);
        M5Cardputer.Display.print("RTC FAIL");
    }

    // --- AlarmSettings from NVS ---
    alarmSettings.begin();
    s_alarmSet  = alarmSettings.isAlarmSet();
    s_alarmHour = alarmSettings.getAlarmHour();
    s_alarmMin  = alarmSettings.getAlarmMin();
    Serial.printf("Alarm from NVS: %s %02d:%02d\n",
                  s_alarmSet ? "ON" : "OFF", s_alarmHour, s_alarmMin);

    // --- Alarm menu ---
    menu.begin();
    Serial.println("Press ENTER, '*' or 'M' to open alarm settings.");

    // --- Buzzer ---
    buzzer.begin();
    Serial.println("Buzzer ready.");

    // --- Initial clock display ---
    if (rtc.isPresent())
    {
        struct tm t = rtc.getDateTime();
        clkDisplay.drawDate(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_wday);
        clkDisplay.drawTime(t.tm_hour, t.tm_min, t.tm_sec);
        clkDisplay.drawAlarmStatus(s_alarmSet, s_alarmHour, s_alarmMin);
    }

    s_lastActivity = millis();
}

// ============================================================
// loop() — main cycle: clock, alarm check, keyboard, sleep
// ============================================================
void loop()
{
    M5Cardputer.update();
    unsigned long now = millis();

    // ================================================================
    // 1. Keyboard input — track activity, wake from sleep
    //    Обработка клавиатуры — отслеживаем активность и пробуждение
    // ================================================================
    bool anyKey = false;

    if (M5Cardputer.Keyboard.isChange())
    {
        auto keys = M5Cardputer.Keyboard.keysState();

        // Любое нажатие: Enter, Del, символы, модификаторы
        // Any press: Enter, Del, printable chars, modifiers
        if (keys.enter || keys.del || keys.space || keys.tab ||
            !keys.word.empty() ||
            keys.fn || keys.shift || keys.ctrl || keys.opt || keys.alt)
        {
            anyKey = true;
            s_lastActivity = now;
        }

        // === Screen was asleep → any key wakes up ===
        // Экран спал → любая клавиша пробуждает
        if (s_screenSleep && anyKey)
        {
            s_screenSleep = false;
            M5Cardputer.Display.wakeup();
            clkDisplay.clearScreen();
            Serial.println("Wake up — screen on");
            return;  // Next loop will redraw clock
        }

        // === Menu is active → forward keypress to menu ===
        // Меню активно → передаём управление меню
        if (menu.isActive())
        {
            menu.update();
            return;
        }

        // === Normal mode: open menu on Enter, '*' or 'M' ===
        // Обычный режим: открыть меню по Enter, '*' или 'M'
        if (keys.enter)
        {
            menu.open();
            return;
        }
        for (auto ch : keys.word)
        {
            if (ch == 'm' || ch == 'M' || ch == '*')
            {
                menu.open();
                return;
            }
        }
    }

    // ================================================================
    // 2. Menu just closed — sync cache from AlarmSettings
    //    Меню закрылось — синхронизируем кэш с AlarmSettings
    // ================================================================
    static bool wasMenuActive = false;

    if (menu.isActive())
    {
        wasMenuActive = true;
        return;  // Menu handles its own display
    }

    if (wasMenuActive)
    {
        wasMenuActive = false;
        s_alarmSet  = alarmSettings.isAlarmSet();
        s_alarmHour = alarmSettings.getAlarmHour();
        s_alarmMin  = alarmSettings.getAlarmMin();
        clkDisplay.clearScreen();
        Serial.printf("Menu closed. Alarm: %s %02d:%02d\n",
                      s_alarmSet ? "ON" : "OFF", s_alarmHour, s_alarmMin);
    }

    // ================================================================
    // 3. Buzzer update (non-blocking pattern)
    //    Обновление буззера (неблокирующий звуковой паттерн)
    // ================================================================
    buzzer.update();

    // Stop alarm on any keypress
    // Отключение будильника по любой клавише
    if (buzzer.isPlaying() && anyKey)
    {
        buzzer.stopAlarm();
        Serial.println("Alarm stopped by keypress");
    }

    // ================================================================
    // 4. Sleep mode — auto-dim after inactivity timeout
    //    Режим сна — автоматическое затемнение при бездействии
    // ================================================================
    if (!s_screenSleep && !buzzer.isPlaying() && !menu.isActive())
    {
        if (now - s_lastActivity >= SLEEP_TIMEOUT_MS)
        {
            s_screenSleep = true;
            // Visual overlay + hardware sleep
            clkDisplay.drawSleepOverlay();
            M5Cardputer.Display.sleep();
            Serial.println("Screen sleep — 30s idle");
        }
    }

    // ================================================================
    // 5. Main tick — once per second
    //    Основной тактовый цикл — раз в секунду
    // ================================================================
    static unsigned long lastTick = 0;
    if (now - lastTick < 1000) return;
    lastTick = now;

    if (!rtc.isPresent()) return;

    struct tm t = rtc.getDateTime();
    int hour = t.tm_hour;
    int min  = t.tm_min;
    int sec  = t.tm_sec;
    int day  = t.tm_mday;

    // ================================================================
    // 6. Update display (only if not sleeping)
    //    Обновление дисплея (только если не в режиме сна)
    // ================================================================
    if (!s_screenSleep)
    {
        int year  = t.tm_year + 1900;
        int month = t.tm_mon + 1;
        int wday  = t.tm_wday;
        clkDisplay.drawTime(hour, min, sec);
        clkDisplay.drawDate(year, month, day, wday);
        clkDisplay.drawAlarmStatus(s_alarmSet, s_alarmHour, s_alarmMin);
    }

    // ================================================================
    // 7. Alarm trigger check
    //    Проверка срабатывания будильника
    // ================================================================
    // The trigger flag automatically resets on a new day:
    // s_alarmTriggerDay stores the day the alarm last fired.
    // If today is a different day, day != s_alarmTriggerDay → alarm can fire.
    // После срабатывания s_alarmTriggerDay = day, и в тот же день
    // alarm не сработает повторно. На следующий день — день изменится,
    // и alarm снова сможет сработать.
    if (s_alarmSet && !buzzer.isPlaying() && sec == 0)
    {
        if (hour == s_alarmHour && min == s_alarmMin && day != s_alarmTriggerDay)
        {
            // Time matches — fire the alarm!
            // Время совпало — запускаем будильник!
            buzzer.playAlarm();
            s_alarmTriggerDay = day;

            // Wake screen if it was asleep
            // Пробуждаем экран, если он спал
            if (s_screenSleep)
            {
                s_screenSleep = false;
                M5Cardputer.Display.wakeup();
                clkDisplay.clearScreen();
                Serial.println("Alarm woke up the screen!");
            }

            Serial.printf(">> ALARM! %02d:%02d <<\n", hour, min);
        }
    }

    // ================================================================
    // 8. Serial log every 5 seconds
    //    Лог в Serial раз в 5 секунд
    // ================================================================
    static unsigned long lastLog = 0;
    if (now - lastLog >= 5000)
    {
        lastLog = now;
        char buf[80];
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d%s",
                 t.tm_year + 1900, t.tm_mon + 1, day, hour, min, sec,
                 s_screenSleep ? " [SLEEP]" : "");
        Serial.println(buf);
    }
}
