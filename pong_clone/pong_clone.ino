/*
 * Pong Clone
 *
 * Simple Pong Clone developed for Arduino Uno R4 WiFi devices
 * 
 * Sources:
 * https://gamedev.stackexchange.com/questions/4253/in-pong-how-do-you-calculate-the-balls-direction-when-it-bounces-off-the-paddl
 * (Used above source for the physics regarding the direction of the ball)
 */


#include "Arduino_LED_Matrix.h"
#include "animations.h"
#include <stdint.h>
#include <math.h>

#define DELAY 1             // Our delay for the paddles / main loop
#define WIDTH 12            // Width of matrix
#define HEIGHT 8            // Height of matrix
#define PADDLE_SIZE 4       // How big the paddles are

ArduinoLEDMatrix matrix;

// Consts
const int speed = 150;              // How fast the ball is going
const int max_score = 5;            // Maximum score for the game
const int max_angle = 5 * PI / 12;  // (75 degrees);

// Variables below are the ranges that our potentiometer moves the paddles by
const float initialRange = (float) 1024 / (float) (HEIGHT + 1 - PADDLE_SIZE);
int range;  // Int version of above equation

// Unused Structs
struct point {
  int x, y, xDir, yDir;
};

struct player {
  int read;                     // Potentiometer value
  int score;                    // Score for the player
  int paddle[PADDLE_SIZE];      // Players paddle
  bool check;                   // Boolean to check if player has won/lost a round  
};

struct point ball = {
  .x = 5,
  .y = 4,
  .xDir = -1,
  .yDir = 0
};

struct player player1;
struct player player2;

int k = 0;        // Timer for the ball
bool gameOver = false;  // Prints a game over screen

uint8_t frame[HEIGHT][WIDTH] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

void setup() {
  Serial.begin(9600);
  matrix.begin();
  player1.score = player2.score = 0;
  player1.check = player2.check = true;
  Serial.println(initialRange);
  range = (int) initialRange;
  if (initialRange - range > 0) {
    range += 1;
  }
  Serial.println(range);
}

bool checkPaddle(int paddle[]) {
  // Check if ball has hit paddle directly
  bool check = false;
  for (int i = 0; i < PADDLE_SIZE; i++) {
    if (paddle[i] == ball.y) {
      check = true;
      break;
    }
  }

  // Handle Edge cases here (If ball will hit paddle by the time it reaches zero)
  if (check == false) {
    int tempY = ball.y + ball.yDir;
    if (tempY == paddle[0] || tempY == paddle[PADDLE_SIZE - 1]) {
      check = true;
    } else {
      return check;
    }
  }

  // New x direction = negative old direction (matrix is too small to do anything more ambitious)
  ball.xDir = -ball.xDir;
  // Get direction that ball should go in y axis
  // Source: https://gamedev.stackexchange.com/questions/4253/in-pong-how-do-you-calculate-the-balls-direction-when-it-bounces-off-the-paddl
  float relativeIntersectY = (paddle[0] + (PADDLE_SIZE / 2)) - ball.y;
  float normalizedIntersectY = 2 * (relativeIntersectY / PADDLE_SIZE);
  float bounceAngle = normalizedIntersectY * max_angle;

  ball.yDir = -round(bounceAngle);
  return check;
}

void roundComplete() {
  if (!player1.check) {
    player2.score++;
    ball.xDir = -1; // Ball gets served to player1
  } else {
    player1.score++;
    ball.xDir = 1;  // Ball gets served to player2
  }
  Serial.print("Score:\t");
  Serial.print(player1.score);
  Serial.print("\t");
  Serial.println(player2.score);

  // Recenter ball
  frame[ball.y][ball.x] = 0;
  ball.x = 5;
  ball.y = 4;
  ball.yDir = 0;

  // Go to gameover state 
  gameOver = (player1.score == max_score || player2.score == max_score) ? true : false;

  delay(1000);
}

void winner() {
  if (player1.score > player2.score) {
    matrix.loadSequence(Player1Wins);
  } else if (player2.score > player1.score) {
    matrix.loadSequence(Player2Wins);
  }

  matrix.play(true);
}


void loop() {
  if (gameOver) {
    winner();
    delay(10000);
    return;
  }

  player1.read = analogRead(A0) / range;
  player2.read = analogRead(A1) / range;

  // Clear Paddles
  for (int i = 0; i < PADDLE_SIZE; i++) {
    frame[player1.paddle[i]][0] = 0;
    frame[player2.paddle[i]][WIDTH - 1] = 0;
  }
  
  // Print Paddles
  for (int i = 0; i < PADDLE_SIZE; i++) {
    player1.paddle[i] = player1.read + i;
    player2.paddle[i] = player2.read + i;

    frame[player1.paddle[i]][0] = 1;
    frame[player2.paddle[i]][WIDTH - 1] = 1;
  }

  k += DELAY;
  if (k % speed != 0) {
    matrix.renderBitmap(frame, HEIGHT, WIDTH);
    delay(DELAY);
    return;
  }
  // Handle ball
  frame[ball.y][ball.x] = 0;

  ball.x += ball.xDir;
  ball.y += ball.yDir;

  // If ball is out of bounds / if it hits the boundaries
  if (ball.y <= 0) {
    ball.y = 0;
    ball.yDir = -ball.yDir;
  } else if (ball.y >= HEIGHT - 1) {
    ball.y = HEIGHT - 1;
    ball.yDir = -ball.yDir;
  }

  // Collision Detection
  if (ball.x == 1) {
    player1.check = checkPaddle(player1.paddle);
  } else if (ball.x == 10) {
    player2.check = checkPaddle(player2.paddle);
  }

  frame[ball.y][ball.x] = 1;
  k = 0;
  matrix.renderBitmap(frame, HEIGHT, WIDTH);

  if (!player1.check || !player2.check) {
    roundComplete();
    player1.check = true;
    player2.check = true;
  }

  delay(DELAY);
}
