/*
  Matrix Game for Introduction to Robotics

  Description:
  Casoleta Gaming is a portable "console" where you can test your gaming skills by playing a Bomberman-like game.
  Your goal is to destroy all the walls on the map smartly so you can achieve the highest score.
  You can choose from multiple difficulties and use special bombs to secure the top rank.

  Components:
    • one joystick
    • one 8x8 led matrix
    • one MAX7219
    • one button
    • one buzzer  
    • resistors and wires as needed

  Created 18-12-2023
  Coriciuc Tudor 

  https://youtu.be/Q_xfbR4qFis
*/


#include <LedControl.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

// Pin declaration
const byte mainButton = A4;
const byte leftButton = A3;
const byte rightButton = A2;
const byte pinY = A1;
const byte pinX = A0;
const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 10;
const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 3;
const byte d7 = 4;
const byte A = 5;
const byte buzzerPin = 2;

/// Lcd and LedMatrix initialization
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);

/// Matrix Brightness and eeprom location
byte matrixBrightness;
byte matrixBrightnessByte = 0;
const byte maxMatrixBrightness = 15;

/// LCD Brightness and eeprom location
byte lcdBrightness;
const byte lcdBrightnessByte = 1;
const byte lcdMultiplier = 10;
const byte maxLCDBrightness = 26;

/// Main menu
const byte numberOfOptions = 5;
const String meniuOptions[numberOfOptions] = { "Start the game!", "Highscores!", "Settings!", "About!", "How to play!" };

/// Rules for the game
const byte numberOfMessages = 14;
const String howToPlay[numberOfMessages] = { "Your goal is to", "destroy all the",
                                             "walls on the map", "without dying", "using special", "bombs. You can", "move using the", "joystick, place",
                                             "bombs with the", "main button, and", "switch between", "the bombs using", "the right and", "left buttons." };

/// Info section
const byte numberOfInfo = 3;
const String info[numberOfInfo] = { "Casoleta Gaming!", "By TudorCoriciuc", "github: Bucovina" };

/// Settings Submeniu
const byte numberOfSettings = 6;
const String settingsOptions[numberOfSettings] = { "LCD Light", "Matrix Light", "Sound", "Difficulty", "Reset Scores", "Exit" };

/// Sound variables and eeprom
bool soundState = true;
const byte soundStateByte = 3;
const String OnAndOff[2] = { "Off", "On" };

/// numberOfDifficulty variables and eeprom
const byte numberOfDifficulties = 4;
const String difficulties[numberOfDifficulties] = { "Easy", "Medium", "Hard", "Test" };
byte currentDifficulty;
const byte currentDifficultyByte = 4;
float mapCovrageDifficulty[numberOfDifficulties] = { 0.5, 0.6, 0.75, 0.1 };

/// Welcome message
bool introOnce = false;


/// Meniu variables
const int delayBetweenMessages = 1500;
unsigned long lastMessageShown = 0;
byte message = 0;
byte currentOption = 0;
byte optionMainMenu = 0;
byte currentOptionSubMenu = 0;
byte optionSubMenu = 0;
bool shown = false;

/// Buttons variables
const byte buttonPressDebounce = 150;

/// Right button variables
bool stateRightButton = LOW;
bool readingRightButton;
bool lastReadingRightButton = HIGH;
bool doRightButtonAction = false;
unsigned long lastDebounceRightButton = 0;

/// Left button variables
bool stateLeftButton = LOW;
bool readingLeftButton;
bool lastReadingLeftButton = HIGH;
bool doLeftButtonAction = false;
unsigned long lastDebounceLeftButton = 0;

/// Main button variables
bool stateMainButton = LOW;
bool readingMainButton;
bool lastReadingMainButton = HIGH;
bool doMainButtonAction = false;
unsigned long lastDebounceMainButton = 0;

/// HighScore Variables
const byte numberOfHighscores = 3;
const byte nameSize = 3;
byte playerNameBytes[numberOfHighscores][nameSize] = { { 5, 6, 7 }, { 8, 9, 10 }, { 11, 12, 13 } };
char playerName[numberOfHighscores][nameSize + 1] = { "ABC", "DEF", "GHI" };
byte currentCharacterNumber = 0;
char currentCharacter = 'A';
char currentPlayer[nameSize + 1] = { "AAA" };
byte playerScoreByte[numberOfHighscores] = { 14, 16, 18 };
int playerScore[numberOfHighscores] = { 500, 2, 1 };
const byte yesOrNo = 2;
bool reset = false;

/// Custom LCD characters
byte arrowUP[8] = {
  0b00100,
  0b01110,
  0b10101,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100
};

byte arrowDown[8] = {
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b10101,
  0b01110,
  0b00100
};

byte dirtBombIcon[8] = {
  B00000,
  B00011,
  B00100,
  B01110,
  B11111,
  B11111,
  B11111,
  B01110
};

byte normalBombIcon[8] = {
  B00000,
  B00011,
  B00100,
  B01110,
  B10001,
  B10001,
  B10001,
  B01110
};

byte rightDown[8] = {
  B00000,
  B00010,
  B01111,
  B01010,
  B01000,
  B11100,
  B01000,
  B00000
};

