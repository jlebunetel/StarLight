/*
   This sketch is the main program of the game.
*/

// librairies
#include "button.h"
#include "led.h"
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include "RTClib.h"


// configuration
#define RED_BUTTON_PIN    10
#define GREEN_BUTTON_PIN  12

#define RED_LED_PIN        2
#define GREEN_LED_PIN      3

#define RED_STRIP_PIN      6
#define GREEN_STRIP_PIN    7

#define RED_STRIP_LENGTH   150
#define GREEN_STRIP_LENGTH 149

#define DEFAULT_RACKET_LENGTH 20
#define MIN_RACKET_LENGTH      5
int racketLength = DEFAULT_RACKET_LENGTH;

#define BLINK_DELAY 500

#define DEFAULT_BALL_SPEED 30
#define MAX_BALL_SPEED 4
int ballSpeed = DEFAULT_BALL_SPEED;

unsigned long stopTop;
#define STOP_DELAY 10000

// variables configuration
Button redButton;
Button greenButton;

Led redLed;
Led greenLed;

Adafruit_NeoPixel redStrip = Adafruit_NeoPixel(RED_STRIP_LENGTH, RED_STRIP_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel greenStrip = Adafruit_NeoPixel(GREEN_STRIP_LENGTH, GREEN_STRIP_PIN, NEO_GRB + NEO_KHZ800);

// HTML colors
#define COLOR_WHITE   0xFFFFFF
#define COLOR_SILVER  0xC0C0C0
#define COLOR_GRAY    0x808080
#define COLOR_BLACK   0x000000
#define COLOR_RED     0xFF0000
#define COLOR_MAROON  0x800000
#define COLOR_YELLOW  0xFFFF00
#define COLOR_OLIVE   0x808000
#define COLOR_LIME    0x00FF00
#define COLOR_GREEN   0x008000
#define COLOR_AQUA    0x00FFFF
#define COLOR_TEAL    0x008080
#define COLOR_BLUE    0x0000FF
#define COLOR_NAVY    0x000080
#define COLOR_FUCHSIA 0xFF00FF
#define COLOR_PURPLE  0x800080


// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
RTC_DS3231 rtc;

#define STANDBY_MODE                0
#define WAIT_FOR_SECOND_PLAYER_MODE 1
#define LAUNCH_MODE                 2
#define PROGRESS_MODE               3
#define STOP_MODE                   4
int mode = STANDBY_MODE;

#define RED_PLAYER   0
#define GREEN_PLAYER 1
int firstPlayer = RED_PLAYER;

#define RED_STRIP 0
#define GREEN_STRIP 1
#define TOWARD_RED 0
#define TOWARD_GREEN 1
int ballStrip;
int ballDir;
int ballPos;
int ballInProgress;

unsigned long top;
int blinkState = 0;

uint8_t currentScoreRedPlayer = 0;
uint8_t currentScoreGreenPlayer = 0;

void setup() {
  // initialize serial communications at 115200 bps:
  Serial.begin(115200);

  // initialize hardware
  redButton.begin(RED_BUTTON_PIN, HIGH);
  greenButton.begin(GREEN_BUTTON_PIN, HIGH);

  redLed.begin(RED_LED_PIN);
  greenLed.begin(GREEN_LED_PIN);

  redStrip.begin();
  redStrip.show(); // Initialize all pixels to 'off'

  greenStrip.begin();
  greenStrip.show(); // Initialize all pixels to 'off'

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  delay(3000);
  DateTime now = rtc.now();
  Serial.print("/YEAR ");
  Serial.println(now.year() - 2000, DEC);
  Serial.print("/MONTH ");
  Serial.println(now.month(), DEC);
  Serial.print("/DAY ");
  Serial.println(now.day(), DEC);
  Serial.print("/HOUR ");
  Serial.println(now.hour(), DEC);
  Serial.print("/MINUTE ");
  Serial.println(now.minute(), DEC);
  Serial.println("/DISPLAY_DATE");
  delay(2000);
  theaterChaseRainbow(50);
  Serial.println("/STANDBY");
  mode = STANDBY_MODE;
}

void loop() {
  // MODE : standby
  top = millis();
  while (mode == STANDBY_MODE) {
    if (millis() > (top + BLINK_DELAY)) {
      if (blinkState) {
        // strips and buttons on
        for (int i = (RED_STRIP_LENGTH - racketLength); i < RED_STRIP_LENGTH; i++) {
          redStrip.setPixelColor(i, COLOR_MAROON);
        }
        for (int i = (GREEN_STRIP_LENGTH - racketLength); i < GREEN_STRIP_LENGTH; i++) {
          greenStrip.setPixelColor(i, COLOR_GREEN);
        }
        redStrip.show();
        greenStrip.show();
        redLed.on();
        greenLed.on();
      }
      else {
        // strips and buttons off
        redStrip.clear();
        greenStrip.clear();
        redStrip.show();
        greenStrip.show();
        redLed.off();
        greenLed.off();
      }
      top = millis();
      blinkState = 1 - blinkState;
    }

    // is a button pressed ?
    redButton.read();
    greenButton.read();
    if (redButton.isJustOn()) {
      mode = WAIT_FOR_SECOND_PLAYER_MODE;
      firstPlayer = RED_PLAYER;
      Serial.println("/WAIT_GREEN");
    }
    if (greenButton.isJustOn()) {
      mode = WAIT_FOR_SECOND_PLAYER_MODE;
      firstPlayer = GREEN_PLAYER;
      Serial.println("/WAIT_RED");
    }

    // wait a little ...
    delay(20);
  }

  if (mode == WAIT_FOR_SECOND_PLAYER_MODE) {

    // MODE : wait for a second player
    // first player button and strip on
    // second player button and strip blink

    if (firstPlayer == RED_PLAYER) {
      for (int i = (RED_STRIP_LENGTH - racketLength); i < RED_STRIP_LENGTH; i++) {
        redStrip.setPixelColor(i, COLOR_MAROON);
      }
      redStrip.show();
      redLed.on();
    }
    else {
      for (int i = (GREEN_STRIP_LENGTH - racketLength); i < GREEN_STRIP_LENGTH; i++) {
        greenStrip.setPixelColor(i, COLOR_GREEN);
      }
      greenStrip.show();
      greenLed.on();
    }

    top = millis();
    stopTop = millis();
    blinkState = 1;
    while (mode == WAIT_FOR_SECOND_PLAYER_MODE) {
      if (millis() > (top + BLINK_DELAY)) {
        if (blinkState) {
          if (firstPlayer == RED_PLAYER) {
            for (int i = (GREEN_STRIP_LENGTH - racketLength); i < GREEN_STRIP_LENGTH; i++) {
              greenStrip.setPixelColor(i, COLOR_GREEN);
            }
            greenStrip.show();
            greenLed.on();
          }
          else {
            for (int i = (RED_STRIP_LENGTH - racketLength); i < RED_STRIP_LENGTH; i++) {
              redStrip.setPixelColor(i, COLOR_MAROON);
            }
            redStrip.show();
            redLed.on();
          }
        }
        else {
          if (firstPlayer == RED_PLAYER) {
            greenStrip.clear();
            greenStrip.show();
            greenLed.off();
          }
          else {
            redStrip.clear();
            redStrip.show();
            redLed.off();
          }
        }
        top = millis();
        blinkState = 1 - blinkState;
      }

      if (millis() > (stopTop + STOP_DELAY)) {
        Serial.println("/STANDBY");
        mode = STANDBY_MODE;
      }

      if (firstPlayer == RED_PLAYER) {
        greenButton.read();
        if (greenButton.isJustOn()) {
          mode = LAUNCH_MODE;
        }
      }
      else {
        redButton.read();
        if (redButton.isJustOn()) {
          mode = LAUNCH_MODE;
        }
      }

      // wait a little ...
      delay(20);
    }

  } // end WAIT_FOR_SECOND_PLAYER_MODE

  if (mode == LAUNCH_MODE) {

    // MODE : LAUNCH

    // every 2 games, racket length decreases
    // update racket length
    int n = (currentScoreRedPlayer + currentScoreGreenPlayer) / 2;
    racketLength = max(DEFAULT_RACKET_LENGTH - n, MIN_RACKET_LENGTH);
    // update speed
    //ballSpeed = max(DEFAULT_BALL_SPEED - (2 * n), MAX_BALL_SPEED);
    ballSpeed = DEFAULT_BALL_SPEED;


    if (firstPlayer == RED_PLAYER) {
      Serial.println("/LAUNCH_RED");
    }
    else {
      Serial.println("/LAUNCH_GREEN");
    }

    // on affiche les raquettes
    // la balle (bleue) clignote au bout du ruban du joueur 1

    // on affiche les raquettes et les boutons
    redStrip.clear();
    greenStrip.clear();
    for (int i = (RED_STRIP_LENGTH - racketLength); i < RED_STRIP_LENGTH; i++) {
      redStrip.setPixelColor(i, COLOR_MAROON);
    }
    for (int i = (GREEN_STRIP_LENGTH - racketLength); i < GREEN_STRIP_LENGTH; i++) {
      greenStrip.setPixelColor(i, COLOR_GREEN);
    }
    redStrip.show();
    greenStrip.show();
    redLed.on();
    greenLed.on();

    // on fait clignoter la led du coté du joueur qui a la main
    blinkState = 1;
    top = millis();
    stopTop = millis();
    while (mode == LAUNCH_MODE) {
      if (millis() > (top + BLINK_DELAY)) {
        if (blinkState) {
          if (firstPlayer == RED_PLAYER) {
            redStrip.setPixelColor(RED_STRIP_LENGTH - 1, COLOR_BLUE);
            redStrip.show();
          }
          else {
            greenStrip.setPixelColor(GREEN_STRIP_LENGTH - 1, COLOR_BLUE);
            greenStrip.show();
          }
        }
        else {
          if (firstPlayer == RED_PLAYER) {
            redStrip.setPixelColor(RED_STRIP_LENGTH - 1, COLOR_MAROON);
            redStrip.show();
          }
          else {
            greenStrip.setPixelColor(GREEN_STRIP_LENGTH - 1, COLOR_GREEN);
            greenStrip.show();
          }
        }
        top = millis();
        blinkState = 1 - blinkState;
      }

      if (millis() > (stopTop + STOP_DELAY)) {
        mode = STOP_MODE;
      }

      if (firstPlayer == RED_PLAYER) {
        redButton.read();
        if (redButton.isJustOn()) {
          mode = PROGRESS_MODE;
        }
      }
      else {
        greenButton.read();
        if (greenButton.isJustOn()) {
          mode = PROGRESS_MODE;
        }
      }

      // wait a little ...
      delay(20);
    }

  } // end LAUNCH_MODE

  if (mode == PROGRESS_MODE) {
    // PROGRESS MODE
    //   la balle avance tant que :
    //       - elle n'a pas atteint l'extremité du ruban
    //       - le joueur 2 n'a pas appuyé sur son bouton

    Serial.println("/PROGRESS");


    if (firstPlayer == RED_PLAYER) {
      // red player starts
      ballStrip = RED_STRIP;
      ballDir = TOWARD_GREEN;
      ballPos = (RED_STRIP_LENGTH - 1);
    }
    else {
      // green player starts
      ballStrip = GREEN_STRIP;
      ballDir = TOWARD_RED;
      ballPos = (GREEN_STRIP_LENGTH - 1);
    }

    // display racket for both players
    redStrip.clear();
    greenStrip.clear();

    for (int i = (RED_STRIP_LENGTH - racketLength); i < RED_STRIP_LENGTH; i++) {
      redStrip.setPixelColor(i, COLOR_MAROON);
    }

    for (int i = (GREEN_STRIP_LENGTH - racketLength); i < GREEN_STRIP_LENGTH; i++) {
      greenStrip.setPixelColor(i, COLOR_GREEN);
    }

    redStrip.show();
    greenStrip.show();

    ballInProgress = 1;

    while (ballInProgress) {
      if (ballStrip == RED_STRIP) {
        // the ball is on the red strip
        if (ballDir == TOWARD_GREEN) {
          // the ball is in the direction of the green button
          if (ballPos >= 0) {
            // clear red strip
            redStrip.clear();
            // display red racket
            for (int i = (RED_STRIP_LENGTH - racketLength); i < RED_STRIP_LENGTH; i++) {
              redStrip.setPixelColor(i, COLOR_MAROON);
            }
            // display the ball
            redStrip.setPixelColor(ballPos, COLOR_BLUE);
            redStrip.show();
            // move the ball
            ballPos -= 1;
          }
          else {
            // the ball go over the green strip
            //delay(ballSpeed * 10); // transition a travers l'écran
            ballStrip = GREEN_STRIP;
            ballPos = 0;
            // clear red strip
            redStrip.clear();
            // display red racket
            for (int i = (RED_STRIP_LENGTH - racketLength); i < RED_STRIP_LENGTH; i++) {
              redStrip.setPixelColor(i, COLOR_MAROON);
            }
            redStrip.show();
          }
        }
        else {
          // the ball is in the direction of the red button
          if (ballPos < RED_STRIP_LENGTH) {
            // clear red strip
            redStrip.clear();
            // display red racket
            for (int i = (RED_STRIP_LENGTH - racketLength); i < RED_STRIP_LENGTH; i++) {
              redStrip.setPixelColor(i, COLOR_MAROON);
            }
            // display the ball
            redStrip.setPixelColor(ballPos, COLOR_BLUE);
            redStrip.show();
            // move the ball
            ballPos += 1;
          }
          else {
            // the ball reached the end of the red strip
            // the red player lose
            ballInProgress = 0;
            currentScoreGreenPlayer++;
            Serial.print("/SCORE_GREEN ");
            Serial.println(currentScoreGreenPlayer, DEC);
            Serial.println("/WINNER_GREEN");
            theaterChaseGreen(50);
            //delay(5000);
            firstPlayer = RED_PLAYER;
            mode = LAUNCH_MODE;
          }
        }
      }
      else {
        // the ball is on the green strip
        if (ballDir == TOWARD_GREEN) {
          // the ball is in the direction of the green button
          if (ballPos < GREEN_STRIP_LENGTH) {
            // clear green strip
            greenStrip.clear();
            // display green racket
            for (int i = (GREEN_STRIP_LENGTH - racketLength); i < GREEN_STRIP_LENGTH; i++) {
              greenStrip.setPixelColor(i, COLOR_GREEN);
            }
            // display the ball
            greenStrip.setPixelColor(ballPos, COLOR_BLUE);
            greenStrip.show();
            // move the ball
            ballPos += 1;
          }
          else {
            // the ball reached the end of the green strip
            // the green player lose
            ballInProgress = 0;
            currentScoreRedPlayer++;
            Serial.print("/SCORE_RED ");
            Serial.println(currentScoreRedPlayer, DEC);
            Serial.println("/WINNER_RED");
            theaterChaseRed(50);
            //delay(5000);
            firstPlayer = GREEN_PLAYER;
            mode = LAUNCH_MODE;
          }
        }
        else {
          // the ball is in the direction of the red button
          if (ballPos >= 0) {
            // clear green strip
            greenStrip.clear();
            // display green racket
            for (int i = (GREEN_STRIP_LENGTH - racketLength); i < GREEN_STRIP_LENGTH; i++) {
              greenStrip.setPixelColor(i, COLOR_GREEN);
            }
            // display the ball
            greenStrip.setPixelColor(ballPos, COLOR_BLUE);
            greenStrip.show();
            // move the ball
            ballPos -= 1;
          }
          else {
            // the ball go over the red strip
            //delay(ballSpeed * 10); // transition a travers l'écran
            ballStrip = RED_STRIP;
            ballPos = 0;
            // clear green strip
            greenStrip.clear();
            // display green racket
            for (int i = (GREEN_STRIP_LENGTH - racketLength); i < GREEN_STRIP_LENGTH; i++) {
              greenStrip.setPixelColor(i, COLOR_GREEN);
            }
            greenStrip.show();
          }
        }
      }


      // is a button pressed ?
      redButton.read();
      greenButton.read();
      if (redButton.isJustOn() and ballDir == TOWARD_RED) {
        if (ballStrip == RED_STRIP and (ballPos >= (RED_STRIP_LENGTH - racketLength))) {
          // the red player send back the ball
          ballSpeed = max(ballSpeed - 2, MAX_BALL_SPEED); // speed up!
          ballDir = TOWARD_GREEN;
        }
        else {
          // the red player lose!
          ballInProgress = 0;
          currentScoreGreenPlayer++;
          Serial.print("/SCORE_GREEN ");
          Serial.println(currentScoreGreenPlayer, DEC);
          Serial.println("/WINNER_GREEN");
          theaterChaseGreen(50);
          //delay(5000);
          firstPlayer = RED_PLAYER;
          mode = LAUNCH_MODE;
        }
      }
      if (greenButton.isJustOn() and ballDir == TOWARD_GREEN) {
        if (ballStrip == GREEN_STRIP and (ballPos >= (GREEN_STRIP_LENGTH - racketLength))) {
          // the green player send back the ball
          ballSpeed = max(ballSpeed - 2, MAX_BALL_SPEED); // speed up!
          ballDir = TOWARD_RED;
        }
        else {
          // the green player lose!
          ballInProgress = 0;
          currentScoreRedPlayer++;
          Serial.print("/SCORE_RED ");
          Serial.println(currentScoreRedPlayer, DEC);
          Serial.println("/WINNER_RED");
          theaterChaseRed(50);
          //delay(5000);
          firstPlayer = GREEN_PLAYER;
          mode = LAUNCH_MODE;
        }
      }
      delay(ballSpeed);
    }
  } // end PROGRESS_MODE

  if (mode == STOP_MODE) {
    // qui a gagné ?
    if (currentScoreRedPlayer > currentScoreGreenPlayer) {
      Serial.println("/STOP_RED");
      theaterChaseRed(50);
    }
    else if (currentScoreRedPlayer < currentScoreGreenPlayer) {
      Serial.println("/STOP_GREEN");
      theaterChaseGreen(50);
    }
    else {
      Serial.println("/STOP_DRAW");
      theaterChaseRainbow(50);
    }
    //delay(5000);

    // nouvelle partie
    mode = STANDBY_MODE;
    Serial.println("/STANDBY");
    currentScoreRedPlayer = 0;
    currentScoreGreenPlayer = 0;
    Serial.println("/SCORE_RED 0");
    Serial.println("/SCORE_GREEN 0");
    racketLength = DEFAULT_RACKET_LENGTH;
    ballSpeed = DEFAULT_BALL_SPEED;
  } // end STOP_MODE

}

//Theatre-style crawling lights.
void theaterChaseRed(uint8_t wait) {
  for (int j = 0; j < 25; j++) { //do 25 cycles of chasing
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < RED_STRIP_LENGTH; i = i + 3) {
        redStrip.setPixelColor(i + q, COLOR_MAROON);  //turn every third pixel on
      }
      redStrip.show();

      delay(wait);

      for (uint16_t i = 0; i < RED_STRIP_LENGTH; i = i + 3) {
        redStrip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

void theaterChaseGreen(uint8_t wait) {
  for (int j = 0; j < 25; j++) { //do 25 cycles of chasing
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < GREEN_STRIP_LENGTH; i = i + 3) {
        greenStrip.setPixelColor(i + q, COLOR_GREEN);  //turn every third pixel on
      }
      greenStrip.show();

      delay(wait);

      for (uint16_t i = 0; i < GREEN_STRIP_LENGTH; i = i + 3) {
        greenStrip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j = 0; j < 256; j = j + 8) { // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < RED_STRIP_LENGTH; i = i + 3) {
        redStrip.setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on
      }
      for (uint16_t i = 0; i < GREEN_STRIP_LENGTH; i = i + 3) {
        greenStrip.setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on
      }
      redStrip.show();
      greenStrip.show();

      delay(wait);

      for (uint16_t i = 0; i < RED_STRIP_LENGTH; i = i + 3) {
        redStrip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
      for (uint16_t i = 0; i < GREEN_STRIP_LENGTH; i = i + 3) {
        greenStrip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return redStrip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return redStrip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return redStrip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
//DateTime now = rtc.now();
//Serial.println(now.unixtime());
//delay(3000);
