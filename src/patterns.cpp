#include <Arduino.h>
#include <FastLED.h>
#include <stdlib.h>
#include <Wire.h>
#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header

//Näyttöjen asetukset:
hd44780_I2Cexp lcd1;
hd44780_I2Cexp lcd2;
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

// Number of LEDs
const int numLeds = 31;

// Player LED numbers
const int LedP1[] = {0,1,2,3,4,5,6};
const int LedP2[] = {7,8,9,10,11,12,13};

// Player matrix pins
const int KeysP1[] = {15,14,13,12,11,10,9};
const int KeysP2[] = {7,6,5,4,3,2,1};

// Player scores
int scoreP1 = 0;
int scoreP2 = 0;

// patterns
#define PATTERN_LENGTH 100
int pattern[PATTERN_LENGTH];

#define PATTERN_INTERVAL 500

bool p1Pattern;
bool p2Pattern;
bool p1PStart = true;
bool p2PStart = true;

// player pattern indexes
int indexP1P = 0;
int indexP2P = 0;
unsigned long prevTimeP1P = 0;
unsigned long prevTimeP2P = 0;

// player pattern variables

int p1Cur = 0;
int p2Cur = 0;
int p1Max = 1;
int p2Max = 1;


// put function declarations here:

void updateKeys();
void updateScreens();
void showPatterns(unsigned long);


void setup() { 
    p1Pattern = true;
    p2Pattern = true;

    randomSeed(analogRead(0)); //randomise seed to avoid every time being the same after reset
    
    // Set up the pattern
    for (int i = 0; i < PATTERN_LENGTH; i++){
        pattern[i] = random(0, 6);
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
        
    status1 = lcd1.begin(LCD_COLS, LCD_ROWS);
    status2 = lcd2.begin(LCD_COLS, LCD_ROWS);
    if(status1) hd44780::fatalError(status1);
    if(status2) hd44780::fatalError(status2);

    // initalization was successful, the backlight should be on now
    // Print a message to the LCD
    lcd1.print("Valopeli, P1");
    lcd2.print("Valopeli, P2");
    delay(2000);
    lcd1.clear(); lcd1.print("3");
    lcd2.clear(); lcd2.print("3");
    delay(700);
    lcd1.clear(); lcd1.print("2");
    lcd2.clear(); lcd2.print("2");
    delay(700);
    lcd1.clear(); lcd1.print("1");
    lcd2.clear(); lcd2.print("1");
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

    if(!p1Pattern && keys[KeysP1[pattern[p1Cur]]] == HIGH){
        p1Cur++;
        if(p1Cur == p1Max){
            scoreP1++;
            p1Cur = 0;
            p1Max++;
            if(p1Max == PATTERN_LENGTH) p1Max = 1;
            p1Pattern = true;
            updateScreens();
        }
    }
    if(!p2Pattern && keys[KeysP2[pattern[p2Cur]]] == HIGH){
        p2Cur++;
        if(p2Cur == p2Max){
            scoreP2++;
            p2Cur = 0;
            p2Max++;
            if(p2Max == PATTERN_LENGTH) p2Max = 1;
            p2Pattern = true;
            updateScreens();
        }
    }
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
  lcd1.clear();
  lcd2.clear();
  lcd1.print("Player 1: " + String(scoreP1));
  lcd2.print("Player 2: " + String(scoreP2));
}

// Returns random free pin between min and max, that's not current
int randomPin(int current, int min, int max){
    int i = random(min, max);
    while(i == current) i = random(min, max);
    return i;
}

// TODO - ratkaise ensimmäisen indeksin ongelma :D
void showPatterns(unsigned long currentTime){
    // Check if in pattern phase and interval has passed
    bool p1Next = ((unsigned long)(currentTime - prevTimeP1P) >= PATTERN_INTERVAL);
    bool p2Next = ((unsigned long)(currentTime - prevTimeP2P) >= PATTERN_INTERVAL);

    // Check p1
    if(p1Pattern && p1Next){
        Serial.print(pattern[indexP1P]);
        if(p1PStart){
            p1PStart = false;
            leds[LedP1[pattern[indexP1P]]] = CRGB::Red;
        }
        else{
            // Set the previous LED in the pattern to off
            leds[LedP1[pattern[indexP1P]]] = CRGB::Black;
            indexP1P++;
            // Check if at the max index
            if(indexP1P == p1Max){
                p1Pattern = false; // End of pattern
                p1PStart = true; // Start flag for next iteration
                indexP1P = 0;
                Serial.println();
            }    
            else leds[LedP1[pattern[indexP1P]]] = CRGB::Red; // Next in pattern
        }
        prevTimeP1P = currentTime; // Time check
        FastLED.show();
    }

    if(p2Pattern && p2Next){
        Serial.print(pattern[indexP2P]);
        if(p2PStart){
            p2PStart = false;
            leds[LedP2[pattern[indexP2P]]] = CRGB::Red;
        }
        else{
            leds[LedP2[pattern[indexP2P++]]] = CRGB::Black;
            if(indexP2P == p2Max){
                p2Pattern = false;
                p2PStart = true;
                indexP2P = 0;
                Serial.println();
            }
            else leds[LedP2[pattern[indexP2P]]] = CRGB::Red;
        }
        prevTimeP2P = currentTime;
        FastLED.show();
    }
}