byte leftDown[8] = {
  B00000,
  B01000,
  B11110,
  B01010,
  B00010,
  B00111,
  B00010,
  B00000
};

byte rightUp[8] = {
  B00000,
  B01000,
  B11100,
  B01000,
  B01010,
  B01111,
  B00010,
  B00000
};

byte leftUp[8] = {
  B00000,
  B00010,
  B00111,
  B00010,
  B01010,
  B11110,
  B01000,
  B00000
};

/// Custom matrix drawings

const uint64_t drawings[] = {
  0x040c1c3c3c1c0c04,
  0x7e3c183cfebdbd7e,
  0x1818181818187e7c,
  0xff7e3c00183c3c18,
  0x1800181830667e3c
};

const uint64_t intro = {
  0x3c7e7e7e7e3c0810,
};

/// Matrix and variables

const byte matrixGameSize = 16;
const byte matrixRealSize = 8;
byte matrix[matrixGameSize][matrixGameSize] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

// Joystick directions
int xValue = 0;
int yValue = 0;

// We use variables(joyMoved and min/maxThreshold) to make sure we get a correct input from the joystick
bool joyMoved = false;
const int minInputThreshold = 200;
const int maxInputThreshold = 800;
const int minIdleThreshold = 500;
const int maxIdleThreshold = 550;

// Player position
byte playerX;
byte playerY;

// Player alive status
bool playerAlive = true;

// Bomb variables
bool activeBomb = false;
byte bombX;
byte bombY;

// Blink variables
const int playerBlinkDelay = 500;
const int bombBlinkDelay = 200;
unsigned long lastBlinkPlayer = 0;
unsigned long lastBlinkBomb = 0;
bool playerState = 1;
bool bombState = 1;

// The bombs explodes 2 seconds after it was placed down
const int explosionDelay = 2000;
unsigned long bombCounter = 0;

// Spawn
bool validSpawn = false;
const byte minSafeDistance = 2;

// Buzzer variables
const int buzzerTone = 1000;
const int buzzerDuration = 750;
const byte shortDuration = 250;

/// Game state

bool gameOn = false;

#define SPACE 0
#define WALL 1
#define PLAYER 2
#define BOMB 3

/// Walls variables

float fillPercentage;
byte numberOfWalls;

// Win and lose custom drawing

const uint64_t winAndLose[] = {
  0x3c42810000666600,
  0x81423c0000666600
};

bool updateLCD = true;

/// Score variables
int score;
byte scoreCounter;
const byte defaultScore = 10;
const float scoreMultiplier = 0.1;

/// Special bombs
bool dirtBomb = false;
byte numberOfDirtBombsDifficulty[numberOfDifficulties] = { 6, 4, 2, 3 };
byte numberOfDirtBombs;

/// Matrix directions
bool lastXMatrix;
bool lastYMatrix;

/// HighScore varibables
bool screenOneOnce = true;
bool screenTwoOnce = true;

const int lenghtPrevString = 12;

void setup() {
  pinMode(leftButton, INPUT_PULLUP);
  pinMode(rightButton, INPUT_PULLUP);
  pinMode(mainButton, INPUT_PULLUP);

  /// we get the data from eeprom
  EEPROM.get(lcdBrightnessByte, lcdBrightness);
  EEPROM.get(matrixBrightnessByte, matrixBrightness);
  EEPROM.get(soundStateByte, soundState);
  EEPROM.get(currentDifficultyByte, currentDifficulty);

  for (int player = 0; player < numberOfHighscores; player++) {
    for (int character = 0; character < nameSize; character++) {
      EEPROM.get(playerNameBytes[player][character], playerName[player][character]);
    }
    EEPROM.get(playerScoreByte[player], playerScore[player]);
  }


  /// we start led matrix
  lc.shutdown(0, false);
  lc.setIntensity(0, matrixBrightness);
  lc.clearDisplay(0);

  /// we initialize the custom lcd characters
  lcd.createChar(1, arrowUP);
  lcd.createChar(2, arrowDown);
  lcd.createChar(3, normalBombIcon);
  lcd.createChar(4, dirtBombIcon);

  lcd.createChar(5, rightDown);
  lcd.createChar(6, leftDown);
  lcd.createChar(7, rightUp);
  lcd.createChar(8, leftUp);

  /// we start the lcd
  lcd.begin(16, 2);
  lcd.clear();
  pinMode(A, OUTPUT);
  analogWrite(A, lcdBrightness * lcdMultiplier);

  /// and print the Welcome message
  lcd.setCursor(0, 0);
  lcd.print(F("Welcome to"));
  lcd.setCursor(0, 1);
  lcd.print(F("Casoleta gaming!"));
  displayImage(intro);
}

void loop() {
  /// We display the menu after a delay
  if (millis() > delayBetweenMessages) {
    if (introOnce == false) {
      lcd.clear();
      lcd.print(meniuOptions[currentOption]);
      arrows(currentOption, numberOfOptions);
      displayImage(drawings[currentOption]);
      introOnce = true;
    }
    if (gameOn) {
      if (playerAlive == true && numberOfWalls != 0) {
        game();
      } else if (playerAlive == true && numberOfWalls == 0) {
        highscore();
      } else {
        displayImage(winAndLose[1]);
        if (doMainButtonAction) {
          doMainButtonAction = false;
          gameOn = false;
          back();
        }
      }
    } else {
      menu();
    }
    actions();
  }
}

