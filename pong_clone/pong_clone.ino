#include "Arduino_LED_Matrix.h"
#include "Player1Wins.h"
#include "Player2Wins.h"
#include <stdint.h>

#define DIVIDE_CONST 205    // Value we divide our potentiometer values by to get our range
#define SPEED 150           // How fast the ball is going
#define DELAY 1             // Our delay for the paddles / main loop
#define WIDTH 12
#define HEIGHT 8

ArduinoLEDMatrix matrix;

// struct player {
//   int score;
//   int paddle[4];
//   bool check;
// };


int x = 5;        // x position of the ball
int y = 4;        // y position of the ball

int xDir = -1;    // Direction in the x axis of the ball
int yDir = 0;     // Direction in the y axis of the ball

int k = 0;        // Timer for the ball

bool check1 = true;     // Checks if game has been lost from paddle1
bool check2 = true;     // Checks if game has been lost from paddle2
bool gameOver = false;  // Prints a game over screen

int player1_Score = 0;
int player2_Score = 0;

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

int paddle1[4] = {2, 3, 4, 5};
int paddle2[4] = {2, 3, 4, 5};

void setup() {
  Serial.begin(9600);
  matrix.begin();
}

bool checkPaddle(int paddle[]) {
  bool check = false;
  int potential = -2;
  for (int i = 0; i < 4; i++) {
    potential += i;
    if (potential > 2) {
      potential = 2;
    }
    if (paddle[i] == y) {
      check = true;
      xDir = -xDir;
      yDir = potential;
      break;
    }
  }
  return check;
}

void roundComplete() {
  if (!check1) {
    player2_Score++;
    xDir = -1;
  } else {
    player1_Score++;
    xDir = 1;
  }
  Serial.print("Score:\t");
  Serial.print(player1_Score);
  Serial.print("\t");
  Serial.println(player2_Score);
  x = 5;
  y = 4;
  yDir = 0;

  if (player1_Score == 5 || player2_Score == 5) {
    gameOver = true;
  }

  delay(1000);
}

void winner() {
  if (player1_Score > player2_Score) {
    matrix.loadSequence(Player1Wins);
  } else if (player2_Score > player1_Score) {
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

  int player1 = analogRead(A0) / DIVIDE_CONST;
  int player2 = analogRead(A1) / DIVIDE_CONST;

  // Clear Paddles
  for (int i = 0; i < 4; i++) {
    frame[paddle1[i]][0] = 0;
    frame[paddle2[i]][11] = 0;
  }
  
  // Print Paddles
  for (int i = 0; i < 4; i++) {
    paddle1[i] = player1 + i;
    paddle2[i] = player2 + i;
    frame[paddle1[i]][0] = 1;
    frame[paddle2[i]][11] = 1;
  }

  k += DELAY;
  if (k % SPEED != 0) {
    matrix.renderBitmap(frame, HEIGHT, WIDTH);
    delay(DELAY);
    return;
  }
  // Handle ball
  frame[y][x] = 0;

  x += xDir;
  y += yDir;

  check1 = true;
  check2 = true;

  // If ball is out of bounds / if it hits the boundaries
  if (y <= 0) {
    y = 0;
    yDir = -yDir;
  } else if (y >= HEIGHT - 1) {
    y = 7;
    yDir = -yDir;
  }

  // If the ball is about to hit either one of the paddles
  if (x == 1) {
    check1 = checkPaddle(paddle1);
  } else if (x == 10) {
    check2 = checkPaddle(paddle2);
  }

  if (!check1 || !check2) {
    roundComplete();
  }

  frame[y][x] = 1;
  k = 0;
  matrix.renderBitmap(frame, HEIGHT, WIDTH);

  delay(DELAY);
}
