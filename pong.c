/* FINAL PROJECT - MICROPROCESSOR PROGRAMMING CLASS
** B.S. IN INFORMATION SYSTEMS - CATÓLICA DE SANTA CATARINA
** NATÃ BARBOSA
MCU:                  PIC16F877A
KIT:  PICKIT PIC18F - MICROGENIOS
==============================================================+==============
KEYS IN USE [0N] / [OFF]:
1- GLCD / LCD = [ON]    2- RX = [OFF]      3- TX = [OFF]     4- REL1 = [OFF]
5- REL2 = [OFF]         6- SCK = [OFF]     7- SDA = [OFF]    8- RTC [OFF]
9- LED1 = [OFF]         0- LED2 = [OFF]
1- DIS1 = [OFF]         2 - DIS2 = [OFF]   3- DIS3 = [OFF]   4- DIS4 = [OFF]
5- INFR = [OFF]         6- RESIS = [OFF]   7- TEMP = [OFF]   8- VENT = [OFF]
9- AN0 = [ON]           0- AN1 = [ON]      a- BUZZER JUMPER CLOSED
=============================================================================
DESCRIPTION: This program runs a 2-player PONG game on the LCD display
with analog input in AN0 and AN1. Score goes until 7 by default.
============================================================================ */

/* ********************************** */
/* **** LCD SETTINGS **** */
/* ********************************** */
sbit LCD_RS at RE2_bit;
sbit LCD_EN at RE1_bit;
sbit LCD_D4 at RD4_bit;
sbit LCD_D5 at RD5_bit;
sbit LCD_D6 at RD6_bit;
sbit LCD_D7 at RD7_bit;

sbit LCD_RS_Direction at TRISE2_bit;
sbit LCD_EN_Direction at TRISE1_bit;
sbit LCD_D4_Direction at TRISD4_bit;
sbit LCD_D5_Direction at TRISD5_bit;
sbit LCD_D6_Direction at TRISD6_bit;
sbit LCD_D7_Direction at TRISD7_bit;
// End LCD Settings

// Timer
void WaitForSec(float t){
     int count = 0;
     while (1){
         while (! INTCON.T0IF);
               TMR0 = 0;
               INTCON.T0IF = 0; // Reset bit TMR1IF
               count ++;

               if (count == 30 * t){
                    count = 0; break;
               }
         }

}

/* ********************************** */
/* **** PADDLE CONTROL GLOBALS **** */
/* ********************************** */

// Max-rows based on size of LCD
const int MAX_LIMIT_Y = 4;
// Special Char for LCD Paddles
const int PADDLE_1_LCD_CMD = 80;
const int PADDLE_2_LCD_CMD = 88;
const int PADDLE_1_LCD_CHAR_ADDRESS = 2;
const int PADDLE_2_LCD_CHAR_ADDRESS = 3;

// ENUM for paddle direction
enum Paddle_Direction
{
    up,
    down
};
// paddle positions
int paddlePos[] = {0, MAX_LIMIT_Y};
enum Paddle_Direction paddleDir[] = {down, up};

/* ********************************** */
/* **** BALL CONTROL VARIABLES **** */
/* ********************************** */

// initial vertical position of ball
int ballInnerPointer = 3;
// initial horizontal position of ball - 16 = first column of cell
int ballInnerPos = 16;
// ball speed (direction): [0] = 1 X axis position and [1] = 1 Y axis position
int ballSpeed[] = {1,1};
// first cell of game area
const int BALL_CELL_START = 2;
// last cell of game area
const int BALL_CELL_END = 5;
// place ball on first cell of game area
int ballCurrentCell = BALL_CELL_START;

/* ********************************** */
/* **** GAME CONTROL VARIABLES **** */
/* ********************************** */

char gameTitle[] = "P O N G !";
int score[] = { 0, 0};
int checkScoreAttacker = 0;
int checkScoreDefender = 1;
int controlPaddlePos[] = {0, 0};
const int MAX_SCORE = 7;

// show game title and score
void showGameInfo() {
    Lcd_Out(1,8, gameTitle);
    Lcd_Chr(2, 8, score[0] + '0');
    Lcd_Chr(2, 10, 'x');
    Lcd_Chr(2, 12, score[1] + '0');
}

