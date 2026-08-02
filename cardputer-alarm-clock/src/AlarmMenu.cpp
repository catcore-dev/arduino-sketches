#include "AlarmMenu.h"

// ============================================================
// Константы отрисовки меню
// ============================================================

// Позиционирование (меню рисуется по центру экрана 240×135)
static constexpr int MENU_W       = 210;
static constexpr int MENU_X       = (240 - MENU_W) / 2;   // 15

static constexpr int TITLE_Y      = 6;
static constexpr int FIELD_START_Y = 30;
static constexpr int FIELD_H      = 20;   // высота одного поля
static constexpr int LABEL_W      = 80;   // ширина области подписи

// Цветовая схема
static constexpr uint16_t C_OVERLAY     = 0x0001;  // почти чёрный (затемнение фона)
static constexpr uint16_t C_MENU_BG     = 0x2124;  // тёмно-серый фон меню
static constexpr uint16_t C_BORDER      = TFT_WHITE;
static constexpr uint16_t C_TITLE       = 0x07FF;  // Cyan
static constexpr uint16_t C_LABEL       = TFT_WHITE;
static constexpr uint16_t C_VALUE       = 0x07FF;  // Cyan
static constexpr uint16_t C_FOCUS_BG    = 0x001F;  // синий
static constexpr uint16_t C_FOCUS_LABEL = TFT_WHITE;
static constexpr uint16_t C_FOCUS_VALUE = TFT_YELLOW;
static constexpr uint16_t C_EXIT_TEXT   = TFT_GREEN;
static constexpr uint16_t C_HINT        = 0x7BEF;  // серый

// ============================================================
// Конструктор
// ============================================================

AlarmMenu::AlarmMenu(AlarmSettings& alarm, ClockDisplay& display)
    : _alarm(alarm)
    , _display(display)
    , _active(false)
    , _justOpened(false)
    , _currentField(FIELD_HOUR)
    , _editHour(7)
    , _editMin(0)
    , _editEnabled(false)
{
}

// ============================================================
// begin()
// ============================================================

void AlarmMenu::begin()
{
    // Меню не требует дополнительной инициализации
}

// ============================================================
// open() — скопировать настройки и показать меню
// ============================================================

void AlarmMenu::open()
{
    if (_active) return;

    _editHour    = _alarm.getAlarmHour();
    _editMin     = _alarm.getAlarmMin();
    _editEnabled = _alarm.getAlarmEnabled();
    _currentField = FIELD_HOUR;
    _justOpened  = true;
    _active      = true;

    _draw();
    Serial.printf("[AlarmMenu] Opened (current: %02d:%02d %s)\n",
                  _editHour, _editMin, _editEnabled ? "ON" : "OFF");
}

// ============================================================
// isActive()
// ============================================================

bool AlarmMenu::isActive() const
{
    return _active;
}

// ============================================================
// update() — обработка клавиатуры, вызывать из loop()
// ============================================================

void AlarmMenu::update()
{
    if (!_active) return;

    // Пропускаем первый кадр после open() — чтобы не подхватить
    // ту же клавишу Enter, которой открыли меню
    if (_justOpened)
    {
        _justOpened = false;
        return;
    }

    // Проверяем, изменилось ли состояние клавиатуры
    if (!M5Cardputer.Keyboard.isChange()) return;

    auto keys = M5Cardputer.Keyboard.keysState();

    // --- Enter: подтвердить и сохранить ---
    if (keys.enter)
    {
        _commitAndExit();
        return;
    }

    // --- Обработка символьных клавиш ---
    for (auto ch : keys.word)
    {
        // Выход без сохранения: C, c, #
        if (ch == 'c' || ch == 'C' || ch == '#')
        {
            _cancelAndExit();
            return;
        }

        switch (ch)
        {
        case 'w': case 'W':     // Up   — предыдущее поле
            _prevField();
            _draw();
            return;

        case 's': case 'S':     // Down — следующее поле
            _nextField();
            _draw();
            return;

        case 'a': case 'A':     // Left — уменьшить значение
            _decreaseValue();
            _draw();
            return;

        case 'd': case 'D':     // Right — увеличить значение
            _increaseValue();
            _draw();
            return;

        default:
            break;
        }
    }

    // Backspace / Del — тоже выход без сохранения
    if (keys.del)
    {
        _cancelAndExit();
    }
}

// ============================================================
// Приватные методы: отрисовка
// ============================================================