/// HighScore logic
void highscore() {
  displayImage(winAndLose[0]);
  /// We check if the beat any highscore
  for (int highscore = 0; highscore < numberOfHighscores; highscore++) {
    if (score > playerScore[highscore] && screenOneOnce) {
      if (!shown) {
        /// Print how many players had a lower score that owers
        shown = true;
        lcd.clear();
        lcd.print(F("New Highscore!"));
        lcd.setCursor(0, 1);
        lcd.print(F("You defeated: "));
        lcd.print(numberOfHighscores - highscore);
      }
      if (doMainButtonAction == true && screenTwoOnce) {
        /// Set a name for the highscore
        doMainButtonAction = false;
        lcd.clear();
        lcd.print(F("New Highscore!"));
        lcd.setCursor(0, 1);
        lcd.print(F("Enter name: "));
        lcd.setCursor(lenghtPrevString, 1);
        lcd.print(currentCharacter);
        while (currentCharacterNumber != nameSize) {
          actions();
          if (doLeftButtonAction == true && currentCharacter > 'A') {
            currentCharacter--;
            lcd.setCursor(lenghtPrevString + currentCharacterNumber, 1);
            lcd.print(currentCharacter);
            doLeftButtonAction = false;
          } else if (doLeftButtonAction == true && currentOption == 'A') {
            doLeftButtonAction = false;
          }
          if (doRightButtonAction == true && currentOption < 'Z') {
            currentCharacter++;
            lcd.setCursor(lenghtPrevString + currentCharacterNumber, 1);
            lcd.print(currentCharacter);
            doRightButtonAction = false;
          } else if (doRightButtonAction == true && currentOption == 'Z') {
            doRightButtonAction = false;
          }
          if (doMainButtonAction == true) {
            currentPlayer[currentCharacterNumber] = currentCharacter;
            currentCharacterNumber++;
            if (currentCharacterNumber < nameSize) {
              currentCharacter = 'A';
              lcd.setCursor(lenghtPrevString + currentCharacterNumber, 1);
              lcd.print(currentCharacter);
            }
            doMainButtonAction = false;
          }
        }
        for (int nextHighscore = numberOfHighscores - 1; nextHighscore > highscore; nextHighscore--) {
          playerScore[nextHighscore] = playerScore[nextHighscore - 1];
          for (int character = 0; character < nameSize; character++) {
            playerName[nextHighscore][character] = playerName[nextHighscore - 1][character];
          }
        }
        playerScore[highscore] = score;
        for (int character = 0; character < nameSize; character++) {
          playerName[highscore][character] = currentPlayer[character];
        }
        screenOneOnce = false;
        screenTwoOnce = false;
        break;
      }
    }
  }
  if (doMainButtonAction) {
    /// Save the highscores to eeprom
    for (int player = 0; player < numberOfHighscores; player++) {
      for (int character = 0; character < nameSize; character++) {
        EEPROM.put(playerNameBytes[player][character], playerName[player][character]);
      }
      EEPROM.put(playerScoreByte[player], playerScore[player]);
    }
    doMainButtonAction = false;
    gameOn = false;
    /// Go back to the main menu
    back();
  }
}

