#include <Arduino.h>
#include <FastLED.h>
#include <stdlib.h>
#include <Wire.h>
#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header

//Näytöjen asetukset:
hd44780_I2Cexp lcd1(0x20);
hd44780_I2Cexp lcd2(0x27);
const int LCD_COLS = 16;
const int LCD_ROWS = 2;

//Ledien asetukset
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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  for(int i=0; i < size; i++){
    pinMode(row[i], OUTPUT);
    pinMode(col[i], INPUT);
  }
  //led strip initialization
  FastLED.addLeds<LED_TYPE, LED_PIN>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  //LCD initialization
    int status1;
    int status2;
    /*
    status1 = lcd1.begin(LCD_COLS, LCD_ROWS);
	status2 = lcd2.begin(LCD_COLS, LCD_ROWS);
	if(status1) hd44780::fatalError(status1);
	if(status2) hd44780::fatalError(status2);

	// initalization was successful, the backlight should be on now
	// Print a message to the LCD
	lcd1.print("Hello,");
	lcd2.print("World");
    */
}

void loop() {
  // put your main code here, to run repeatedly:
  if(currentP1 == -1){
    while(1){
      int i = random(1,7);
      if(i != lastP1){
        currentP1 = i;
        lastP1 = i;
        break;
      }
    }
  }
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
  updateKeys();
  leds[LedP1[currentP1-1]] = CRGB(255,255,255);
  leds[LedP2[currentP2-1]] = CRGB(255,255,255);
  FastLED.show();
  if(keys[KeysP1[currentP1]] == HIGH){
    leds[LedP1[currentP1-1]] = CRGB::Black;
    currentP1 = -1;
  }
  if(keys[KeysP2[currentP2]] == HIGH){
    leds[LedP2[currentP2-1]] = CRGB::Black;
    currentP2 = -1;
  }
  /*
  for(int i=0; i < size*size; i++){
    if(keys[i]==HIGH){
      leds[i] = CRGB(255,255,255);
    }
    else{
      leds[i] = CRGB::Black;
    }
  }
  */
  

  delay(10);

}

// put function definitions here:
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