void AlarmMenu::_draw()
{
    // Координаты меню (центрированы)
    const int menuH = FIELD_START_Y + FIELD_H * FIELD_COUNT + 18;  // ~118
    const int menuY = (135 - menuH) / 2;

    // Затемняем фон
    M5Cardputer.Display.fillRect(0, 0, 240, 135, C_OVERLAY);

    // Фон меню
    M5Cardputer.Display.fillRect(MENU_X, menuY, MENU_W, menuH, C_MENU_BG);
    M5Cardputer.Display.drawRect(MENU_X, menuY, MENU_W, menuH, C_BORDER);

    // --- Заголовок ---
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(C_TITLE);
    const char* title = "ALARM";
    int titleX = MENU_X + (MENU_W - strlen(title) * 12) / 2;
    M5Cardputer.Display.setCursor(titleX, menuY + TITLE_Y);
    M5Cardputer.Display.print(title);

    // --- Поля ---
    int fy = menuY + FIELD_START_Y;

    // Поле: час
    char bufH[4];
    snprintf(bufH, sizeof(bufH), "%02d", _editHour);
    _drawField(MENU_X + 8, fy, "Hour", bufH, _currentField == FIELD_HOUR);
    fy += FIELD_H;

    // Поле: минуты
    char bufM[4];
    snprintf(bufM, sizeof(bufM), "%02d", _editMin);
    _drawField(MENU_X + 8, fy, "Min", bufM, _currentField == FIELD_MIN);
    fy += FIELD_H;

    // Поле: вкл/выкл
    const char* state = _editEnabled ? "ON" : "OFF";
    _drawField(MENU_X + 8, fy, "Alarm", state, _currentField == FIELD_ENABLED);
    fy += FIELD_H;

    // Поле: выход
    bool isExit = (_currentField == FIELD_EXIT);
    if (isExit)
    {
        M5Cardputer.Display.fillRect(MENU_X + 8, fy, MENU_W - 16, FIELD_H - 2, C_FOCUS_BG);
    }
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(isExit ? TFT_YELLOW : C_EXIT_TEXT);
    M5Cardputer.Display.setCursor(MENU_X + (MENU_W - 16 * 6) / 2, fy + 2);
    M5Cardputer.Display.print("[ SAVE ]");

    // --- Подсказка внизу ---
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(C_HINT);
    M5Cardputer.Display.setCursor(MENU_X + 6, menuY + menuH - 14);
    M5Cardputer.Display.print("W/S:nav  A/D:val");
    M5Cardputer.Display.setCursor(MENU_X + 6, menuY + menuH - 6);
    M5Cardputer.Display.print("Enter:ok  C:cancel");
}

void AlarmMenu::_drawField(int x, int y, const char* label, const char* value, bool isFocused)
{
    if (isFocused)
    {
        // Подсветка активного поля
        M5Cardputer.Display.fillRect(x, y, MENU_W - 16, FIELD_H - 2, C_FOCUS_BG);
        M5Cardputer.Display.setTextColor(C_FOCUS_LABEL);
    }
    else
    {
        M5Cardputer.Display.setTextColor(C_LABEL);
    }

    // Подпись
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setCursor(x + 4, y + 2);
    M5Cardputer.Display.print(label);

    // Значение
    M5Cardputer.Display.setTextColor(isFocused ? C_FOCUS_VALUE : C_VALUE);
    M5Cardputer.Display.setCursor(x + LABEL_W, y + 2);
    M5Cardputer.Display.print(value);
}

// ============================================================
// commit / cancel
// ============================================================

void AlarmMenu::_commitAndExit()
{
    _alarm.saveAlarmTime(_editHour, _editMin);
    _alarm.setAlarmEnabled(_editEnabled);
    _alarm.commit();

    Serial.printf("[AlarmMenu] Saved: %02d:%02d enabled=%d\n",
                  _editHour, _editMin, _editEnabled);

    _active = false;
    _display.clearScreen();
}

void AlarmMenu::_cancelAndExit()
{
    Serial.println("[AlarmMenu] Cancelled — no changes saved");
    _active = false;
    _display.clearScreen();
}

// ============================================================
// Навигация
// ============================================================

void AlarmMenu::_nextField()
{
    int n = (int)_currentField + 1;
    if (n >= FIELD_COUNT) n = 0;
    _currentField = (Field)n;
}

void AlarmMenu::_prevField()
{
    int p = (int)_currentField - 1;
    if (p < 0) p = FIELD_COUNT - 1;
    _currentField = (Field)p;
}

// ============================================================
// Изменение значений
// ============================================================

void AlarmMenu::_increaseValue()
{
    switch (_currentField)
    {
    case FIELD_HOUR:
        _editHour = (_editHour + 1) % 24;
        break;
    case FIELD_MIN:
        _editMin = (_editMin + 1) % 60;
        break;
    case FIELD_ENABLED:
        _editEnabled = !_editEnabled;
        break;
    default:
        break;
    }
}

void AlarmMenu::_decreaseValue()
{
    switch (_currentField)
    {
    case FIELD_HOUR:
        _editHour = (_editHour - 1 + 24) % 24;
        break;
    case FIELD_MIN:
        _editMin = (_editMin - 1 + 60) % 60;
        break;
    case FIELD_ENABLED:
        _editEnabled = !_editEnabled;
        break;
    default:
        break;
    }
}