/* ********************************** */
/* **** PADDLE BEHAVIOR **** */
/* ********************************** */

// move paddle up
void paddleUp(int which) {
   if (paddlePos[which] > 0) {
       paddlePos[which] = paddlePos[which] - 1;
   }
}

// move paddle down
void paddleDown(int which) {
   if (paddlePos[which] < MAX_LIMIT_Y) {
       paddlePos[which] = paddlePos[which] + 1;
   }
}
// has paddle hit top?
int paddleHitTop(int which) {
   if (paddlePos[which] == 0) {
      return 1;
   }
   return 0;
}

// has paddle hit bottom?
int paddleHitBottom(int which) {
   if (paddlePos[which] == MAX_LIMIT_Y) {
      return 1;
   }
   return 0;
}

// LCD cmd for each paddle
int getPaddleLCDCmd(int which) {
    if (which == 0) {
       return PADDLE_1_LCD_CMD;
    } else {
       return PADDLE_2_LCD_CMD;
    }
}

// LCD char address for each paddle
int getPaddleLCDCharAddress(int which) {
    if (which == 0) {
       return PADDLE_1_LCD_CHAR_ADDRESS;
    } else {
       return PADDLE_2_LCD_CHAR_ADDRESS;
    }

}

// draw a paddle
void drawPaddle(char pos_row, char pos_char, int which) {
    int i;
    int limitY = paddlePos[which] + 4;
    Lcd_Cmd(getPaddleLCDCmd(which));
    for (i = 0; i < 8; i++) {
      if (i >= paddlePos[which] && i < limitY) {
         if (which == 0) {
           Lcd_Chr_CP(3);
         } else {
           Lcd_Chr_CP(24);
         }
      } else {
         Lcd_Chr_CP(0);
      }
    }
    Lcd_Cmd(_LCD_RETURN_HOME);
    Lcd_Chr(pos_row, pos_char, getPaddleLCDCharAddress(which));
}

// move a paddle
void movePaddle(int which) {

     if (paddleDir[which] == up) {
        paddleUp(which);
     }

     if (paddleDir[which] == down) {
        paddleDown(which);
     }

     if (paddleHitBottom(which) != 0){
        paddleDir[which] = up;
     }

     if (paddleHitTop(which) != 0){
        paddleDir[which] = down;
     }
}

/* ********************************** */
/* **** BALL BEHAVIOR **** */
/* ********************************** */

// inverse X axis ball speed
void inverseBallXSpeed() {
     int tmpCheckScoreHelper = checkScoreAttacker;
     checkScoreAttacker = checkScoreDefender;
     checkScoreDefender = tmpCheckScoreHelper;
     ballSpeed[0] *= -1;
}
// inverse Y axis ball speed
void inverseBallYSpeed() {
     ballSpeed[1] *= -1;
}

// draw ball
void drawBall() {
    char i;
    Lcd_Cmd(72);
    
    for (i = 0; i<=7; i++) {
        if (ballInnerPointer == i) {
           Lcd_Chr_CP(ballInnerPos);
        } else {
           Lcd_Chr_CP(0);
        }
    }
    Lcd_Cmd(_LCD_RETURN_HOME);
    Lcd_Chr(1, ballCurrentCell, 1);
}

// check if ball is moving forward
int isBallSpeedPositive(int axis) {
    if (ballSpeed[axis] > 0) {
       return 1;
    }
    return 0;
}

// has the ball hit vertical boundaries of game area?
int ballHitXBoundaries() {
    if ((ballInnerPos == 1 && ballCurrentCell == BALL_CELL_END && isBallSpeedPositive(0) != 0)
    || (ballInnerPos == 16 && ballCurrentCell== BALL_CELL_START && isBallSpeedPositive(0) == 0)) {
        return 1;
    }

    return 0;
}

// has the ball hit horizontal boundaries of game area?
int ballHitYBoundaries () {
   if (ballInnerPointer == 7 || ballInnerPointer == 0) {
      return 1;
   }
   return 0;
}

/* ********************************** */
/* **** GAME ACTIONS **** */
/* ********************************** */

