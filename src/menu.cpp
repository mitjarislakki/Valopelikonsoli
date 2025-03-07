#include <Arduino.h>
#include <FastLED.h>
#include <stdlib.h>
#include <Wire.h>
#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header

#include "patterns.h"

// Print debug information to serial
constexpr bool debug = true;

#define MOLE_TIMER 30 // Length of whack-a-mole game in seconds
#define SPEED_TIMER 30 // Length of speed game in seconds

//Screen settings:
hd44780_I2Cexp p1LCD;
hd44780_I2Cexp p2LCD;
const int LCD_COLS = 16;
const int LCD_ROWS = 2;

#define MOLE_MIN (long)500
#define MOLE_MAX (long)2000

//LED settings:
#define LED_PIN 7
#define LED_TYPE NEOPIXEL
#define COLOR_ORDER GRB
#define NUM_LEDS 16
CRGB leds[NUM_LEDS];

/*
Input Matrix

kytke diodit niin että virta menee row → col:
row → napit → IN4148 diodit → col + 10k pulldown
*/

// Matrix row/col pins
const int row[] = {8,9,10,11};
const int col[] = {2,3,4,5};
const int size = 4;
int keys[size*size];
int lastKeys[size*size];
constexpr long debounceDelay = 200;
unsigned long lastDebounce[size*size] = {0};


// Players
Player p1;
Player p2;

//Menu
int mode = 0;
String modes[] = {"Memory Game", "Whac-a-mole", "Speed Game"};
int selection = -1;

unsigned long startTime = 0;

//paskapeli
int currentP1 = -1;
int lastP1 = -1;
int currentP2 = -1;
int lastP2 = -1;

//moles
int currentMoles[] = {0,0,0,0,0,0,0};
unsigned long nextMole = 0;

// Player LED numbers
const int p1LED[] = {0,1,2,3,4,5,6};
const int p2LED[] = {7,12,11,10,9,8,13};

// Player matrix pins
const int p1Keys[] = {0,1,2,3,4,5,6};
const int p2Keys[] = {7,13,12,10,9,8,14};

// patterns
#define PATTERN_LENGTH 100
#define PATTERN_INTERVAL 500
int pattern[PATTERN_LENGTH];

// put function declarations here:

void updateKeys(unsigned long);
void showScores();
void paskapeli();
void clearLeds();
bool onKeyDown(int key);
void updateScreens(String disp);
void menu();
void showPatterns(unsigned long);
void initializePlayer(Player*);
void nextPattern(Player*, const int*);
void processInputs(Player*, const int*);
void timedGame(int game, int timer, unsigned long currentTime);
void moles(unsigned long currentTime);
void failPlayer(Player*, const int*);
void endGame();


void initializePlayer(Player* player){
    player->inPattern = true,
    player->patternStart = true,
    player->patternIndex = 0,
    player->patternMax = 1,
    player->prevTime = 0,
    player->score = 0;
}

void setup() { 
    initializePlayer(&p1);
    initializePlayer(&p2);

    randomSeed(analogRead(0)); //randomise seed to avoid every time being the same after reset
    
    // Set up the pattern
    for (int i = 0; i < PATTERN_LENGTH; i++){
        pattern[i] = random(0, 7);
    }

    Serial.begin(9600); //enable serial for debugging
    //pin setup for button matrix:
    for(int i=0; i < size; i++){
        pinMode(row[i], OUTPUT);
        pinMode(col[i], INPUT);
    }
    //led strip initialization
    FastLED.addLeds<LED_TYPE, LED_PIN>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

    //clear leds
    clearLeds();

    //LCD initialization
    int status1;
    int status2;
        
    status2 = p2LCD.begin(LCD_COLS, LCD_ROWS);
    status1 = p1LCD.begin(LCD_COLS, LCD_ROWS);
    if(status1) hd44780::fatalError(status1);
    if(status2) hd44780::fatalError(status2);

    // initalization was successful, the backlight should be on now
    // Print a message to the LCD
    p1LCD.print("Valopeli, P1");
    p2LCD.print("Valopeli, P2");
    delay(2000);
    updateScreens("Select Game:");
    delay(1100);
    updateScreens(""); // Set to default
}

