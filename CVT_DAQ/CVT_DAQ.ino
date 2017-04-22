//#include <TimeLib.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_MAX31856.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(A5, A4, A3, A2, 5, 3); //CHANGED LCD PINS

// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31856 max = Adafruit_MAX31856(7, 8, 9, 10); //initializes Thermocouple pins

const int chipSelect = 4;

//int switchState = 0; //initialize switch value
//int newFileSwitch = 6; //pin the switch is connected to
//int LED = 2; //pin the LED is in
int incrFileName = 0; //adds 1 to the SD CARD file name
int rpmUpdates = 0; //The number of times that rpm was updated on the screen

String fileName = "datalog.txt"; //name of the SD card txt file
//String currentName;
String testName = "test2.txt";
int pin=20;
int magnet;
float rpm;
long startTime, endTime, totalTime;
volatile float revs;

void setup() {
    //Initialize Pins
    pinMode(pin,INPUT);
    //pinMode(LED,OUTPUT); // LED
   //pinMode(newFileSwitch,INPUT); // Switch state
    lcd.begin(16, 2);// set up the LCD's number of columns and rows:
    Serial.begin(9600);//Setup the Serial Communication
    
    //THERMOCOUPLE
    max.begin();
    max.setThermocoupleType(MAX31856_TCTYPE_K);
    Serial.print("Thermocouple type: ");
    switch (max.getThermocoupleType() ) {
        case MAX31856_TCTYPE_B: Serial.println("B Type"); break;
        case MAX31856_TCTYPE_E: Serial.println("E Type"); break;
        case MAX31856_TCTYPE_J: Serial.println("J Type"); break;
        case MAX31856_TCTYPE_K: Serial.println("K Type"); break;
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
        return;
    }
    Serial.println("Card initialized.");
    
    //update
    String file_name = "version.txt";
    File _version = SD.open(file_name, FILE_READ);
    if (_version) {
      Serial.println("Version file opened successfully");
    }
    incrFileName = _version.read();
    incrFileName++;
    _version.close();
    SD.remove(file_name);
    File Update = SD.open("version.txt", FILE_WRITE);
    char up_date = incrFileName;
    Update.write(up_date);
    Update.close();

    

    //RPM SENSOR
    attachInterrupt(digitalPinToInterrupt(2), rev, RISING);
    startTime = millis();

    
    lcd.print("Get Dirty");
    lcd.setCursor(0,1);
    lcd.print("MotherFucker");
    delay(5000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Systems ARE A GO");
    lcd.setCursor(0,1);
    lcd.print("Version " + String(incrFileName));
    delay(1500);
    lcd.clear();
    Serial.println("Reached end of setup()");
    endTime = millis();
}


void loop() {
    //update
    /*
    switchState = digitalRead(newFileSwitch);
    if (switchState == HIGH){
        incrFileName = incrFileName + 1;
        SD.remove("version.txt");
        File Update = SD.open("version.txt", FILE_WRITE);
        char up_date = incrFileName;
        Update.write(up_date);
        Update.close();

    }
    */
    //Serial.println(revs);

    //Calculates RPM
    if (revs>10 /*millis() % 1000 < 10*/){
        lcd.clear();
        totalTime= millis()- startTime;
        rpm = 60000.00*(revs/totalTime);
        lcd.setCursor(0,0);
        lcd.print("RPM: ");
        //Serial.println(rpm);
        lcd.print(rpm);
        rpmUpdates++;
        startTime = millis();
        revs = 0;

        // UPDATING ELAPSED TIME
        lcd.setCursor(9,1);
        long totalSec = (millis()-endTime) / 1000;
        int hour = totalSec / 3600;
        int minute = (totalSec % 3600) / 60;
        int second = (totalSec % 3600) % 60;
        lcd.print(String(hour) + "h" + String(minute) + "m" + String(second) + "s");
    }
    
    if (rpmUpdates > 1){//Calls function to log temperature value to SD CARD
        TempLog();
        rpmUpdates = 0;
    }
    //digitalWrite(LED,LOW);
}

//Counts the amount of revolutions using the interrupt function declared in the setup() loop
void rev(){
    revs++;
}

void updateFileName(){
    String tmp = "DataLog" + String(incrFileName) + ".txt";
    fileName = tmp.c_str();
}

//Logs the current temperature to the SD card
void TempLog(){
    Serial.print("FileName: ");
    //Serial.println(incrFileName);
    
    //String fileName = NewName(); //add current count to the filename
    
    float temperature = max.readThermocoupleTemperature(); //Saves temperature to variable
    uint8_t fault = max.readFault();
    if (fault) {
        if (MAX31856_FAULT_CJRANGE) Serial.println("Cold Junction Range Fault");
        if (MAX31856_FAULT_TCRANGE) Serial.println("Thermocouple Range Fault");
        if (MAX31856_FAULT_CJHIGH)  Serial.println("Cold Junction High Fault");
        if (MAX31856_FAULT_CJLOW)   Serial.println("Cold Junction Low Fault");
        if (MAX31856_FAULT_TCHIGH)  Serial.println("Thermocouple High Fault");
        if (MAX31856_FAULT_TCLOW)   Serial.println("Thermocouple Low Fault");
        if (MAX31856_FAULT_OVUV)    Serial.println("Over/Under Voltage Fault");
        if (MAX31856_FAULT_OPEN)    Serial.println("Thermocouple Open Fault");
    }
    
    String dataString; //make a string for assembling the data log
    
    dataString += String(millis()-endTime);
    dataString += ",";
    dataString += temperature; //concatenate to SD string
    dataString += "," + String(rpm);
    
    lcd.setCursor(0,1);
    lcd.print(String(temperature) + " C");
    /*
    //Open the file. note that only one file can be open at a time,so you have to close this one before opening another.
    fileName = String(incrFileName) + ".txt";
    char * temp = fileName.c_str();
    //updateFileName();
    File dataFile = SD.open("test.txt", FILE_WRITE);
    dataFile.println("TEST\n");
    //If the file is available, write to it:
    if (dataFile) {
        dataFile.println(dataString);
        Serial.println("Data Written to File");
        dataFile.close();
        // print to the serial port too:
        //Serial.println(dataString);
    }
    //If the file isn't open, pop up an error:
    else {
        Serial.println("error opening datalog.txt, inrcFileName=" + String(incrFileName));
        Serial.println("filename = " + String(fileName));
    }
    
    //digitalWrite(LED,LOW); //turns LED off
}*/
  String test = String(incrFileName) + ".txt";
  File dataFile = SD.open(test, FILE_WRITE);
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
}
