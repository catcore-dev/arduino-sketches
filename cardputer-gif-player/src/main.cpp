#include <M5Cardputer.h>
#include <SD.h>
#include <AnimatedGIF.h>
#include <vector>
#include <string>

// --- AnimatedGIF callbacks ---
int32_t GIF_ReadCB(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen) {
    File *file = (File *)pFile->fHandle;
    return file->read(pBuf, iLen);
}

int32_t GIF_SeekCB(GIFFILE *pFile, int32_t iPosition) {
    File *file = (File *)pFile->fHandle;
    return file->seek(iPosition);
}

void *GIF_OpenCB(const char *szFilename, int32_t *pFileSize) {
    File file = SD.open(szFilename);
    if (!file) return nullptr;
    *pFileSize = file.size();
    // We store the File object — but we need to keep it alive.
    // Return a heap-allocated copy; close callback will free it.
    File *pFile = new File(file);
    return (void *)pFile;
}

void GIF_CloseCB(void *pHandle) {
    if (pHandle) {
        File *file = (File *)pHandle;
        file->close();
        delete file;
    }
}

// --- Draw callback ---
void GIF_DrawCB(GIFDRAW *pDraw) {
    // In COOKED mode with RGB565_LE, pPixels points to 16-bit rendered pixels per line
    // pDraw->y = current line number within this frame
    // pDraw->iX, pDraw->iY = top-left corner of this frame on canvas
    uint16_t *dst = (uint16_t *)pDraw->pPixels;
    M5Cardputer.Display.pushImage(
        pDraw->iX, pDraw->iY + pDraw->y,
        pDraw->iWidth, 1,
        dst
    );
}

// --- Globals ---
AnimatedGIF gif;
std::vector<std::string> fileList;
int selectedIndex = 0;
int scrollOffset = 0;
bool gifMode = false;

const int SCREEN_W = 320;
const int SCREEN_H = 240;
const int LINE_H = 20;
const int MAX_VISIBLE = SCREEN_H / LINE_H; // 12

void scanFiles() {
    fileList.clear();
    File root = SD.open("/");
    if (!root) return;

    File entry;
    while ((entry = root.openNextFile())) {
        String name = entry.name();
        if (!entry.isDirectory() && name.endsWith(".gif")) {
            fileList.push_back(std::string(name.c_str()));
        }
        entry.close();
    }
    root.close();
    selectedIndex = 0;
    scrollOffset = 0;
}

void drawMenu() {
    M5Cardputer.Display.fillScreen(BLACK);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(0, 0);

    if (fileList.empty()) {
        M5Cardputer.Display.drawString("No .gif files found", 10, 10);
        M5Cardputer.Display.drawString("Put .gif files on SD", 10, 30);
        return;
    }

    int visibleCount = (fileList.size() < MAX_VISIBLE) ? fileList.size() : MAX_VISIBLE;
    for (int i = 0; i < visibleCount; i++) {
        int idx = scrollOffset + i;
        if (idx >= (int)fileList.size()) break;

        String line = String(fileList[idx].c_str());
        int y = i * LINE_H;
        if (idx == selectedIndex) {
            M5Cardputer.Display.fillRect(0, y, SCREEN_W, LINE_H, WHITE);
            M5Cardputer.Display.setTextColor(BLACK, WHITE);
        } else {
            M5Cardputer.Display.setTextColor(WHITE, BLACK);
        }
        M5Cardputer.Display.drawString(line, 5, y + 2);
    }
}

void playGIF(const char *filename) {
    // Build full path: SD card root + filename
    String fullPath = String("/") + String(filename);

    M5Cardputer.Display.fillScreen(BLACK);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setCursor(10, 10);
    M5Cardputer.Display.printf("Loading: %s", filename);

    gif.begin(GIF_PALETTE_RGB565_LE);
    gif.setDrawType(GIF_DRAW_COOKED);

    if (!gif.open(fullPath.c_str(), GIF_OpenCB, GIF_CloseCB, GIF_ReadCB, GIF_SeekCB, GIF_DrawCB)) {
        M5Cardputer.Display.fillScreen(BLACK);
        M5Cardputer.Display.setCursor(10, 10);
        M5Cardputer.Display.printf("Open failed: %d", gif.getLastError());
        delay(2000);
        return;
    }

    int gifW = gif.getCanvasWidth();
    int gifH = gif.getCanvasHeight();

    // Center on screen
    int offX = (SCREEN_W - gifW) / 2;
    int offY = (SCREEN_H - gifH) / 2;
    if (offX < 0) offX = 0;
    if (offY < 0) offY = 0;

    gifMode = true;
    int frameDelay = 0;

    while (gifMode) {
        int result = gif.playFrame(true, &frameDelay, nullptr);
        if (result <= 0) {
            // End of GIF or error — restart (reset seek + close/open)
            gif.close();
            gif.begin(GIF_PALETTE_RGB565_LE);
            gif.setDrawType(GIF_DRAW_COOKED);
            if (!gif.open(fullPath.c_str(), GIF_OpenCB, GIF_CloseCB, GIF_ReadCB, GIF_SeekCB, GIF_DrawCB)) {
                break;
            }
        }

        // Check for ESC (Backspace or DEL)
        M5Cardputer.update();
        if (M5Cardputer.Keyboard.isChange()) {
            auto &state = M5Cardputer.Keyboard.keysState();
            if (state.del) {
                gifMode = false;
            }
        }
    }

    gif.close();
    gifMode = false;
}

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.fillScreen(BLACK);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(10, 10);
    M5Cardputer.Display.printf("GIF Player");

    // Init SD card: CS=GPIO_NUM_4, SPI, 25MHz
    if (!SD.begin(GPIO_NUM_4, SPI, 25000000)) {
        M5Cardputer.Display.setCursor(10, 30);
        M5Cardputer.Display.printf("SD init fail!");
    } else {
        M5Cardputer.Display.setCursor(10, 30);
        M5Cardputer.Display.printf("Scanning SD...");
        scanFiles();
        M5Cardputer.Display.printf(" Found: %d GIFs", fileList.size());
        delay(500);
    }
}

void loop() {
    M5Cardputer.update();

    if (!gifMode) {
        drawMenu();
    }

    if (M5Cardputer.Keyboard.isChange()) {
        auto &state = M5Cardputer.Keyboard.keysState();

        // W / UP (via shift logic — W key: row1col1)
        bool wPressed = M5Cardputer.Keyboard.isKeyPressed('w') || M5Cardputer.Keyboard.isKeyPressed('W');
        // S / DOWN
        bool sPressed = M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S');

        if (wPressed && selectedIndex > 0) {
            selectedIndex--;
            if (selectedIndex < scrollOffset) scrollOffset = selectedIndex;
        } else if (sPressed && selectedIndex < (int)fileList.size() - 1) {
            selectedIndex++;
            if (selectedIndex >= scrollOffset + MAX_VISIBLE) scrollOffset = selectedIndex - MAX_VISIBLE + 1;
        } else if (state.enter || state.space) {
            if (!fileList.empty() && selectedIndex < (int)fileList.size()) {
                playGIF(fileList[selectedIndex].c_str());
                // Restore screen after playback
                M5Cardputer.Display.setRotation(1);
                M5Cardputer.Display.fillScreen(BLACK);
            }
        } else if (state.del) {
            // Backspace = exit / no-op in menu
        }
    }

    delay(30);
}
