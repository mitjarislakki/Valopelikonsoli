#include <Arduino.h>
#include <FastLED.h>
#include <stdlib.h>
#include <Wire.h>
#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header
#include <LinkedList.h>

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
//matriisin asetukset:
const int row[] = {8,9,10,11};
const int col[] = {2,3,4,5};
const int size = 4;
int keys[size*size];

const int numLeds = 31;

//pelaajien asetukset:
const int LedP1[] = {0,1,2,3,4,5,6};
const int LedP2[] = {7,8,9,10,11,12,13};

const int KeysP1[] = {15,14,13,12,11,10,9};
const int KeysP2[] = {7,6,5,4,3,2,1};

int scoreP1 = 0;
int scoreP2 = 0;

// patterns
#define PATTERN_LENGTH 100
int pattern[PATTERN_LENGTH];
int p1Cur = 0;
int p2Cur = 0;
int p1Max = 1;
int p2Max = 1;
bool p1Pattern = true;
bool p2Pattern = true;


// put function declarations here:
void updateKeys();
void updateScreens();
void showPattern(int);


void setup() { 
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
    if(p1Pattern){
        showPattern(p1Max);
        p1Pattern = false;
    }
    else{
        updateKeys();
        if(keys[KeysP1[pattern[p1Cur]]] == HIGH){
            p1Cur++;
            if(p1Cur == p1Max){
                p1Cur = 0;
                p1Max++;
                p1Pattern = true;
            }
        }
    }
    delay(10);
}

// put function definitions here:
//reads the state of the matrix and puts it into keys[]
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
  //lcd1.setCursor(0,0);
  lcd2.clear();
  //lcd2.setCursor(0,0);
  lcd1.print("Player 1: " + String(scoreP1));
  lcd2.print("Player 2: " + String(scoreP2));
}

int randomPin(int current, int min, int max){
    int i = random(min, max);
    while(i == current) i = random(min, max);
    return i;
}

void showPattern(int until){
    for(int i = 0; i < until; i++){
        leds[LedP1[pattern[i]]] = CRGB::Red;
        FastLED.show();
        delay(500);
        leds[LedP1[pattern[i]]] = CRGB::Black;
        FastLED.show();
    }
}
