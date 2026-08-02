#pragma once

#include <cstdint>

class PongGame {
public:
    PongGame() = default;

    void init() {
        ballX_ = fieldWidth_ / 2 - ballSize_ / 2;
        ballY_ = fieldHeight_ / 2 - ballSize_ / 2;
        ballVx_ = ballSpeed_;
        ballVy_ = ballSpeed_;
        paddle1Y_ = fieldHeight_ / 2 - paddleHeight_ / 2;
        paddle2Y_ = fieldHeight_ / 2 - paddleHeight_ / 2;
        score1_ = 0;
        score2_ = 0;
    }

    void update() {
        // Move ball
        ballX_ += ballVx_;
        ballY_ += ballVy_;

        // Top / bottom wall bounce
        if (ballY_ <= 0) {
            ballY_ = 0;
            ballVy_ = -ballVy_;
        } else if (ballY_ + ballSize_ >= fieldHeight_) {
            ballY_ = fieldHeight_ - ballSize_;
            ballVy_ = -ballVy_;
        }

        // Paddle 1 (left) collision
        if (ballVx_ < 0 &&
            ballX_ <= paddle1X_ + paddleWidth_ &&
            ballX_ + ballSize_ >= paddle1X_ &&
            ballY_ + ballSize_ >= paddle1Y_ &&
            ballY_ <= paddle1Y_ + paddleHeight_) {
            ballX_ = paddle1X_ + paddleWidth_;
            ballVx_ = -ballVx_;
        }

        // Paddle 2 (right) collision
        if (ballVx_ > 0 &&
            ballX_ + ballSize_ >= paddle2X_ &&
            ballX_ <= paddle2X_ + paddleWidth_ &&
            ballY_ + ballSize_ >= paddle2Y_ &&
            ballY_ <= paddle2Y_ + paddleHeight_) {
            ballX_ = paddle2X_ - ballSize_;
            ballVx_ = -ballVx_;
        }

        // Goal detection
        if (ballX_ + ballSize_ < 0) {
            // Ball passed left edge -> point for player 2
            ++score2_;
            resetBall();
        } else if (ballX_ > fieldWidth_) {
            // Ball passed right edge -> point for player 1
            ++score1_;
            resetBall();
        }

        // Move paddles
        paddle1Y_ += paddle1Dir_ * paddleSpeed_;
        paddle2Y_ += paddle2Dir_ * paddleSpeed_;

        // Clamp paddles
        if (paddle1Y_ < 0) paddle1Y_ = 0;
        if (paddle1Y_ + paddleHeight_ > fieldHeight_) paddle1Y_ = fieldHeight_ - paddleHeight_;
        if (paddle2Y_ < 0) paddle2Y_ = 0;
        if (paddle2Y_ + paddleHeight_ > fieldHeight_) paddle2Y_ = fieldHeight_ - paddleHeight_;
    }

    int getBallX()    const { return ballX_; }
    int getBallY()    const { return ballY_; }
    int getPaddle1Y() const { return paddle1Y_; }
    int getPaddle2Y() const { return paddle2Y_; }
    int getScore1()   const { return score1_; }
    int getScore2()   const { return score2_; }

    void movePaddle1(int dir) { paddle1Dir_ = dir; }
    void movePaddle2(int dir) { paddle2Dir_ = dir; }

private:
    void resetBall() {
        ballX_ = fieldWidth_ / 2 - ballSize_ / 2;
        ballY_ = fieldHeight_ / 2 - ballSize_ / 2;
        // Alternate vertical direction for variety
        ballVy_ = (ballVy_ > 0) ? ballSpeed_ : -ballSpeed_;
        // Serve toward the player who was scored against
        ballVx_ = (score1_ + score2_) % 2 == 0 ? -ballSpeed_ : ballSpeed_;
    }

    // Field dimensions (ST7789 on M5Cardputer)
    static constexpr int fieldWidth_  = 240;
    static constexpr int fieldHeight_ = 135;

    // Sizes
    static constexpr int ballSize_     = 4;
    static constexpr int paddleWidth_  = 6;
    static constexpr int paddleHeight_ = 24;

    // Positions
    static constexpr int paddle1X_ = 8;
    static constexpr int paddle2X_ = 226;

    // Speeds (pixels per frame)
    static constexpr int ballSpeed_    = 3;
    static constexpr int paddleSpeed_  = 3;

    // Ball state
    int ballX_ = 0;
    int ballY_ = 0;
    int ballVx_ = 0;
    int ballVy_ = 0;

    // Paddle state
    int paddle1Y_ = 0;
    int paddle2Y_ = 0;
    int paddle1Dir_ = 0;
    int paddle2Dir_ = 0;

    // Score
    int score1_ = 0;
    int score2_ = 0;
};