// throw ball in the middle
void throwBall() {
   ballCurrentCell = (BALL_CELL_END + BALL_CELL_START) / 2;
}
// new game
void resetGame() {
   score[0] = 0;
   score[1] = 0;
   throwBall();
   WaitForSec(1);
}

// keep score for each player
void countScore(int which) {
     score[which] = score[which] + 1;
     showGameInfo();
     throwBall();
     // has the game ended?
     if (score[which] == MAX_SCORE ) {
        resetGame();
     }
}

// verifica se houve pontuação
void checkScore() {
   // did not hit the defense, attacker gets 1 point
   if (ballInnerPointer < paddlePos[checkScoreDefender] || ballInnerPointer > (paddlePos[checkScoreDefender] + 4)) {
      countScore(checkScoreAttacker);
      WaitForSec(1);
   // rebound on corners inverse speed, only on corners
   } else if (ballInnerPointer == paddlePos[checkScoreDefender] || ballInnerPointer == (paddlePos[checkScoreDefender] + 4) ) {
     inverseBallYSpeed();
   }
}


// controls a paddle
void playerControl(int which) {
     int adc = ADC_Read(which);
     // if the new position is greater than current one, move up, else move down
     if (adc > controlPaddlePos[which]) {
        paddleDir[which] = up;
     } else {
       paddleDir[which] = down;
     }
     movePaddle(which);
}


// moves the ball around
void moveBall() {
     int movedCell = 0;
     // if ball position = 1, ball has reached end of LCD cell, then it must move to next cell when speed is positive
     if (ballInnerPos == 1 && isBallSpeedPositive(0) != 0) {
        ballInnerPos = 16;
        ballCurrentCell++;
        movedCell = 1;
     }
     
     // if ball position = 16, ball has reached end of LCD cell, then it must move to previous cell when speed is negative
     if (ballInnerPos == 16 && isBallSpeedPositive(0) == 0) {
        ballInnerPos = 1;
        ballCurrentCell--;
        movedCell = 1;
     }

     // only move if cell is the same
     if (movedCell == 0) {
       // if speed X is positive, divide by 2 to move right, else multiply to go left
       if (isBallSpeedPositive(0) != 0) {
          ballInnerPos /= 2;
       } else {
         ballInnerPos *= 2;
       }
     // if cell is not the same, reset the variable
     } else {
       movedCell = 0;
     }

    
     if (isBallSpeedPositive(1) != 0) {
       ballInnerPointer = (ballInnerPointer + 1) % 8;
     } else {
       ballInnerPointer = (ballInnerPointer - 1) % 8;
     }

     // draw ball based on current move
     drawBall();

     // has the ball hit X boundaries? then inverse X speed
     if (ballHitXBoundaries() != 0) {
        checkScore();
        inverseBallXSpeed();
     }

     // has the ball hit Y boundaries? then inverse Y speed
     if (ballHitYBoundaries() != 0) {
        inverseBallYSpeed();
     }
}

void main(){
   OPTION_REG.T0CS = 0; // Timer 0 Clock Source
   OPTION_REG.T0SE = 0; // Timer 0 Source Edge Select - Incrementa no low-to-high
   OPTION_REG.PSA = 0; // Prescaler Assignment - Acionado para o Timer 0
   OPTION_REG.PS0 = 1;  // Prescale 1:256
   OPTION_REG.PS1 = 1;  // Prescale 1:256
   OPTION_REG.PS2 = 1;  // Prescale 1:256
   ADCON1  = 0x8e; // Analog input
   Lcd_Init();                       // start CLD
   Lcd_Cmd(_LCD_CLEAR);               // clear LCD
   Lcd_Cmd(_LCD_CURSOR_OFF);          // turn LCD cursor off

   while(1) {
      // add analog controls
      playerControl(0);
      playerControl(1);
      // clear LCD
      Lcd_Cmd(_LCD_CLEAR);
      // show game score and title
      showGameInfo();
      // start paddles
      drawPaddle(1, 1, 0);
      drawPaddle(1, 6, 1);
      // start ball movement
      moveBall();
      WaitForSec(0.2);
   }
}