void loop() {
    // Get current running time
    unsigned long currentTime = millis();

    // update keypress map
    updateKeys(currentTime);

    // Print keys
    if(debug){
        for(int i=0; i < size; i++){
            for(int j = 0; j < size; j++){
                Serial.print(keys[size*i+j]);
            }
                Serial.print(" ");
        }
        Serial.println();
    }

    //mode 0 = menu, 1 = memory game etc.
    if(mode == 0){
        menu();
    }
    else if(mode == 1){
        // Memory game
        // Update pattern
        showPatterns(currentTime);

        if(!p1.failed) processInputs(&p1, p1Keys);
        if(!p2.failed) processInputs(&p2, p2Keys);
        if(p1.failed && p2.failed) endGame();
    }
    else if(mode == 2){
        //Whac-a-mole
        timedGame(2, MOLE_TIMER*1000, currentTime);
    }

    else{
        //Speed game
        timedGame(3, SPEED_TIMER*1000, currentTime);
    }
    delay(10);
}

// function definitions:

//reads the state of the matrix and puts it into array keys,
// keys[i] represents whether or not i:th key is pressed
void updateKeys(unsigned long currentTime){
    for(int i=0; i < size; i++){
        for(int k=0; k < size; k++){
        digitalWrite(row[k], LOW);
        }

        digitalWrite(row[i], HIGH);
        for(int j = 0; j < size; j++){
            int state = digitalRead(col[j]);
            int btnIndex = i*size + j;
            lastKeys[btnIndex] = keys[btnIndex];
            if(state != lastKeys[btnIndex] && (currentTime - lastDebounce[btnIndex]) > debounceDelay){
                lastDebounce[btnIndex] = currentTime;
                keys[btnIndex] = state;
            }
        }
        digitalWrite(row[i], LOW);
    }
}

//updates the screens to show the points of player 1 and 2
void updateScreens(String disp){
  if(disp == ""){
    p1LCD.clear();
    p2LCD.clear();
    p1LCD.print("Player 1: " + String(p1.score));
    p2LCD.print("Player 2: " + String(p2.score));
  }
  else{
    p1LCD.clear();
    p2LCD.clear();
    p1LCD.print(disp);
    p2LCD.print(disp);
  }
}

// Returns random free pin between min and max, that's not current
int randomPin(int current, int min, int max){
    int i = random(min, max);
    while(i == current) i = random(min, max);
    return i;
}

//show player scores on bottom row of both LCDs
void showScores(){
    p1LCD.setCursor(0,1);
    p1LCD.print(String(p1.score) + " vs " + String(p2.score));
    p2LCD.setCursor(0,1);
    p2LCD.print(String(p1.score) + " vs " + String(p2.score));
}

void showPatterns(unsigned long currentTime){
    // Check if in pattern phase and interval has passed
    bool p1Next = ((unsigned long)(currentTime - p1.prevTime) >= PATTERN_INTERVAL);
    bool p2Next = ((unsigned long)(currentTime - p2.prevTime) >= PATTERN_INTERVAL);

    // Check p1
    if(p1.inPattern && p1Next){
        nextPattern(&p1, p1LED);
        p1.prevTime = currentTime; // Time check
        FastLED.show();
    }

    // Check p2
    if(p2.inPattern && p2Next){
        nextPattern(&p2, p2LED);
        p2.prevTime = currentTime; // Time check
        FastLED.show();
    }
}

// Show the next LEDs in the pattern
void nextPattern(Player* player, const int* pins){
    int i = player->patternIndex;
    if(debug) Serial.print(pattern[i]);
    if(player->patternStart){
        player->patternStart = false;
        leds[pins[pattern[i]]] = CRGB::Red;
    }
    else{
        // Set the previous LED in the pattern to off
        leds[pins[pattern[i]]] = CRGB::Black;
        player->patternIndex++;
        // Check if at the max index
        if(player->patternIndex == player->patternMax){
            player->inPattern = false; // End of pattern
            player->patternStart = true; // Start flag for next iteration
            player->patternIndex = 0;
            if(debug) Serial.println();
        }    
        else leds[pins[pattern[player->patternIndex]]] = CRGB::Red; // Next in pattern
    }
    FastLED.show();
}

