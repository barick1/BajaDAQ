#include <TimeLib.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_MAX31856.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(45, 44, 25, 24, 23, 22);

// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31856 max = Adafruit_MAX31856(7, 8, 9, 10); //initializes Thermocouple pins

const int chipSelect = 4;

int switchState = 0; //initialize switch value
int newFileSwitch = 6; //pin the switch is connected to
int LED = 2; //pin the LED is in
int incrFileName = 0; //adds 1 to the SD CARD file name
int rpmUpdates = 0; //The number of times that rpm was updated on the screen

String fileName = "DataLog"; //name of the SD card txt file
String currentName;

int pin=20;
int magnet;
float rpm;
long startTime, endTime, totalTime;
volatile float revs;

void setup() {
//Initialize Pins
  pinMode(pin,INPUT);
  pinMode(LED,OUTPUT); // LED
  pinMode(newFileSwitch,INPUT); // Switch state

  lcd.begin(16, 2);// set up the LCD's number of columns and rows:
  
  Serial.begin(9600);//Setup the Serial Communication

//THERMOCOUPLE
  max.begin();
  max.setThermocoupleType(MAX31856_TCTYPE_K);
  Serial.print("Thermocouple type: ");
  switch ( max.getThermocoupleType() ) {
    case MAX31856_TCTYPE_B: lcd.print("B Type"); break;
    case MAX31856_TCTYPE_E: Serial.println("E Type"); break;
    case MAX31856_TCTYPE_J: Serial.println("J Type"); break;
    case MAX31856_TCTYPE_K: lcd.print("K Type"); break;
    case MAX31856_TCTYPE_N: Serial.println("N Type"); break;
    case MAX31856_TCTYPE_R: Serial.println("R Type"); break;
    case MAX31856_TCTYPE_S: Serial.println("S Type"); break;
    case MAX31856_TCTYPE_T: Serial.println("T Type"); break;
    case MAX31856_VMODE_G8: Serial.println("Voltage x8 Gain mode"); break;
    case MAX31856_VMODE_G32: Serial.println("Voltage x8 Gain mode"); break;
    default: Serial.println("Unknown"); break;}
  
//SD CARD
  Serial.print("Initializing SD card.....");
  if(!SD.begin(chipSelect)){
    Serial.println("Card failed, or not present");
  return;}
  Serial.println(" card initialized.");
  
//RPM SENSOR
  attachInterrupt(1, rev, RISING);
  startTime = millis();
  
  lcd.print(", SD Card Initialized");
  lcd.setCursor(0,1);
  lcd.print("Get Dirty MotherFucker");
  delay(1500);
  for (int positionCounter = 0; positionCounter < 13; positionCounter++) {
    // scroll one position left:
    lcd.scrollDisplayLeft();
    // wait a bit:
    delay(500);
  }
  for (int positionCounter = 0; positionCounter < 13; positionCounter++) {
  // scroll one position left:
  lcd.scrollDisplayRight();
  // wait a bit:
  delay(500);
}

  delay(5000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Systems ARE A GO");
  delay(3000);
  lcd.clear();
}



void loop() {
//  switchState = digitalRead(newFileSwitch);
//  while (switchState == HIGH){
//    incrFileName = incrFileName + 1;
//    digitalWrite(LED,HIGH); //turns LED on
//    if (switchState == LOW){
//      break;
//    }
//  }

  //Serial.println(revs);
  
  if (revs>10){//Calculates RPM
  totalTime= millis()- startTime;
  rpm = 60000.00*(revs/totalTime);
  lcd.setCursor(0,0);
  lcd.print("RPM: ");
  Serial.println(rpm);
  lcd.print(rpm);
  rpmUpdates++;
  startTime = millis();
  revs = 0;
  }

  if (rpmUpdates > 1){//Calls function to log temperature value to SD CARD
    TempLog();
    rpmUpdates = 0;
  }
  digitalWrite(LED,LOW);
}

//Counts the amount of revolutions using the interrupt function declared in the setup() loop
void rev(){
  revs++;
}

String NewName(){
  currentName = "DataLog" + String(incrFileName);
  return currentName;
}

//Logs the current temperature to the SD card
void TempLog(){
  Serial.print("FileName: ");
  Serial.println(incrFileName);

  fileName = "DataLog" + String(incrFileName); //add current count to the filename
  
  float temperature = max.readThermocoupleTemperature(); //Saves temperature to variable
  uint8_t fault = max.readFault();
  if (fault) {
    if (fault & MAX31856_FAULT_CJRANGE) Serial.println("Cold Junction Range Fault");
    if (fault & MAX31856_FAULT_TCRANGE) Serial.println("Thermocouple Range Fault");
    if (fault & MAX31856_FAULT_CJHIGH)  Serial.println("Cold Junction High Fault");
    if (fault & MAX31856_FAULT_CJLOW)   Serial.println("Cold Junction Low Fault");
    if (fault & MAX31856_FAULT_TCHIGH)  Serial.println("Thermocouple High Fault");
    if (fault & MAX31856_FAULT_TCLOW)   Serial.println("Thermocouple Low Fault");
    if (fault & MAX31856_FAULT_OVUV)    Serial.println("Over/Under Voltage Fault");
    if (fault & MAX31856_FAULT_OPEN)    Serial.println("Thermocouple Open Fault");
  }
  
  String dataString = ""; //make a string for assembling the data log

  dataString += String(millis());
  dataString += ",";
  dataString += temperature; //concatenate to SD string

  lcd.setCursor(0,1);
  lcd.print(String(temperature) + " C");

//Open the file. note that only one file can be open at a time,so you have to close this one before opening another.
  File dataFile = SD.open(fileName + ".txt", FILE_WRITE);

//If the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
//If the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }

  digitalWrite(LED,LOW); //turns LED off
}


