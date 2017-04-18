#include <LiquidCrystal.h>
#include <SD.h>

LiquidCrystal lcd(12, 11, 6, 5, 4, 3);

volatile float revs;
long startTime, endTime, totalTime;
float rpm;

void setup() {
  // set up the LCD's number of columns and rows:
    startTime = millis();
  Serial.begin(9600);//Setup the Serial Communication
      attachInterrupt(digitalPinToInterrupt(2), rev, RISING);
}

void loop() {
  Serial.println(revs);
    
    if (revs>10){//Calculates RPM
        totalTime= millis()- startTime;
        rpm = 60000.00*(revs/totalTime);
        //Serial.println(rpm);
        startTime = millis();
        revs = 0;
    }
}

void rev(){
    revs++;
}