// Menu logic and printing function
void menu() {
  // We nagivate the menu using the right and left buttons
  if (optionMainMenu == 0) {
    if (doLeftButtonAction == true && currentOption > 0) {
      lcd.clear();
      if (soundState) {
        // We mark each move with a sound
        tone(buzzerPin, buzzerTone, shortDuration);
      }
      lcd.print(meniuOptions[--currentOption]);
      arrows(currentOption, numberOfOptions);
      displayImage(drawings[currentOption]);
      doLeftButtonAction = false;
    } else if (doLeftButtonAction == true && currentOption == 0) {
      // We validate the moves
      doLeftButtonAction = false;
    }

    if (doRightButtonAction == true && currentOption < numberOfOptions - 1) {
      lcd.clear();
      if (soundState) {
        tone(buzzerPin, buzzerTone, shortDuration);
      }
      lcd.print(meniuOptions[++currentOption]);
      arrows(currentOption, numberOfOptions);
      displayImage(drawings[currentOption]);
      doRightButtonAction = false;
    } else if (doRightButtonAction == true && currentOption == numberOfOptions - 1) {
      doRightButtonAction = false;
    }
    // We can choose the option in the meniu with the main button
    if (doMainButtonAction == true) {
      optionMainMenu = currentOption + 1;
      doMainButtonAction = false;
    }
  }

  switch (optionMainMenu) {
    /// The first option in the menu is for starting the game
    case 1:
      /// We make sure the map, player and score are valid
      gameOn = true;
      playerAlive = true;
      updateLCD = true;
      validSpawn = false;
      score = 0;
      screenOneOnce = true;
      screenTwoOnce = true;
      resetMatrix();
      fillPercentage = mapCovrageDifficulty[currentDifficulty];
      numberOfDirtBombs = numberOfDirtBombsDifficulty[currentDifficulty];
      generateMap();
      break;
    /// The second option is the highscore
    case 2:
      /// We nagivate this submenu using the right and left buttons in order to see the highscores
      if (shown == false) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("Highscore:"));
        lcd.setCursor(0, 1);
        lcd.print(optionSubMenu + 1);
        lcd.print(F("."));
        for (int character = 0; character < nameSize; character++) {
          lcd.print(playerName[optionSubMenu][character]);
        }
        lcd.print(F(": "));
        lcd.print(playerScore[optionSubMenu]);
        arrows(optionSubMenu, numberOfHighscores);
        shown = true;
      }
      if (doRightButtonAction == true && optionSubMenu != numberOfHighscores - 1) {
        doRightButtonAction = false;
        optionSubMenu++;
        shown = false;
      } else if (doRightButtonAction == true && optionSubMenu == numberOfHighscores - 1) {
        doRightButtonAction = false;
      }
      if (doLeftButtonAction == true && optionSubMenu != 0) {
        doLeftButtonAction = false;
        optionSubMenu--;
        shown = false;
      } else if (doLeftButtonAction == true && optionSubMenu == 0) {
        doLeftButtonAction = false;
      }
      /// We exit this submenu with the main button
      if (doMainButtonAction == true) {
        doMainButtonAction = false;
        optionSubMenu = 0;
        back();
      }
      break;
      /// Third option is for settings
    case 3:
      /// We nagivate this submenu using the right and left buttons in order to see the options
      if (shown == false && optionSubMenu == 0) {
        lcd.clear();
        lcd.print(settingsOptions[currentOptionSubMenu]);
        arrows(currentOptionSubMenu, numberOfSettings);
        shown = true;
      }
      if (optionSubMenu == 0) {
        if (doLeftButtonAction == true && currentOptionSubMenu != 0) {
          currentOptionSubMenu--;
          if (soundState) {
            // We mark each move with a sound
            tone(buzzerPin, buzzerTone, shortDuration);
          }
          doLeftButtonAction = false;
          shown = false;
        } else if (doLeftButtonAction == true && currentOptionSubMenu == 0) {
          doLeftButtonAction = false;
        }
        if (doRightButtonAction == true && currentOptionSubMenu < numberOfSettings - 1) {
          currentOptionSubMenu++;
          if (soundState) {
            // We mark each move with a sound
            tone(buzzerPin, buzzerTone, shortDuration);
          }
          doRightButtonAction = false;
          shown = false;
        } else if (doRightButtonAction == true && currentOptionSubMenu == numberOfSettings - 1) {
          doRightButtonAction = false;
        }
        /// We choose the option with the main button
        if (doMainButtonAction == true) {
          optionSubMenu = currentOptionSubMenu + 1;
          shown = false;
          doMainButtonAction = false;
        }
      }
      switch (optionSubMenu) {
        /// We can adjust the lcd brightness using the right/left buttons
        case 1:
          if (shown == false) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(settingsOptions[optionSubMenu - 1]);
            lcd.print(F(":"));
            lcd.setCursor(0, 1);
            lcd.print(lcdBrightness);
            arrows(lcdBrightness, maxLCDBrightness);
            shown = true;
          }
          if (doRightButtonAction == true && lcdBrightness != maxLCDBrightness - 1) {
            doRightButtonAction = false;
            lcdBrightness++;
            analogWrite(A, lcdBrightness * lcdMultiplier);
            shown = false;
          } else if (doRightButtonAction == true && lcdBrightness == maxLCDBrightness - 1) {
            doRightButtonAction = false;
          }
          if (doLeftButtonAction == true && lcdBrightness != 0) {
            doLeftButtonAction = false;
            lcdBrightness--;
            analogWrite(A, lcdBrightness * lcdMultiplier);
            shown = false;
          } else if (doLeftButtonAction == true && lcdBrightness == 0) {
            doLeftButtonAction = false;
          }
          /// We save the brightness in eeprom

          /// We exit the lcd brightness option with main button
          if (doMainButtonAction == true) {
            EEPROM.put(lcdBrightnessByte, lcdBrightness);
            doMainButtonAction = false;
            optionSubMenu = 0;
            shown = 0;
          }
          break;
          /// We can adjust the lcd brightness using the right/left buttons
        case 2:
          if (shown == false) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(settingsOptions[optionSubMenu - 1]);
            lcd.print(F(":"));
            lcd.setCursor(0, 1);
            lcd.print(matrixBrightness);
            arrows(matrixBrightness, maxMatrixBrightness);
            shown = true;
          }
          if (doRightButtonAction == true && matrixBrightness != maxMatrixBrightness - 1) {
            doRightButtonAction = false;
            matrixBrightness++;
            lc.setIntensity(0, matrixBrightness);
            shown = false;
          } else if (doRightButtonAction == true && matrixBrightness == maxMatrixBrightness - 1) {
            doRightButtonAction = false;
          }
          if (doLeftButtonAction == true && matrixBrightness != 0) {
            doLeftButtonAction = false;
            matrixBrightness--;
            lc.setIntensity(0, matrixBrightness);
            shown = false;
          } else if (doLeftButtonAction == true && matrixBrightness == 0) {
            doLeftButtonAction = false;
          }
          /// We save the brightness in eeprom

          /// We exit the matrix brightness option with main button
          if (doMainButtonAction == true) {
            EEPROM.put(matrixBrightnessByte, matrixBrightness);
            doMainButtonAction = false;
            optionSubMenu = 0;
            shown = false;
          }
          break;
        case 3:
          /// We can choose if we want sound on or off with right or left buttons
          if (shown == false) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(settingsOptions[optionSubMenu - 1]);
            lcd.print(F(":"));
            lcd.setCursor(0, 1);
            lcd.print(OnAndOff[soundState]);
            arrows(soundState, yesOrNo);
            shown = true;
          }
          if (doRightButtonAction == true && soundState == false) {
            doRightButtonAction = false;
            soundState = true;
            shown = false;
          } else if (doRightButtonAction == true && soundState == true) {
            doRightButtonAction = false;
          }
          if (doLeftButtonAction == true && soundState == true) {
            doLeftButtonAction = false;
            soundState = false;
            shown = false;
          } else if (doLeftButtonAction == true && soundState == false) {
            doLeftButtonAction = false;
          }
          /// We save it in eeprom

          /// We exit the sound option with main button
          if (doMainButtonAction == true) {
            EEPROM.put(soundStateByte, soundState);
            doMainButtonAction = false;
            optionSubMenu = 0;
            shown = false;
          }
          break;
        case 4:
          /// We can choose the difficulty with right or left buttons
          if (shown == false) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(settingsOptions[optionSubMenu - 1]);
            lcd.print(F(":"));
            lcd.setCursor(0, 1);
            lcd.print(difficulties[currentDifficulty]);
            arrows(currentDifficulty, numberOfDifficulties);
            shown = true;
          }
          if (doRightButtonAction == true && currentDifficulty != numberOfDifficulties - 1) {
            doRightButtonAction = false;
            currentDifficulty++;
            shown = false;
          } else if (doRightButtonAction == true && currentDifficulty == numberOfDifficulties - 1) {
            doRightButtonAction = false;
          }
          if (doLeftButtonAction == true && currentDifficulty != 0) {
            doLeftButtonAction = false;
            currentDifficulty--;
            shown = false;
          } else if (doLeftButtonAction == true && currentDifficulty == 0) {
            doLeftButtonAction = false;
          }
          /// We can exit with main button
          if (doMainButtonAction == true) {
            /// We save the difficulty to eeprom
            EEPROM.put(currentDifficultyByte, currentDifficulty);
            doMainButtonAction = false;
            optionSubMenu = 0;
            shown = false;
          }
          break;
        case 5:
        /// We can choose if we want to reset the highscores
          if (shown == false) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(settingsOptions[optionSubMenu - 1]);
            lcd.print(F(":"));
            lcd.setCursor(0, 1);
            if (reset)
              lcd.print(F("Yes"));
            else
              lcd.print(F("No"));
            arrows(reset, yesOrNo);
            shown = true;
          }
          if (doRightButtonAction == true && reset == false) {
            doRightButtonAction = false;
            reset = true;
            shown = false;
          } else if (doRightButtonAction == true && reset == true) {
            doRightButtonAction = false;
          }
          if (doLeftButtonAction == true && reset == true) {
            doLeftButtonAction = false;
            reset = false;
            shown = false;
          } else if (doLeftButtonAction == true && reset == false) {
            doLeftButtonAction = false;
          }
          /// We exit the reset hishscores option with main button
          if (doMainButtonAction == true) {
            if (reset == true) {
              for (int player = 0; player < numberOfHighscores; player++) {
                for (int character = 0; character < nameSize; character++) {
                  /// Save the AAA 0 scores to eeprom
                  playerName[player][character] = 'A';
                  EEPROM.put(playerNameBytes[player][character], playerName[player][character]);
                }
                playerScore[player] = 0;
                EEPROM.put(playerScoreByte[player], playerScore[player]);
              }
            }
            doMainButtonAction = false;
            optionSubMenu = 0;
            shown = false;
          }
          break;
        /// Exit: we go to the main menu
        case 6:
          back();
          break;
        default:
          break;
      }
      break;
      /// We can see data about the game and the develover
    case 4:
      if (millis() - lastMessageShown > delayBetweenMessages) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(info[message]);
        lcd.setCursor(0, 1);
        lcd.print(info[message + 1]);
        lastMessageShown = millis();
        if (message < numberOfInfo - 2) {
          message++;
        } else if (message == numberOfInfo - 2) {
          message = 0;
        }
      }
      /// We go back to the main menu with main button
      if (doMainButtonAction == true) {
        doMainButtonAction = false;
        back();
      }
      break;
      /// We can see the game rules
    case 5:
      if (millis() - lastMessageShown > delayBetweenMessages) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(howToPlay[message]);
        lcd.setCursor(0, 1);
        lcd.print(howToPlay[message + 1]);
        lastMessageShown = millis();
        if (message < numberOfMessages - 2) {
          message++;
        } else if (message == numberOfMessages - 2) {
          message = 0;
        }
      }
      /// We go back to the main menu with main button
      if (doMainButtonAction == true) {
        doMainButtonAction = false;
        back();
      }
      break;
    default:
      break;
  }
}