// Check that the player input is the pattern
void processInputs(Player* player, const int* playerKeys){
    // Check that player in pattern phase and expected button is pressed
    if(!player->inPattern){
        int correctKey = playerKeys[pattern[player->expected]];
        // Check that no other key than the correct one is pressed
        bool failed = false;
        for(int i = 0; i < 7; i++){
            int key = playerKeys[i]; 
            if(keys[key] == HIGH && !lastKeys[key] && key != correctKey){
                failed = true;
                break;
            }
        }
        if(failed) failPlayer(player, playerKeys); // Fail the player if failure detected
        else if(keys[correctKey] == HIGH){
            player->expected++; // Increment expected button
            if(player->expected == player->patternMax){
                player->score++; // Increment player's score
                player->expected = 0; // Expect the first in pattern
                player->patternMax++; // Increment the pattern by one
                // Avoid pattern overflow (max is PATTERN_LENGTH)
                if(player->patternMax == PATTERN_LENGTH) player->patternMax = 1;
                player->inPattern = true; // Show the next pattern
                updateScreens(""); // Update screens
            }
        }

    }
}

//check for new keypress
bool onKeyDown(int key){
    if(keys[key] && !lastKeys[key]){
        return true;
    }
    else return false;
}

//handle the menu
void menu(){
    if(selection == -1){
            selection = 0;
            updateScreens(modes[0]);
            leds[p1LED[1]] = CRGB(255,255,255);
            leds[p1LED[4]] = CRGB(255,255,255);
            leds[p1LED[6]] = CRGB(255,0,0);
            FastLED.show();
        }
        if (onKeyDown(p1Keys[1])){
            selection = selection-1;
            if(selection<0){selection = 0;}
            updateScreens(modes[selection]);
        }
        else if (onKeyDown(p1Keys[4])) {
           selection = selection+1;
            if(selection>2){selection = 2;}
            updateScreens(modes[selection]); 
        }
        else if (onKeyDown(p1Keys[6])){
            clearLeds();
            mode = selection + 1;
            updateScreens("3");
            delay(700);
            updateScreens("2");
            delay(700);
            updateScreens("1");
            delay(700);
            updateScreens("");
        }
}

//turn off all leds
void clearLeds(){
    for(int i = 0; i < NUM_LEDS; i++){
        leds[i] = CRGB::Black;
    }
    FastLED.show();
}

//handle the logic for the speed game
void paskapeli(){
    if(currentP1 == -1){
        currentP1 = randomPin(lastP1,0,7);
        lastP1 = currentP1;
    }
  //do the same for player 2
    if(currentP2 == -1){
        currentP2 = randomPin(lastP2,0,7);
        lastP2 = currentP2;
    }
  leds[p1LED[currentP1]] = CRGB(255,255,255); //turn on the current leds for player 1 and 2
  leds[p2LED[currentP2]] = CRGB(255,255,255);
  FastLED.show();

  //check if player 1 is pressing the correct button, give point and set to randomise new led
  if(keys[p1Keys[currentP1]] == HIGH){
    //check if player 1 is also pressing an incorrect key, in that case, don't give points
    int pass1 = 1;
    for(int i = 1; i<7; i++){
      if(i!=currentP1 && keys[p1Keys[i]] == HIGH){
        pass1 = 0;
        break;
      }
    }
    if(pass1 == 1){
      leds[p1LED[currentP1]] = CRGB::Black;
      currentP1 = -1;
      p1.score++;
      updateScreens("");
    }
  }
  if(keys[p2Keys[currentP2]] == HIGH){
    //check if player 2 is also pressing an incorrect key, in that case, don't give points
    int pass2 = 1;
    for(int i = 1; i<7; i++){
      if(i!=currentP2 && keys[p2Keys[i]] == HIGH){
        pass2 = 0;
        break;
      }
    }
    if(pass2 == 1){
      leds[p2LED[currentP2]] = CRGB::Black;
      currentP2 = -1;
      p2.score++;
      updateScreens("");
    }
  }
}

