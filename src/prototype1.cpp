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

//game variables
int currentP1 = -1;
int lastP1 = -1;
int currentP2 = -1;
int lastP2 = -1;

int scoreP1 = 0;
int scoreP2 = 0;

// put function declarations here:
void updateKeys();
void updateScreens();

void setup() {
  randomSeed(analogRead(0)); //randomise seed to avoid every time being the same after reset
  Serial.begin(9600); //enable serial for debugging
  //pin setup for button matrix:
  for(int i=0; i < size; i++){
    pinMode(row[i], OUTPUT);
    pinMode(col[i], INPUT);
  }
  //led strip initialization
  FastLED.addLeds<LED_TYPE, LED_PIN>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

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
    delay(1000);
    lcd1.clear();
    lcd2.clear();
    lcd1.print("P1: 0");
	lcd2.print("P2: 0");
}

void loop() {
  // put your main code here, to run repeatedly:
  //check if player 1 should get a new led
  if(currentP1 == -1){
    while(1){
      int i = random(1,7); //randomise until new led is different to previous led
      if(i != lastP1){
        currentP1 = i;
        lastP1 = i;
        break;
      }
    }
  }
  //do the same for player 2
  if(currentP2 == -1){
    while(1){
      int i = random(1,7);
      if(i != lastP2){
        currentP2 = i;
        lastP2 = i;
        break;
      }
    }
  }
  updateKeys(); //read the state of the button matrix
  leds[LedP1[currentP1-1]] = CRGB(255,255,255); //turn on the current leds for player 1 and 2
  leds[LedP2[currentP2-1]] = CRGB(255,255,255);
  FastLED.show();

  //check if player 1 is pressing the correct button, give point and set to randomise new led
  if(keys[KeysP1[currentP1]] == HIGH){
    //check if player 1 is also pressing an incorrect key, in that case, don't give points
    int pass1 = 1;
    for(int i = 1; i<7; i++){
      if(i!=currentP1 && keys[KeysP1[i]] == HIGH) pass1 = 0;
    }
    if(pass1 == 1){
      leds[LedP1[currentP1-1]] = CRGB::Black;
      currentP1 = -1;
      scoreP1++;
      updateScreens();
    }
  }
  //do the same for player 2
  if(keys[KeysP2[currentP2]] == HIGH){
    int pass2 = 1;
    for(int i = 1; i<7; i++){
      if(i!=currentP2 && keys[KeysP2[i]] == HIGH) pass2 = 0;
    }
    if(pass2 == 1){
      leds[LedP2[currentP2-1]] = CRGB::Black;
      currentP2 = -1;
      scoreP2++;
      updateScreens();
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
  lcd1.setCursor(0,0);
  lcd2.clear();
  lcd2.setCursor(0,0);
  lcd1.print("P1: " + String(scoreP1));
  lcd2.print("P2: " + String(scoreP2));
}