/// The back function is used when we want to go back to the previous menu
/// and makes sure that everything is fine
void back() {
  optionMainMenu = 0;
  optionSubMenu = 0;
  shown = 0;
  currentOptionSubMenu = 0;
  message = 0;
  displayImage(drawings[currentOption]);
  lcd.clear();
  lcd.print(meniuOptions[currentOption]);
  arrows(currentOption, numberOfOptions);
}

/// We print the arrow to the side of the lcd in order to know if we can
/// go up or down on the menu
void arrows(byte option, byte maxNumber) {
  if (option != 0) {
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  }
  if (option != maxNumber - 1) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
  }
}

/// Game logic and rendering function
void game() {
  renderMatrix();
  move();
  chooseBomb();
  explosions();
  if (updateLCD) {
    renderLCD();
    updateLCD = false;
  }
}

/// The move functions move the player and sometimes leave a bomb behind
/// its a function that we reused from a previos homework
void move() {
  // We read the Joystick inputs and move the player
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);
  if (joyMoved == false) {
    if (xValue < minInputThreshold) {
      // We verify that the player remains in the matrix
      if (matrix[playerX - 1][playerY] == SPACE && playerX - 1 >= 0) {
        // If the button is pressed we place a bomb behind us
        if (doMainButtonAction == true && (dirtBomb == false || (dirtBomb == true && numberOfDirtBombs > 0)) && activeBomb == false) {
          matrix[playerX][playerY] = BOMB;
          if (dirtBomb == true)
            numberOfDirtBombs--;
          bombCounter = millis();
          activeBomb = true;
          doMainButtonAction = false;
          bombX = playerX;
          bombY = playerY;
        } else {
          matrix[playerX][playerY] = SPACE;
        }
        playerX--;
        matrix[playerX][playerY] = PLAYER;
        // The player led is ON on the next position
        playerBlink(playerX % matrixRealSize, playerY % matrixRealSize, true);
      }
      joyMoved = true;
    } else if (xValue > maxInputThreshold) {
      if (matrix[playerX + 1][playerY] == SPACE && playerX + 1 < matrixGameSize) {
        if (doMainButtonAction == true && (dirtBomb == false || (dirtBomb == true && numberOfDirtBombs > 0)) && activeBomb == false) {
          matrix[playerX][playerY] = BOMB;
          if (dirtBomb == true)
            numberOfDirtBombs--;
          bombCounter = millis();
          activeBomb = true;
          doMainButtonAction = false;
          bombX = playerX;
          bombY = playerY;
        } else {
          matrix[playerX][playerY] = SPACE;
        }
        playerX++;
        matrix[playerX][playerY] = PLAYER;
        playerBlink(playerX % matrixRealSize, playerY % matrixRealSize, true);
      }
      joyMoved = true;
    } else if (yValue < minInputThreshold) {
      if (matrix[playerX][playerY - 1] == SPACE && playerY - 1 >= 0) {
        if (doMainButtonAction == true && (dirtBomb == false || (dirtBomb == true && numberOfDirtBombs > 0)) && activeBomb == false) {
          matrix[playerX][playerY] = BOMB;
          if (dirtBomb == true)
            numberOfDirtBombs--;
          bombCounter = millis();
          activeBomb = true;
          doMainButtonAction = false;
          bombX = playerX;
          bombY = playerY;
        } else {
          matrix[playerX][playerY] = SPACE;
        }
        playerY--;
        matrix[playerX][playerY] = PLAYER;
        playerBlink(playerX % matrixRealSize, playerY % matrixRealSize, true);
      }
      joyMoved = true;
    } else if (yValue > maxInputThreshold) {
      if (matrix[playerX][playerY + 1] == SPACE && playerY + 1 < matrixGameSize) {
        if (doMainButtonAction == true && (dirtBomb == false || (dirtBomb == true && numberOfDirtBombs > 0)) && activeBomb == false) {
          matrix[playerX][playerY] = BOMB;
          if (dirtBomb == true)
            numberOfDirtBombs--;
          bombCounter = millis();
          activeBomb = true;
          doMainButtonAction = false;
          bombX = playerX;
          bombY = playerY;
        } else {
          matrix[playerX][playerY] = SPACE;
        }
        playerY++;
        matrix[playerX][playerY] = PLAYER;
        playerBlink(playerX % matrixRealSize, playerY % matrixRealSize, true);
      }
      // We mark that the joystick moved
      joyMoved = true;
    }
    // If the joystick is in the idle position again we can make a new move after
  } else if ((xValue >= minIdleThreshold && xValue <= maxIdleThreshold)
             && (yValue >= minIdleThreshold && yValue <= maxIdleThreshold)) {
    joyMoved = false;
  }
}

