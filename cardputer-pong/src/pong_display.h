#pragma once

#include <M5GFX.h>

class PongDisplay {
public:
    PongDisplay() : _gfx(nullptr) {}

    void init(M5GFX &gfx) {
        _gfx = &gfx;
    }

    void drawBackground() {
        if (!_gfx) return;
        _gfx->fillScreen(TFT_BLACK);
        drawNet();
    }

    void drawPaddle(int x, int y, int color) {
        if (!_gfx) return;
        _gfx->fillRect(x, y, 6, 24, color);
    }

    void drawBall(int x, int y, int color) {
        if (!_gfx) return;
        _gfx->fillRect(x, y, 4, 4, color);
    }

    void drawScore(int s1, int s2, int color) {
        if (!_gfx) return;
        _gfx->setTextSize(4);
        _gfx->setTextColor(color);
        _gfx->setCursor(60, 8);
        _gfx->printf("%02d", s1);
        _gfx->setCursor(156, 8);
        _gfx->printf("%02d", s2);
    }

    void drawNet() {
        if (!_gfx) return;
        const int centerX = 120;
        const int dashHeight = 4;
        const int gapHeight = 4;
        const int totalHeight = 135;  // M5Cardputer display height
        int y = 0;
        while (y < totalHeight) {
            _gfx->fillRect(centerX - 1, y, 2, dashHeight, TFT_WHITE);
            y += dashHeight + gapHeight;
        }
    }

private:
    M5GFX *_gfx;
};
