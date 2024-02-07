#include <Arduino.h>

const int row[] = {11, 12, 13};
const int col[] = {2,3,4};
const int size = 3;
int keys[size*size];

// put function declarations here:
void updateKeys();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  for(int i=0; i < size; i++){
    pinMode(row[i], OUTPUT);
    pinMode(col[i], INPUT);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  updateKeys();
  for(int i=0; i < size; i++){
    for(int j = 0; j < size; j++){
        Serial.print(keys[size*i+j]);
    }
    Serial.print(" ");
  }
  Serial.println();
  delay(200);

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