void renderMatrix() {
  /// We check if the went to a different zone of the matrix
  if (lastXMatrix != (playerX / matrixRealSize)) {
    lastXMatrix = (playerX / matrixRealSize);
    updateLCD = true;
  }
  if (lastYMatrix != (playerY / matrixRealSize)) {
    lastYMatrix = (playerY / matrixRealSize);
    updateLCD = true;
  }
  // We go through each tile of the matrix and set it accordingly
  for (int i = 0; i < matrixRealSize; i++) {
    for (int j = 0; j < matrixRealSize; j++) {
      switch (matrix[i + lastXMatrix * matrixRealSize][j + lastYMatrix * matrixRealSize]) {
        case SPACE:
          lc.setLed(0, i, j, 0);
          break;
        case WALL:
          lc.setLed(0, i, j, 1);
          break;
        case PLAYER:
          // The player blinks
          if (millis() - lastBlinkPlayer >= playerBlinkDelay) {
            playerBlink(i, j, false);
          }
          break;
        case BOMB:
          // The bomb blinks
          if (millis() - lastBlinkBomb >= bombBlinkDelay) {
            bombState = !bombState;
            lc.setLed(0, i, j, bombState);
            lastBlinkBomb = millis();
          }
        default:
          break;
      }
    }
  }
}

