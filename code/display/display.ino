
#include <EEPROM.h>
uint8_t lastScoreRedPlayer = 12;
uint8_t lastScoreGreenPlayer = 15;
uint8_t lastScoreYear = 0;
uint8_t lastScoreMonth = 0;
uint8_t lastScoreDay = 0;
uint8_t lastScoreHour = 0;
uint8_t lastScoreMinute = 0;

uint8_t bestScoreRedPlayer = 30;
uint8_t bestScoreGreenPlayer = 19;
uint8_t bestScoreYear = 0;
uint8_t bestScoreMonth = 0;
uint8_t bestScoreDay = 0;
uint8_t bestScoreHour = 0;
uint8_t bestScoreMinute = 0;

uint8_t currentScoreRedPlayer = 0;
uint8_t currentScoreGreenPlayer = 0;
uint8_t currentYear = 0;
uint8_t currentMonth = 1;
uint8_t currentDay = 1;
uint8_t currentHour = 0;
uint8_t currentMinute = 0;

unsigned long top;
int blinkOn = 0;
#define BLINK_DELAY     500
#define SCREEN_DELAY   5000
#define WAIT_DELAY    10000
int standbyChange = 0;

// progress bar
unsigned long progressTop;
int progressBar = 0;
#define PROGRESS_LENGHT  63
// 10000 / 64 = 156
#define PROGRESS_DELAY  156

#include <SmartLEDShieldV4.h>  // comment out this line for if you're not using SmartLED Shield V4 hardware (this line needs to be before #include <SmartMatrix3.h>)
#include <SmartMatrix3.h>

// uncomment if you want to debug
//#define DEBUG

#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint8_t kMatrixWidth = 64;        // known working: 32, 64, 96, 128
const uint8_t kMatrixHeight = 32;       // known working: 16, 32, 48, 64
const uint8_t kRefreshDepth = 36;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN; // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels, or use SMARTMATRIX_HUB75_64ROW_MOD32SCAN for common 64x64 panels
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);

#define FULL_BRIGHTNESS 255
//const rgb24 defaultBackgroundColor = {0, 0, 0};

// HTML colors
#define COLOR_WHITE   {0xff, 0xff, 0xff}
#define COLOR_SILVER  {0xc0, 0xc0, 0xc0}
#define COLOR_GRAY    {0x80, 0x80, 0x80}
#define COLOR_BLACK   {0, 0, 0}
#define COLOR_RED     {0xff, 0, 0}
#define COLOR_MAROON  {0x80, 0, 0}
#define COLOR_YELLOW  {0xff, 0xff, 0}
#define COLOR_OLIVE   {0x80, 0x80, 0}
#define COLOR_LIME    {0, 0xff, 0}
#define COLOR_GREEN   {0, 0x80, 0}
#define COLOR_AQUA    {0, 0xff, 0xff}
#define COLOR_TEAL    {0, 0x80, 0x80}
#define COLOR_BLUE    {0, 0, 0xff}
#define COLOR_NAVY    {0, 0, 0x80}
#define COLOR_FUCHSIA {0xff, 0, 0xff}
#define COLOR_PURPLE  {0x80, 0, 0x80}


// variables for serial reading
char receivedChar = '0';
String receivedString = "";
String message = "";

// commands
#define STANDBY       1
#define WAIT_RED      2
#define WAIT_GREEN    3
#define LAUNCH_RED    4
#define LAUNCH_GREEN  5
#define PROGRESS      6
#define WINNER_RED    7
#define WINNER_GREEN  8
#define STOP_RED      9
#define STOP_GREEN   10
#define SCORE_RED    11
#define SCORE_GREEN  12
#define BOOTING      13
#define YEAR         14
#define MONTH        15
#define DAY          16
#define HOUR         17
#define MINUTE       18
#define DISPLAY_DATE 19
#define STOP_DRAW    20
bool command_flag = false;
int command = 0;
bool value_flag = false;
int mode = 0;


void setup() {
  // init Serial communication
#ifdef DEBUG
  Serial.begin(115200);
#else
  Serial2.begin(115200);
#endif
  delay(200);

  matrix.addLayer(&backgroundLayer);
  matrix.begin();
  matrix.setBrightness(FULL_BRIGHTNESS);
  backgroundLayer.enableColorCorrection(true);

  // reads values from EEPROM
  /*
    lastScoreRedPlayer   = EEPROM.read(0);
    lastScoreGreenPlayer = EEPROM.read(1);
    lastScoreYear        = EEPROM.read(2);
    lastScoreMonth       = EEPROM.read(3);
    lastScoreDay         = EEPROM.read(4);
    lastScoreHour        = EEPROM.read(5);
    lastScoreMinute      = EEPROM.read(6);

    bestScoreRedPlayer   = EEPROM.read(7);
    bestScoreGreenPlayer = EEPROM.read(8);
    bestScoreYear        = EEPROM.read(9);
    bestScoreMonth       = EEPROM.read(10);
    bestScoreDay         = EEPROM.read(11);
    bestScoreHour        = EEPROM.read(12);
    bestScoreMinute      = EEPROM.read(13);
  */

  // wait for Arduino Mega to start ...
  mode = BOOTING;

}


