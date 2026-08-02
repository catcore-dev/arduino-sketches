#include "M5Cardputer.h"

#define COLS 12
#define ROWS 6
#define CELL 20
#define GRID_Y 16
#define MAX_TAIL 256

struct Point {
    int8_t x, y;
};

Point tail[MAX_TAIL];
int tailLen;
int8_t foodX, foodY;
int8_t dirX, dirY;
int score;
bool gameOver;

void placeFood() {
    bool occupied;
    do {
        occupied = false;
        foodX = random(COLS);
        foodY = random(ROWS);
        for (int i = 0; i < tailLen; i++) {
            if (tail[i].x == foodX && tail[i].y == foodY) {
                occupied = true;
                break;
            }
        }
    } while (occupied);
}

void initGame() {
    tailLen = 3;
    tail[0].x = 3; tail[0].y = 3;
    tail[1].x = 2; tail[1].y = 3;
    tail[2].x = 1; tail[2].y = 3;
    dirX = 1; dirY = 0;
    score = 0;
    gameOver = false;
    placeFood();
}

void drawCell(int gx, int gy, uint16_t color) {
    M5Cardputer.Display.fillRect(gx * CELL, GRID_Y + gy * CELL, CELL, CELL, color);
}

void draw() {
    M5Cardputer.Display.fillScreen(BLACK);
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            M5Cardputer.Display.drawRect(x * CELL, GRID_Y + y * CELL, CELL, CELL, 0x222222);
        }
    }
    for (int i = 0; i < tailLen; i++) {
        drawCell(tail[i].x, tail[i].y, i == 0 ? 0x07E0 : 0x05A0);
    }
    drawCell(foodX, foodY, 0xF800);
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.setTextDatum(top_center);
    M5Cardputer.Display.drawString("Score: " + String(score), M5Cardputer.Display.width() / 2, 2);
}

void moveSnake() {
    int8_t nx = tail[0].x + dirX;
    int8_t ny = tail[0].y + dirY;

    if (nx < 0 || nx >= COLS || ny < 0 || ny >= ROWS) {
        gameOver = true;
        return;
    }
    for (int i = 1; i < tailLen; i++) {
        if (tail[i].x == nx && tail[i].y == ny) {
            gameOver = true;
            return;
        }
    }

    bool ate = (nx == foodX && ny == foodY);
    if (!ate) {
        for (int i = tailLen - 1; i > 0; i--) {
            tail[i] = tail[i - 1];
        }
    } else {
        for (int i = tailLen; i > 0; i--) {
            tail[i] = tail[i - 1];
        }
        tailLen++;
        score++;
        placeFood();
    }
    tail[0].x = nx;
    tail[0].y = ny;
}

void showGameOver() {
    M5Cardputer.Display.fillScreen(BLACK);
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.setTextDatum(middle_center);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.drawString("GAME OVER", M5Cardputer.Display.width() / 2, M5Cardputer.Display.height() / 2 - 20);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.drawString("Score: " + String(score), M5Cardputer.Display.width() / 2, M5Cardputer.Display.height() / 2 + 5);
    M5Cardputer.Display.drawString("Press Enter", M5Cardputer.Display.width() / 2, M5Cardputer.Display.height() / 2 + 25);
}

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    randomSeed(micros());
    initGame();
}

void loop() {
    M5Cardputer.update();
    if (M5Cardputer.Keyboard.isKeyPressed('w') && dirY == 0) { dirX = 0; dirY = -1; }
    if (M5Cardputer.Keyboard.isKeyPressed('s') && dirY == 0) { dirX = 0; dirY = 1; }
    if (M5Cardputer.Keyboard.isKeyPressed('a') && dirX == 0) { dirX = -1; dirY = 0; }
    if (M5Cardputer.Keyboard.isKeyPressed('d') && dirX == 0) { dirX = 1; dirY = 0; }

    if (gameOver) {
        showGameOver();
        if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
            initGame();
        }
        delay(50);
        return;
    }

    moveSnake();
    draw();
    delay(200);
}