void playerBlink(int i, int j, bool moved) {
  // If the player moves the led is ON in order to be more visible
  if (moved == true) {
    playerState = true;
  } else {
    // else we change the change its state
    playerState = !playerState;
  }
  lc.setLed(0, i, j, playerState);
  lastBlinkPlayer = millis();
}

void chooseBomb() {
  // We can choose between the normal bomb or the dirt bombs
  if (doRightButtonAction == true && dirtBomb == false) {
    doRightButtonAction = false;
    dirtBomb = true;
    updateLCD = true;
  } else if (doRightButtonAction == true && dirtBomb == true) {
    doRightButtonAction = false;
  }
  if (doLeftButtonAction == true && dirtBomb == true) {
    doLeftButtonAction = false;
    dirtBomb = false;
    updateLCD = true;
  } else if (doLeftButtonAction == true && dirtBomb == false) {
    doLeftButtonAction = false;
  }
}

void explosions() {
  // The bomb explodes after 2 seconds if it is active
  if (millis() - bombCounter >= explosionDelay && activeBomb == true) {
    // It explodes in a 3X3 square area
    for (int explosionRadiusX = -1; explosionRadiusX <= 1; explosionRadiusX++) {
      for (int explosionRadiusY = -1; explosionRadiusY <= 1; explosionRadiusY++) {
        // We make sure that the bomb cant explode the other side of the map if it is placed near an edge
        if (bombX + explosionRadiusX < matrixGameSize && bombX + explosionRadiusX >= 0 && bombY + explosionRadiusY < matrixGameSize && bombY + explosionRadiusY >= 0) {
          if (dirtBomb == false) {
            if (matrix[bombX + explosionRadiusX][bombY + explosionRadiusY] == WALL) {
              // We destory all the tiles in the radius
              matrix[bombX + explosionRadiusX][bombY + explosionRadiusY] = SPACE;
              numberOfWalls--;
              scoreCounter++;
            }
            matrix[bombX][bombY] = SPACE;
          } else {
            if (matrix[bombX + explosionRadiusX][bombY + explosionRadiusY] == SPACE) {
              // We wall all the tiles in the radius
              matrix[bombX + explosionRadiusX][bombY + explosionRadiusY] = WALL;
              numberOfWalls++;
            } else if (matrix[bombX + explosionRadiusX][bombY + explosionRadiusY] == BOMB) {
              matrix[bombX + explosionRadiusX][bombY + explosionRadiusY] = WALL;
              numberOfWalls++;
            }
          }
          if (matrix[bombX + explosionRadiusX][bombY + explosionRadiusY] == PLAYER) {
            // If the player is in the radius of the bomb he dies and its game over
            playerAlive = false;
          }
        }
      }
    }
    // We mark the explosion with a buzzer sound
    if (soundState) {
      tone(buzzerPin, buzzerTone, buzzerDuration);
    }
    score += defaultScore * scoreCounter * (1.0 + (scoreCounter - 1) * scoreMultiplier);
    scoreCounter = 0;
    activeBomb = false;
    updateLCD = true;
  }
}

void renderLCD() {
  /// We render on the lcd game info
  lcd.clear();
  lcd.print(F("Walls:"));
  lcd.print(numberOfWalls);
  lcd.setCursor(10, 0);
  lcd.print(F("Use:"));
  if (dirtBomb == true) {
    lcd.write(byte(4));
    lcd.print(numberOfDirtBombs);
  } else {
    lcd.write(byte(3));
  }
  lcd.setCursor(0, 1);
  lcd.print(F("Score:"));
  lcd.print(score);

  /// We get the direction we can go to
  lcd.setCursor(12, 1);
  if (playerX / matrixRealSize == 0 && playerY / matrixRealSize == 0) {
    lcd.write(byte(5));
  }
  if (playerX / matrixRealSize == 0 && playerY / matrixRealSize == 1) {
    lcd.write(byte(6));
  }
  if (playerX / matrixRealSize == 1 && playerY / matrixRealSize == 0) {
    lcd.write(byte(7));
  }
  if (playerX / matrixRealSize == 1 && playerY / matrixRealSize == 1) {
    lcd.write(byte(8));
  }
}

