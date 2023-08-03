#include "Arduino_LED_Matrix.h"
#include "animations.h"
#include <stdint.h>

#define SPEED 150           // How fast the ball is going
#define DELAY 1             // Our delay for the paddles / main loop
#define WIDTH 12
#define HEIGHT 8
#define PADDLE_SIZE 4       // How big the paddles are
#define DIVIDE_CONST 205    // Value we divide our potentiometer values by to get our range for the paddles; this works for paddle_size = 4
#define MAX_SCORE 5         // Highest score a player can get before winning

ArduinoLEDMatrix matrix;

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
}

bool checkPaddle(int paddle[]) {
  bool check = false;
  int potential = -2;   // our potential yDir value
  for (int i = 0; i < PADDLE_SIZE; i++) {
    potential += i;
    if (potential > 2) {
      potential = 2;
    }
    if (paddle[i] == ball.y) {
      check = true;
      ball.xDir = -ball.xDir;
      ball.yDir = potential;
      break;
    }
  }
  return check;
}

void roundComplete() {
  if (!player1.check) {
    player2.score++;
    ball.xDir = -1;
  } else {
    player1.score++;
    ball.xDir = 1;
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
  if (player1.score == MAX_SCORE || player2.score == MAX_SCORE) {
    gameOver = true;
  }

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

  player1.read = analogRead(A0) / DIVIDE_CONST;
  player2.read = analogRead(A1) / DIVIDE_CONST;

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
  if (k % SPEED != 0) {
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