//higher level handler for the timed games, runs game for set time, then shows results
void timedGame(int game, int timer, unsigned long currentTime){
    //game = game number
    //timer = time before end in millis
    if(startTime == 0){startTime = millis();}
    if(currentTime < startTime + timer){ //game still running
        if(game == 2) moles(currentTime);
        else if(game == 3) paskapeli();
        }
    else endGame();
}



//handle the logic for the mole game
void moles(unsigned long currentTime){
    //mole array: 0 = empty, 1 = mole, 2 = snake
    //randomize time for next mole
    if(nextMole == 0){
        nextMole = currentTime + random(MOLE_MIN, MOLE_MAX); //change mole timing here
    }
    //add mole
    if(currentTime >= nextMole){
        nextMole = 0;
        //remove old snakes
        for(int i=0; i <7; i++){
            if (currentMoles[i] == 2) currentMoles[i] = 0;
        }
        int moleType = (int)random(1,8);
        if(moleType > 6) moleType = 2;
        else moleType = 1;
        int newMole[] = {(int)random(0,7), moleType};
        currentMoles[newMole[0]] = newMole[1];
    }
    //update LEDs
    for(int i = 0; i < 7; i++){
        //player 1
        if (currentMoles[i] == 0){
            leds[p1LED[i]] = CRGB::Black;
            leds[p2LED[i]] = CRGB::Black;
        }
        else if (currentMoles[i] == 1){
            leds[p1LED[i]] = CRGB(175,255,0); //mole color
            leds[p2LED[i]] = CRGB(175,255,0); 
        }
        else {
            leds[p1LED[i]] = CRGB(0,255,0); //snake color
            leds[p2LED[i]] = CRGB(0,255,0);
        }
        FastLED.show();
    }
    //check if mole is pressed, give points
    for(int i = 0; i < 7; i++){
        //player 1
        if(onKeyDown(p1Keys[i])){
            if (currentMoles[i] == 1) p1.score++;
            else if(currentMoles[i] == 2) p1.score--;
            //player 1 and 2
            if (onKeyDown(p2Keys[i])){
                if (currentMoles[i] == 1) p2.score++;
                else if(currentMoles[i] == 2) p2.score--;
            }
            currentMoles[i] = 0;
            updateScreens("");
        }
        //player 2 only
        else if(onKeyDown(p2Keys[i])){
            if (currentMoles[i] == 1) p2.score++;
            else if(currentMoles[i] == 2) p2.score--;
            currentMoles[i] = 0;
            updateScreens("");
        }
    }
    
}

void failPlayer(Player* player, const int* playerKeys){
    player->failed = true;
    for(int i = 0; i < 7; i++){
        leds[playerKeys[i]] = CRGB::Green;
    }
    FastLED.show();
}

void endGame(){
    if(p1.score > p2.score){ //game ended, p1 won
        updateScreens("Player 1 won!");
        showScores();
        for(int i=0; i<7;i++){
            leds[p1LED[i]] = CRGB(255,0,0);
            leds[p2LED[i]] = CRGB(0,255,0);
        }
        FastLED.show();
        delay(60000);
    }
    else if(p2.score > p1.score){
        updateScreens("Player 2 won!");
        showScores();
        for(int i=0; i<7;i++){
            leds[p2LED[i]] = CRGB(255,0,0);
            leds[p1LED[i]] = CRGB(0,255,0);
        }
        FastLED.show();
        delay(60000);}
    else{
        updateScreens("Tie!");
        showScores();
        for(int i=0; i<7;i++){
            leds[p1LED[i]] = CRGB(175,255,0);
            leds[p2LED[i]] = CRGB(175,255,0);
        }
        FastLED.show();
        delay(60000);}
}