void generateMap() {
  // We choose a random seed using an empty analog port
  randomSeed(analogRead(A5));
  // We calculate how many tiles we fill with walls
  int numElementsToFill = fillPercentage * matrixGameSize * matrixGameSize;
  numberOfWalls = numElementsToFill;

  for (int count = 0; count < numElementsToFill;) {
    int randomRow = random(matrixGameSize);
    int randomCol = random(matrixGameSize);

    // Check if the element is not already filled
    if (matrix[randomRow][randomCol] == SPACE) {
      matrix[randomRow][randomCol] = WALL;
      count++;
    }
  }

  // We spawn the player and make sure that he can place at least a bomb without dying
  playerX = random(matrixGameSize);
  lastXMatrix = playerX / matrixRealSize;
  playerY = random(matrixGameSize);
  lastYMatrix = playerY / matrixRealSize;
  if (matrix[playerX][playerY] == WALL) {
    numberOfWalls--;
  }
  matrix[playerX][playerY] = PLAYER;
  const int numberOfDirections = 4;
  // In order to do that we make a 2 tiles space in a random direction
  while (!validSpawn) {
    int direction = random(numberOfDirections);
    switch (direction) {
      case 1:
        if (playerX - minSafeDistance >= 0) {
          for (int distance = 1; distance <= minSafeDistance; distance++) {
            if (matrix[playerX - distance][playerY] == WALL) {
              numberOfWalls--;
            }
            matrix[playerX - distance][playerY] = SPACE;
          }
          validSpawn = true;
        }
        break;
      case 2:
        if (playerY - minSafeDistance >= 0) {
          for (int distance = 1; distance <= minSafeDistance; distance++) {
            if (matrix[playerX][playerY - distance] == WALL) {
              numberOfWalls--;
            }
            matrix[playerX][playerY - distance] = SPACE;
          }
          validSpawn = true;
        }
        break;
      case 3:
        if (playerX + minSafeDistance < matrixGameSize) {
          for (int distance = 1; distance <= minSafeDistance; distance++) {
            if (matrix[playerX + distance][playerY] == WALL) {
              numberOfWalls--;
            }
            matrix[playerX + distance][playerY] = SPACE;
          }
          validSpawn = true;
        }
        break;
      case 4:
        if (playerY + minSafeDistance < matrixGameSize) {
          for (int distance = 1; distance <= minSafeDistance; distance++) {
            if (matrix[playerX][playerY + distance] == WALL) {
              numberOfWalls--;
            }
            matrix[playerX][playerY + distance] = SPACE;
          }
          validSpawn = true;
        }
        break;
    }
  }
}

void resetMatrix() {
  /// We use this function to reset the matrix between games
  for (byte i = 0; i < matrixGameSize; ++i) {
    for (byte j = 0; j < matrixGameSize; ++j) {
      matrix[i][j] = 0;
    }
  }
}

void actions() {
  /// Buttons logic
  pressLeftButton();
  pressRightButton();
  pressMainButton();
}

void pressRightButton() {
  readingRightButton = digitalRead(rightButton);

  if (readingRightButton != lastReadingRightButton) {
    lastDebounceRightButton = millis();
  }

  if (millis() - lastDebounceRightButton > buttonPressDebounce) {
    if (readingRightButton != stateRightButton) {
      stateRightButton = readingRightButton;
      if (stateRightButton == LOW) {
        doRightButtonAction = true;
      }
    }
  }
  lastReadingRightButton = readingRightButton;
}

void pressLeftButton() {
  readingLeftButton = digitalRead(leftButton);

  if (readingLeftButton != lastReadingLeftButton) {
    lastDebounceLeftButton = millis();
  }

  if (millis() - lastDebounceLeftButton > buttonPressDebounce) {
    if (readingLeftButton != stateLeftButton) {
      stateLeftButton = readingLeftButton;
      if (stateLeftButton == LOW) {
        doLeftButtonAction = true;
      }
    }
  }
  lastReadingLeftButton = readingLeftButton;
}

void pressMainButton() {
  readingMainButton = digitalRead(mainButton);

  if (readingMainButton != lastReadingMainButton) {
    lastDebounceMainButton = millis();
  }

  if (millis() - lastDebounceMainButton > buttonPressDebounce) {
    if (readingMainButton != stateMainButton) {
      stateMainButton = readingMainButton;
      if (stateMainButton == LOW) {
        doMainButtonAction = true;
        if (soundState && activeBomb == false) {
          tone(buzzerPin, buzzerTone, shortDuration);
        }
      }
    }
  }
  lastReadingMainButton = readingMainButton;
}

// We use this function to display the custom drawings on the LED Matrix
void displayImage(uint64_t image) {
  for (int i = 0; i < matrixRealSize; i++) {
    byte row = (image >> i * matrixRealSize) & 0xFF;
    for (int j = 0; j < matrixRealSize; j++) {
      lc.setLed(0, i, j, bitRead(row, j));
    }
  }
}