#include <Arduino.h>
#include <FastLED.h>
#include <stdlib.h>
#include <Wire.h>
#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header

#include "patterns.h"

// Print debug information to serial
constexpr bool debug = true;

//Näyttöjen asetukset:
hd44780_I2Cexp p1LCD;
hd44780_I2Cexp p2LCD;
const int LCD_COLS = 16;
const int LCD_ROWS = 2;

//Ledien asetukset:
#define LED_PIN 7
#define LED_TYPE NEOPIXEL
#define COLOR_ORDER GRB
#define NUM_LEDS 31
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


// Players
Player p1;
Player p2;

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

void updateKeys();
void updateScreens();
void showPatterns(unsigned long);
void initializePlayer(Player*);
void nextPattern(Player*, const int*);
void processInputs(Player*, const int*);


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
    for(int i = 0; i < NUM_LEDS; i++){
        leds[i] = CRGB::Black;
    }
    FastLED.show();

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
    p1LCD.clear(); p1LCD.print("3");
    p2LCD.clear(); p2LCD.print("3");
    delay(700);
    p1LCD.clear(); p1LCD.print("2");
    p2LCD.clear(); p2LCD.print("2");
    delay(700);
    p1LCD.clear(); p1LCD.print("1");
    p2LCD.clear(); p2LCD.print("1");
    delay(700);
    updateScreens();
}

void loop() {
    // Get current running time
    unsigned long currentTime = millis();

    // Update pattern
    showPatterns(currentTime);
    // update keypress map
    updateKeys();

    // Print keys
    for(int i=0; i < size; i++){
    for(int j = 0; j < size; j++){
        Serial.print(keys[size*i+j]);
    }
        Serial.print(" ");
    }
    Serial.println();

    processInputs(&p1, p1Keys);
    processInputs(&p2, p2Keys);
    delay(10);
}

// function definitions:

//reads the state of the matrix and puts it into array keys,
// keys[i] represents whether or not i:th key is pressed
void updateKeys(){
    for(int i=0; i < size; i++){
        for(int k=0; k < size; k++){
        digitalWrite(row[k], LOW);
        }

        digitalWrite(row[i], HIGH);
        for(int j = 0; j < size; j++){
            int state = digitalRead(col[j]);
            if(state == HIGH){
                keys[i*size+j] = HIGH;
            }
            else{
                keys[i*size+j] = LOW;
            }
        }
        digitalWrite(row[i], LOW);
    }

  
}

//updates the screens to show the points of player 1 and 2
void updateScreens(){
  p1LCD.clear();
  p2LCD.clear();
  p1LCD.print("Player 1: " + String(p1.score));
  p2LCD.print("Player 2: " + String(p2.score));
}

// Returns random free pin between min and max, that's not current
int randomPin(int current, int min, int max){
    int i = random(min, max);
    while(i == current) i = random(min, max);
    return i;
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
    if(!player->inPattern && keys[playerKeys[pattern[player->expected]]] == HIGH){
        player->expected++; // Increment expected button
        if(player->expected == player->patternMax){
            player->score++; // Increment player's score
            player->expected = 0; // Expect the first in pattern
            player->patternMax++; // Increment the pattern by one
            // Avoid pattern overflow (max is PATTERN_LENGTH)
            if(player->patternMax == PATTERN_LENGTH) player->patternMax = 1;
            player->inPattern = true; // Show the next pattern
            updateScreens(); // Update screens
        }
    }
}