void loop() {
  // Listening to serial
#ifdef DEBUG
  while (Serial.available() > 0) {
    receivedChar = Serial.read();
#else
  while (Serial2.available() > 0) {
    receivedChar = Serial2.read();
#endif
    if (receivedChar == '/') {
      // Recieving command
      command_flag = true;
    }
    else if (receivedChar == '\n' || receivedChar == ' ' || receivedChar == 13) {
      // Message processing

      if (command_flag) {
        // Command processing

        if (receivedString == "SCORE_RED") {
          command = SCORE_RED;
          value_flag = true; // Waiting for a following argument
        }

        if (receivedString == "SCORE_GREEN") {
          command = SCORE_GREEN;
          value_flag = true; // Waiting for a following argument
        }

        if (receivedString == "YEAR") {
          command = YEAR;
          value_flag = true; // Waiting for a following argument
        }

        if (receivedString == "MONTH") {
          command = MONTH;
          value_flag = true; // Waiting for a following argument
        }

        if (receivedString == "DAY") {
          command = DAY;
          value_flag = true; // Waiting for a following argument
        }

        if (receivedString == "HOUR") {
          command = HOUR;
          value_flag = true; // Waiting for a following argument
        }

        if (receivedString == "MINUTE") {
          command = MINUTE;
          value_flag = true; // Waiting for a following argument
        }

        if (receivedString == "DISPLAY_DATE") {
          // display date
          mode = DISPLAY_DATE;
          top = 0; // refresh screen !
          progressBar = 0; // hide progress bar
        }

        if (receivedString == "STANDBY") {
          // standby mode
          mode = STANDBY;
          top = 0; // refresh screen !
          progressBar = 0; // hide progress bar
          standbyChange = 0;
        }

        if (receivedString == "WAIT_RED") {
          // wait for red player
          mode = WAIT_RED;
          top = 0; // refresh screen !
          progressBar = PROGRESS_LENGHT; // show progress bar
          progressTop = millis();
        }

        if (receivedString == "WAIT_GREEN") {
          // wait for green player
          mode = WAIT_GREEN;
          top = 0; // refresh screen !
          progressBar = PROGRESS_LENGHT; // show progress bar
          progressTop = millis();
        }

        if (receivedString == "LAUNCH_RED") {
          // wait for red player to launch the ball
          mode = LAUNCH_RED;
          top = 0; // refresh screen !
          progressBar = PROGRESS_LENGHT; // show progress bar
          progressTop = millis();
        }

        if (receivedString == "LAUNCH_GREEN") {
          // wait for green player to launch the ball
          mode = LAUNCH_GREEN;
          top = 0; // refresh screen !
          progressBar = PROGRESS_LENGHT; // show progress bar
          progressTop = millis();
        }

        if (receivedString == "PROGRESS") {
          // game in progress (display scores)
          mode = PROGRESS;
          top = 0; // refresh screen !
          progressBar = 0; // hide progress bar
        }

        if (receivedString == "WINNER_RED") {
          // red player wins
          mode = WINNER_RED;
          top = 0; // refresh screen !
          progressBar = 0; // hide progress bar
        }

        if (receivedString == "WINNER_GREEN") {
          // green player wins
          mode = WINNER_GREEN;
          top = 0; // refresh screen !
          progressBar = 0; // hide progress bar
        }

        if (receivedString == "STOP_RED") {
          // red player wins the whole game
          mode = STOP_RED;
          top = 0; // refresh screen !
          progressBar = 0; // hide progress bar
        }

        if (receivedString == "STOP_GREEN") {
          // green player wins the whole game
          mode = STOP_GREEN;
          top = 0; // refresh screen !
          progressBar = 0; // hide progress bar
        }

        if (receivedString == "STOP_DRAW") {
          // no one wins
          mode = STOP_DRAW;
          top = 0; // refresh screen !
          progressBar = 0; // hide progress bar
        }

        command_flag = false; // Command processed!
      }
      else if (value_flag) {
        // Argument processing
        switch (command) {

          case SCORE_RED:
            // update score for the red player
            currentScoreRedPlayer = receivedString.toInt();
            break;

          case SCORE_GREEN:
            // update score for the green player
            currentScoreGreenPlayer = receivedString.toInt();
            break;

          case YEAR:
            // update current year
            currentYear = receivedString.toInt();
            break;

          case MONTH:
            // update current month
            currentMonth = receivedString.toInt();
            break;

          case DAY:
            // update current day
            currentDay = receivedString.toInt();
            break;

          case HOUR:
            // update current hour
            currentHour = receivedString.toInt();
            break;

          case MINUTE:
            // update current minute
            currentMinute = receivedString.toInt();
            break;
        }
        value_flag = false; // Argument processed!
      }
      else {
        // ?


      }
      // Empty the message buffer
      receivedString = "";
    }
    else {
      // Addition of a character to the message buffer
      receivedString += receivedChar;
    }
    delay(1); // A little delay for the microcontroler stability
  }

  if (millis() > progressTop + PROGRESS_DELAY) {
    progressTop = millis();

    if (progressBar > 0) {
      backgroundLayer.drawLine(0, 0, progressBar, 0, COLOR_PURPLE);
      backgroundLayer.drawLine(progressBar, 0, 63, 0, COLOR_BLACK);
      backgroundLayer.swapBuffers();
      progressBar = progressBar - 1;
    }
  }


  if (millis() > top + BLINK_DELAY) {
    top = millis();
    blinkOn = 1 - blinkOn;

    backgroundLayer.fillScreen(COLOR_BLACK);
    backgroundLayer.setFont(font5x7);
    if (progressBar > 0) {
      backgroundLayer.drawLine(0, 0, progressBar, 0, COLOR_PURPLE);
      backgroundLayer.drawLine(progressBar, 0, 63, 0, COLOR_BLACK);
    }


    switch (mode) {

      case BOOTING:
        backgroundLayer.drawString(10, 6, COLOR_NAVY, "StarLight");
        backgroundLayer.drawString(10, 5, COLOR_BLUE, "StarLight");

        backgroundLayer.drawString(10, 18, COLOR_TEAL, "Demarrage");
        backgroundLayer.drawString(10, 17, COLOR_AQUA, "Demarrage");

        backgroundLayer.drawString(8, 26, COLOR_TEAL, "en cours ...");
        backgroundLayer.drawString(8, 25, COLOR_AQUA, "en cours ...");
        break;

      case DISPLAY_DATE:
        displayDate();
        break;

      case STANDBY:
        if (standbyChange < 8) {
          backgroundLayer.drawString(10, 6, COLOR_NAVY, "StarLight");
          backgroundLayer.drawString(10, 5, COLOR_BLUE, "StarLight");

          if (blinkOn) {
            backgroundLayer.drawString(5, 18, COLOR_TEAL, "Appuyez sur");
            backgroundLayer.drawString(5, 17, COLOR_AQUA, "Appuyez sur");

            backgroundLayer.drawString(10, 26, COLOR_TEAL, "un bouton");
            backgroundLayer.drawString(10, 25, COLOR_AQUA, "un bouton");
          }
        }
        else if (standbyChange < 14) {
          backgroundLayer.drawString(12, 2, COLOR_NAVY, "Derniere");
          backgroundLayer.drawString(12, 1, COLOR_BLUE, "Derniere");

          backgroundLayer.drawString(12, 10, COLOR_NAVY, "partie :");
          backgroundLayer.drawString(12, 9, COLOR_BLUE, "partie :");

          displayLastScore();
        }
        else {
          backgroundLayer.drawString(12, 2, COLOR_NAVY, "Meilleur");
          backgroundLayer.drawString(12, 1, COLOR_BLUE, "Meilleur");

          backgroundLayer.drawString(15, 10, COLOR_NAVY, "score :");
          backgroundLayer.drawString(15, 9, COLOR_BLUE, "score :");

          displayBestScore();
        }
        standbyChange++;
        if (standbyChange == 20) {
          standbyChange = 0;
        }
        break;

      case WAIT_RED:
        backgroundLayer.drawString(12, 2, COLOR_NAVY, "Nouvelle");
        backgroundLayer.drawString(12, 1, COLOR_BLUE, "Nouvelle");

        backgroundLayer.drawString(17, 10, COLOR_NAVY, "partie");
        backgroundLayer.drawString(17, 9, COLOR_BLUE, "partie");

        if (blinkOn) {
          backgroundLayer.drawString(15, 18, COLOR_MAROON, "Attente");
          backgroundLayer.drawString(15, 17, COLOR_RED, "Attente");

          backgroundLayer.drawString(2, 26, COLOR_MAROON, "joueur rouge");
          backgroundLayer.drawString(2, 25, COLOR_RED, "joueur rouge");
        }
        break;

      case WAIT_GREEN:
        backgroundLayer.drawString(12, 2, COLOR_NAVY, "Nouvelle");
        backgroundLayer.drawString(12, 1, COLOR_BLUE, "Nouvelle");

        backgroundLayer.drawString(17, 10, COLOR_NAVY, "partie");
        backgroundLayer.drawString(17, 9, COLOR_BLUE, "partie");

        if (blinkOn) {
          backgroundLayer.drawString(15, 18, COLOR_OLIVE, "Attente");
          backgroundLayer.drawString(15, 17, COLOR_LIME, "Attente");

          backgroundLayer.drawString(5, 26, COLOR_OLIVE, "joueur vert");
          backgroundLayer.drawString(5, 25, COLOR_LIME, "joueur vert");
        }
        break;

      case LAUNCH_RED:
        if (blinkOn) {
          backgroundLayer.drawString(17, 2, COLOR_MAROON, "Lancez");
          backgroundLayer.drawString(17, 1, COLOR_RED, "Lancez");

          backgroundLayer.drawString(12, 10, COLOR_MAROON, "la balle");
          backgroundLayer.drawString(12, 9, COLOR_RED, "la balle");
        }

        displayScore();
        break;

      case LAUNCH_GREEN:
        if (blinkOn) {
          backgroundLayer.drawString(17, 2, COLOR_OLIVE, "Lancez");
          backgroundLayer.drawString(17, 1, COLOR_LIME, "Lancez");

          backgroundLayer.drawString(12, 10, COLOR_OLIVE, "la balle");
          backgroundLayer.drawString(12, 9, COLOR_LIME, "la balle");
        }

        displayScore();
        break;

      case PROGRESS:
        backgroundLayer.drawString(15, 6, COLOR_NAVY, "Score :");
        backgroundLayer.drawString(15, 5, COLOR_BLUE, "Score :");
        displayScore();
        break;

      case WINNER_RED:
        if (blinkOn) {
          backgroundLayer.drawString(2, 2, COLOR_MAROON, "Joueur rouge");
          backgroundLayer.drawString(2, 1, COLOR_RED, "Joueur rouge");

          backgroundLayer.drawString(12, 10, COLOR_MAROON, "marque !");
          backgroundLayer.drawString(12, 9, COLOR_RED, "marque !");
        }

        displayScore();
        break;

      case WINNER_GREEN:
        if (blinkOn) {
          backgroundLayer.drawString(5, 2, COLOR_OLIVE, "Joueur vert");
          backgroundLayer.drawString(5, 1, COLOR_LIME, "Joueur vert");

          backgroundLayer.drawString(12, 10, COLOR_OLIVE, "marque !");
          backgroundLayer.drawString(12, 9, COLOR_LIME, "marque !");
        }

        displayScore();
        break;

      case STOP_RED:
        if (blinkOn) {
          backgroundLayer.drawString(2, 2, COLOR_MAROON, "Joueur rouge");
          backgroundLayer.drawString(2, 1, COLOR_RED, "Joueur rouge");

          backgroundLayer.drawString(10, 10, COLOR_MAROON, "gagne !!!");
          backgroundLayer.drawString(10, 9, COLOR_RED, "gagne !!!");
        }

        displayScore();
        break;

      case STOP_GREEN:
        if (blinkOn) {
          backgroundLayer.drawString(5, 2, COLOR_OLIVE, "Joueur vert");
          backgroundLayer.drawString(5, 1, COLOR_LIME, "Joueur vert");

          backgroundLayer.drawString(10, 10, COLOR_OLIVE, "gagne !!!");
          backgroundLayer.drawString(10, 9, COLOR_LIME, "gagne !!!");
        }

        displayScore();
        break;

      case STOP_DRAW:
        if (blinkOn) {
          backgroundLayer.drawString(5, 6, COLOR_NAVY, "Match nul !");
          backgroundLayer.drawString(5, 5, COLOR_BLUE, "Match nul !");
        }

        displayScore();
        break;

    }

    backgroundLayer.swapBuffers();

  }

  delay(1); // A little delay for the microcontroler stability
}

void displayDate() {
  char texte[3] = "00";
  texte[0] = (currentHour / 10) + '0';
  texte[1] = (currentHour % 10) + '0';
  backgroundLayer.drawString(20, 6, COLOR_NAVY, texte);
  backgroundLayer.drawString(20, 5, COLOR_BLUE, texte);

  if (blinkOn) {
    backgroundLayer.drawString(30, 6, COLOR_NAVY, ":");
    backgroundLayer.drawString(30, 5, COLOR_BLUE, ":");
  }

  texte[0] = (currentMinute / 10) + '0';
  texte[1] = (currentMinute % 10) + '0';
  backgroundLayer.drawString(35, 6, COLOR_NAVY, texte);
  backgroundLayer.drawString(35, 5, COLOR_BLUE, texte);

  texte[0] = (currentDay / 10) + '0';
  texte[1] = (currentDay % 10) + '0';
  backgroundLayer.drawString(7, 22, COLOR_NAVY, texte);
  backgroundLayer.drawString(7, 21, COLOR_BLUE, texte);

  backgroundLayer.drawString(17, 22, COLOR_NAVY, "-");
  backgroundLayer.drawString(17, 21, COLOR_BLUE, "-");

  texte[0] = (currentMonth / 10) + '0';
  texte[1] = (currentMonth % 10) + '0';
  backgroundLayer.drawString(22, 22, COLOR_NAVY, texte);
  backgroundLayer.drawString(22, 21, COLOR_BLUE, texte);

  backgroundLayer.drawString(32, 22, COLOR_NAVY, "-20");
  backgroundLayer.drawString(32, 21, COLOR_BLUE, "-20");

  texte[0] = (currentYear / 10) + '0';
  texte[1] = (currentYear % 10) + '0';
  backgroundLayer.drawString(47, 22, COLOR_NAVY, texte);
  backgroundLayer.drawString(47, 21, COLOR_BLUE, texte);
}

void displayScore() {
  // green score
  char score[4] = "000";
  score[0] = (currentScoreGreenPlayer / 100) + '0';
  score[1] = (currentScoreGreenPlayer / 10) % 10 + '0';
  score[2] = (currentScoreGreenPlayer % 10) + '0';
  backgroundLayer.drawString(10, 22, COLOR_OLIVE, score);
  backgroundLayer.drawString(10, 21, COLOR_LIME, score);

  // tiret
  backgroundLayer.drawString(30, 22, COLOR_NAVY, "-");
  backgroundLayer.drawString(30, 21, COLOR_BLUE, "-");

  // red score
  score[0] = (currentScoreRedPlayer / 100) + '0';
  score[1] = (currentScoreRedPlayer / 10) % 10 + '0';
  score[2] = (currentScoreRedPlayer % 10) + '0';
  backgroundLayer.drawString(40, 22, COLOR_MAROON, score);
  backgroundLayer.drawString(40, 21, COLOR_RED, score);
}

void displayBestScore() {
  // green score
  char score[4] = "000";
  score[0] = (bestScoreGreenPlayer / 100) + '0';
  score[1] = (bestScoreGreenPlayer / 10) % 10 + '0';
  score[2] = (bestScoreGreenPlayer % 10) + '0';
  backgroundLayer.drawString(10, 22, COLOR_OLIVE, score);
  backgroundLayer.drawString(10, 21, COLOR_LIME, score);

  // tiret
  backgroundLayer.drawString(30, 22, COLOR_NAVY, "-");
  backgroundLayer.drawString(30, 21, COLOR_BLUE, "-");

  // red score
  score[0] = (bestScoreRedPlayer / 100) + '0';
  score[1] = (bestScoreRedPlayer / 10) % 10 + '0';
  score[2] = (bestScoreRedPlayer % 10) + '0';
  backgroundLayer.drawString(40, 22, COLOR_MAROON, score);
  backgroundLayer.drawString(40, 21, COLOR_RED, score);
}

void displayLastScore() {
  // green score
  char score[4] = "000";
  score[0] = (lastScoreGreenPlayer / 100) + '0';
  score[1] = (lastScoreGreenPlayer / 10) % 10 + '0';
  score[2] = (lastScoreGreenPlayer % 10) + '0';
  backgroundLayer.drawString(10, 22, COLOR_OLIVE, score);
  backgroundLayer.drawString(10, 21, COLOR_LIME, score);

  // tiret
  backgroundLayer.drawString(30, 22, COLOR_NAVY, "-");
  backgroundLayer.drawString(30, 21, COLOR_BLUE, "-");

  // red score
  score[0] = (lastScoreRedPlayer / 100) + '0';
  score[1] = (lastScoreRedPlayer / 10) % 10 + '0';
  score[2] = (lastScoreRedPlayer % 10) + '0';
  backgroundLayer.drawString(40, 22, COLOR_MAROON, score);
  backgroundLayer.drawString(40, 21, COLOR_RED, score);
}
