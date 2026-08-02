// Notes App for M5Cardputer — terminal-style
#include <Arduino.h>
#include <M5Cardputer.h>

// ============================================================
// Constants
// ============================================================
const int MAX_NOTES   = 50;
const int NOTE_LEN    = 60;
const int INPUT_LEN   = 80;

const int TOP_BAR_H   = 12;    // top bar height in pixels
const int LINE_H      = 8;     // font height at textSize=1 (6x8)
const int INPUT_Y     = 222;   // input area Y start
const int SEP_Y       = 220;   // separator Y
const int NOTES_Y     = 14;    // first note line Y

const uint16_t COL_BG       = TFT_BLACK;
const uint16_t COL_GREEN    = TFT_GREEN;
const uint16_t COL_WHITE    = TFT_WHITE;
const uint16_t COL_TOPBAR   = 0x001F;  // dark blue
const uint16_t COL_SEP      = 0x3333;  // grey separator

// ============================================================
// State
// ============================================================
char notes[MAX_NOTES][NOTE_LEN];
int  nextNoteIdx = 0;     // where to write next note
int  totalNotes  = 0;     // total notes added (includes overwritten)

char inputBuf[INPUT_LEN];
int  inputLen    = 0;

bool cursorVis  = true;
unsigned long lastCursorMs = 0;

// How many notes to skip at top (for scrolling — future use)
int scrollOffset = 0;

// ============================================================
// Forward declarations
// ============================================================
void saveNote();
void clearAllNotes();
void clearInput();
void drawTopBar();
void drawNotes();
void drawInputLine(bool showCursor);
void handleKeyboard();

// ============================================================
// Note functions
// ============================================================
void saveNote() {
  if (inputLen == 0) return;
  // Copy input to notes buffer
  int len = inputLen;
  if (len >= NOTE_LEN) len = NOTE_LEN - 1;
  memcpy(notes[nextNoteIdx], inputBuf, len);
  notes[nextNoteIdx][len] = '\0';
  nextNoteIdx = (nextNoteIdx + 1) % MAX_NOTES;
  totalNotes++;
  ets_printf("NOTE_SAVED [%d]: %s\n", (nextNoteIdx - 1 + MAX_NOTES) % MAX_NOTES, notes[(nextNoteIdx - 1 + MAX_NOTES) % MAX_NOTES]);
  clearInput();
}

void clearAllNotes() {
  for (int i = 0; i < MAX_NOTES; i++) {
    notes[i][0] = '\0';
  }
  nextNoteIdx = 0;
  totalNotes = 0;
  scrollOffset = 0;
  ets_printf("ALL_NOTES_CLEARED\n");
}

void clearInput() {
  inputBuf[0] = '\0';
  inputLen = 0;
}

// ============================================================
// Display functions
// ============================================================
void drawTopBar() {
  auto& d = M5Cardputer.Display;
  d.fillRect(0, 0, 320, TOP_BAR_H, COL_TOPBAR);
  d.setTextColor(COL_WHITE, COL_TOPBAR);
  d.setTextSize(1);
  d.setCursor(4, 2);
  d.print("cardputer-notes");
  // Show note count on right
  d.setCursor(320 - 80, 2);
  d.printf("%d notes", totalNotes);
}

void drawNotes() {
  auto& d = M5Cardputer.Display;
  // Clear notes area
  d.fillRect(0, NOTES_Y, 320, SEP_Y - NOTES_Y, COL_BG);

  // Calculate which notes to show
  int maxLines = (SEP_Y - NOTES_Y) / LINE_H;  // ~25 lines
  int count = totalNotes < MAX_NOTES ? totalNotes : MAX_NOTES;  // visible notes
  if (count > maxLines) count = maxLines;

  int displayIdx;  // index in circular buffer to display
  if (totalNotes <= MAX_NOTES) {
    // Not yet wrapped: show 0..totalNotes-1
    displayIdx = 0;
  } else {
    // Wrapped: show the most recent MAX_NOTES
    // The "newest" note is at (nextNoteIdx - 1 + MAX_NOTES) % MAX_NOTES
    // We want to show up to 'maxLines' notes ending at the newest
    displayIdx = nextNoteIdx;  // this is the oldest in the circular buffer
  }

  d.setTextSize(1);
  for (int i = 0; i < count; i++) {
    int idx = (displayIdx + i) % MAX_NOTES;
    if (notes[idx][0] == '\0') continue;  // skip empty

    int y = NOTES_Y + i * LINE_H;
    int noteNum = totalNotes - count + i + 1;

    // Green note number
    d.setTextColor(COL_GREEN, COL_BG);
    d.setCursor(4, y);
    d.printf("%02d ", noteNum % 100);

    // White note text (truncated to fit)
    d.setTextColor(COL_WHITE, COL_BG);
    // Available width: 320 - 4 - 24 (for number) = 292 pixels
    // At 6px per char: ~48 chars
    int maxChars = (320 - 28) / 6;
    char buf[64];
    int slen = strlen(notes[idx]);
    if (slen > maxChars) slen = maxChars;
    memcpy(buf, notes[idx], slen);
    buf[slen] = '\0';
    d.print(buf);
  }
}

