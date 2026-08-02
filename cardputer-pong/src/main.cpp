#include <M5Cardputer.h>
#include "pong_game.h"
#include "pong_display.h"

// Arrow key scan codes for M5Cardputer keyboard matrix
// Adjust these values if arrow keys don't respond correctly
#define KEY_UP_ARROW   0x5C
#define KEY_DOWN_ARROW 0x5D

M5GFX &display = M5Cardputer.Display;
PongGame game;
PongDisplay renderer;

void setup() {
    // Initialize M5Cardputer hardware
    M5Cardputer.begin();
    M5Cardputer.Display.init();

    // Initialize game and renderer
    game.init();
    renderer.init(display);
}

void loop() {
    M5Cardputer.update();

    // ---- Player 1 controls (W / S) ----
    if (M5Cardputer.Keyboard.isKeyPressed('w')) {
        game.setPaddle1Dir(-1);
    } else if (M5Cardputer.Keyboard.isKeyPressed('s')) {
        game.setPaddle1Dir(1);
    } else {
        game.setPaddle1Dir(0);
    }

    // ---- Player 2 controls (Arrow keys) ----
    if (M5Cardputer.Keyboard.isKeyPressed(KEY_UP_ARROW)) {
        game.setPaddle2Dir(-1);
    } else if (M5Cardputer.Keyboard.isKeyPressed(KEY_DOWN_ARROW)) {
        game.setPaddle2Dir(1);
    } else {
        game.setPaddle2Dir(0);
    }

    // Update game state (ball movement, collision, scoring)
    game.update();

    // ---- Render frame ----
    renderer.drawBackground();
    renderer.drawPaddle(8, game.getPaddle1Y(), TFT_WHITE);
    renderer.drawPaddle(226, game.getPaddle2Y(), TFT_WHITE);
    renderer.drawBall(game.getBallX(), game.getBallY(), TFT_GREEN);
    renderer.drawScore(game.getScore1(), game.getScore2(), TFT_WHITE);
    renderer.drawNet();

    // Maintain ~60 FPS
    delay(16);
}
