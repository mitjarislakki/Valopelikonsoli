#include <Arduino.h>

const int row[] = {11, 12, 13};
const int col[] = {2,3,4};
int keys[9];

// put function declarations here:
void updateKeys();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  for(int i=0; i < 3; i++){
    pinMode(row[i], OUTPUT);
    pinMode(col[i], INPUT);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  updateKeys();
  for(int i=0; i < 3; i++){
    Serial.print(keys[3*i]);
    Serial.print(keys[3*i+1]);
    Serial.print(keys[3*i+2]);
    Serial.print(" ");
  }
  Serial.println();
  delay(200);

}

// put function definitions here:
void updateKeys(){
    for(int i=0; i < 3; i++){
        digitalWrite(row[0], LOW);
        digitalWrite(row[1], LOW);
        digitalWrite(row[2], LOW);

        digitalWrite(row[i], HIGH);
        for(int j = 0; j < 3; j++){
            int state = digitalRead(col[j]);
            if(state == HIGH){
                keys[i*3+j] = HIGH;
            }
            else{
                keys[i*3+j] = LOW;
            }
        }
        digitalWrite(row[i], LOW);
    }

  
}