void drawInputLine(bool showCursor) {
  auto& d = M5Cardputer.Display;
  // Separator line
  d.drawFastHLine(0, SEP_Y, 320, COL_SEP);

  // Clear input area
  d.fillRect(0, INPUT_Y, 320, 240 - INPUT_Y, COL_BG);

  d.setTextSize(1);
  // Prompt
  d.setTextColor(COL_GREEN, COL_BG);
  d.setCursor(4, INPUT_Y + 2);
  d.print("> ");

  // Input text — print as a single string, not char-by-char
  d.setTextColor(COL_WHITE, COL_BG);
  int maxChars = (320 - 24) / 6;  // ~49 chars including cursor
  int slen = inputLen;
  if (slen > maxChars) slen = maxChars;
  if (slen > 0) {
    char tmp[64];
    memcpy(tmp, inputBuf, slen);
    tmp[slen] = '\0';
    d.print(tmp);
  }

  // Cursor — just print at current cursor position
  if (showCursor) {
    d.setTextColor(COL_GREEN, COL_BG);
    d.print("_");
  }
}

void drawScreen() {
  drawTopBar();
  drawNotes();
  drawInputLine(cursorVis);
}

// ============================================================
// Keyboard handling
// ============================================================
void handleKeyboard() {
  M5Cardputer.update();
  auto keys = M5Cardputer.Keyboard.keysState();
  bool changed = false;

  // Regular chars
  for (auto ch : keys.word) {
    if (inputLen < INPUT_LEN - 1) {
      inputBuf[inputLen++] = ch;
      inputBuf[inputLen] = '\0';
      changed = true;
      ets_printf("CHAR: '%c' (0x%02X) buf=[%s]\n", ch, (unsigned char)ch, inputBuf);
    }
  }

  // Delete
  if (keys.del && inputLen > 0) {
    inputBuf[--inputLen] = '\0';
    changed = true;
    ets_printf("DEL buf=[%s]\n", inputBuf);
  }

  // Enter — save note
  if (keys.enter) {
    ets_printf("ENTER: [%s]\n", inputBuf);
    saveNote();
    changed = true;
  }

  // Tab — clear all notes
  if (keys.tab) {
    ets_printf("TAB\n");
    clearAllNotes();
    changed = true;
  }

  // Fn — clear input
  if (keys.fn) {
    ets_printf("FN\n");
    if (inputLen > 0) {
      clearInput();
      changed = true;
    }
  }

  // Redraw if something changed
  if (changed) {
    drawScreen();
  }
}

// ============================================================
// Setup & Loop
// ============================================================
void setup() {
  auto cfg = M5.config();
  cfg.internal_mic = false;
  cfg.internal_spk = false;
  cfg.external_spk = false;
  cfg.external_rtc = false;
  cfg.serial_baudrate = 0;  // Don't touch Serial (it's broken on this HW for UART0)
  M5Cardputer.begin(cfg, true);  // true = enable keyboard

  // Init display
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.fillScreen(COL_BG);

  // Draw initial screen
  drawTopBar();
  drawNotes();
  drawInputLine(true);

  // Serial not usable on this board (USB_MODE=1 conflict with CH9102F UART)
  // Use ets_printf() for debug output instead
  Serial.begin(115200);  // dummy — placate compiler
  ets_printf("\n=== CARDPUTER NOTES BOOT OK ===\n");
  ets_printf("Heap: %u\n", ESP.getFreeHeap());
}

void loop() {
  handleKeyboard();

  // Blink cursor every 500ms
  unsigned long now = millis();
  if (now - lastCursorMs >= 500) {
    lastCursorMs = now;
    cursorVis = !cursorVis;
    // Only redraw the input line, not the whole screen
    drawInputLine(cursorVis);
  }

  delay(30);
}
