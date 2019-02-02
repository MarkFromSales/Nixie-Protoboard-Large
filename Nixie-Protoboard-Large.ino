/* 
https://github.com/MarkFromSales/Nixie-Protoboard-Large
More information in the README.MD
*/

// Setup MCP GPIO expanders
#include "Adafruit_MCP23017.h"
Adafruit_MCP23017 mcp1;
Adafruit_MCP23017 mcp2;
#define addr1 0  // 0 = A2 low , A1 low , A0 low
#define addr2 7  // 7 = A2 high, A1 high, A0 high

// Setup Tube Variables
int nxeBL = 2;   // Nixie tube backlight control pin
int pinSW = 3;   // Enable switch GPIO 3, pin 5
int valSW = 0;   // Variable for Enable switch pin value
int pinCT = 16;  // How many pins in the MCPs to enable +1
int psuSW = 5;   // Shutdown for HVDC PSU GPIO 5, pin 11

// Function to set all the MCP GPIO pins to HIGH, shutting off the Nixies
void NixiesOff (){
  for (int thisPin = 0; thisPin < pinCT; thisPin++) {
      mcp1.digitalWrite(thisPin, HIGH);
      mcp2.digitalWrite(thisPin, HIGH);
  }
};

// Setup LCD Display
#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);  // Initialize the library with the numbers of the interface pins
int pinBS = A3; // Analog  pin 3 (D17) to wiper of pot
int pinBL = 6;  // Digital pin 6 (PWM) to LCD backlight VCC
int valBL = 0;  // Variable to store the read value

// Setup Adafruit INA219 current sensor
#include <Adafruit_INA219.h>
Adafruit_INA219 ina219_A;
int power_mW;

  
// Tube state machine
int tubeMatrix[9][4] = {
  { 0, 0, 0, 0 },      // Row 0: Not used, loops will start with 1
  { 0, 1, 2, 3 },      // Row 1: Tube 1
  { 4, 5, 6, 7 },      // Row 2: Tube 2
  { 8, 9, 10, 11 },    // Row 3: Tube 3
  { 12, 13, 14, 15 },  // Row 4: Tube 4
  { 0, 1, 2, 3 },      // Row 5: Tube 5
  { 4, 5, 6, 7 },      // Row 6: Tube 6
  { 8, 9, 10, 11 },    // Row 7: Tube 7
  { 12, 13, 14, 15 }   // Row 8: Tube 8
};

class Rotator {
  // Class member values
  int ledPin;     // LED state pin
  int tubeID;     // Sets which tube to update
  long OnTime;    // milliseconds of on-time
  long OffTime;   // milliseconds of off-time
  int tubeVal;    // Sets the value of the tube
  int ledState;   // Maintains the current state
  unsigned long previousMillis;   // will store last time LED was updated

  public:
  Rotator(int tube, long on, long off, int value) {
    ledPin = 13;
    pinMode(ledPin, OUTPUT);
    tubeID = tube;
    OnTime = on;
    OffTime = off;
    tubeVal = value;
    ledState = LOW;
    previousMillis = 0;
  }

  void Update(){
    // Check to see if it's time to change the state of the LED
    unsigned long currentMillis = millis();

    if((ledState == HIGH) && (currentMillis - previousMillis >= OnTime)) {
      ledState = LOW;                  // Turn LED off
      NixiesOff();                     // Turn nixie tubes off
      previousMillis = currentMillis;  // Remember the time
      digitalWrite(ledPin, ledState);  // Update the actual LED
      Serial.print("Nixies_OFF  TubeID: ");
      Serial.print(tubeID);
      Serial.print("  PreviousMillis: ");
      Serial.print(previousMillis);
      Serial.print("  CurrentMillis: ");
      Serial.println(currentMillis);
    }

    else if ((ledState == LOW) && (currentMillis - previousMillis >= OffTime)) {
      ledState = HIGH;                  // Turn LED on
      previousMillis = currentMillis;   // Remember the time
      digitalWrite(ledPin, ledState);    // Update the actual LED
      Serial.print("Nixies_ON   TubeID: ");
      Serial.print(tubeID);
      Serial.print("  PreviousMillis: ");
      Serial.print(previousMillis);
      Serial.print("  CurrentMillis: ");
      Serial.println(currentMillis);

      // Setting the value of the tube, 1st set of 4 tubes on MCP1
      if (tubeID > 0 && tubeID < 5) {
        mcp1.digitalWrite(tubeMatrix[tubeID][3], (tubeVal & 0x08) >> 3);
        mcp1.digitalWrite(tubeMatrix[tubeID][2], (tubeVal & 0x04) >> 2);
        mcp1.digitalWrite(tubeMatrix[tubeID][1], (tubeVal & 0x02) >> 1);
        mcp1.digitalWrite(tubeMatrix[tubeID][0], tubeVal & 0x01);
      }

      // Setting the value of the tube, 2nd set of 4 tubes on MCP2
      else {
        mcp2.digitalWrite(tubeMatrix[tubeID][3], (tubeVal & 0x08) >> 3);
        mcp2.digitalWrite(tubeMatrix[tubeID][2], (tubeVal & 0x04) >> 2);
        mcp2.digitalWrite(tubeMatrix[tubeID][1], (tubeVal & 0x02) >> 1);
        mcp2.digitalWrite(tubeMatrix[tubeID][0], tubeVal & 0x01);
      }
    }
  }
};

int timeOn = 5000;   // Tube On duration X
int timeOff = 5000;  // Tube Off duration Y

Rotator tube1(1, timeOn, timeOff, 1);  // Tube #, on for X, off for Y, display number
Rotator tube2(2, timeOn, timeOff, 2);
Rotator tube3(3, timeOn, timeOff, 3);
Rotator tube4(4, timeOn, timeOff, 4);
Rotator tube5(5, timeOn, timeOff, 5);
Rotator tube6(6, timeOn, timeOff, 6);
Rotator tube7(7, timeOn, timeOff, 7);
Rotator tube8(8, timeOn, timeOff, 8);

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println("Setup Start");
  mcp1.begin(addr1);
  mcp2.begin(addr2);

  // Set MCP Nixie Tube pins to OUTPUT
  for (int thisPin = 0; thisPin < pinCT; thisPin++) {
    mcp1.pinMode(thisPin, OUTPUT);
    mcp2.pinMode(thisPin, OUTPUT);
  }

  pinMode(nxeBL, OUTPUT);
  pinMode(pinSW, INPUT);
  pinMode(psuSW, OUTPUT);

  // set up the LCD's number of rows and columns: 
  pinMode(pinBS, INPUT);
  pinMode(pinBL, OUTPUT);
  lcd.begin(16, 2);

  // Print a message to the LCD as an initialization test
  lcd.setCursor(0,0);
  lcd.print("LCD 16x2 Display");
  lcd.setCursor(0,1);
  lcd.print("Mono Color LCD  ");
  lcd.clear();
  
  // Adafruit INA219 Current Sensor
  ina219_A.begin();  // Initialize first board (default address 0x40)
  uint32_t currentFrequency;
  
  Serial.println("Setup End");

}

void loop() {
  valBL = analogRead(pinBS);       // Read the wiper from the LCD brightness pot
  analogWrite(pinBL, valBL / 4);   // Turn on LCD backlight
  valSW = digitalRead(pinSW);      // Check the state of the Nixie enable switch
  
  // Adafruit INA219
  //shuntvoltage = ina219_A.getShuntVoltage_mV();
  //busvoltage = ina219_A.getBusVoltage_V();
  //current_mA = ina219_A.getCurrent_mA();
  power_mW = ina219_A.getPower_mW();
  //loadvoltage = busvoltage + (shuntvoltage / 1000);
  
  // Check to see if user switch is on (otherwise Else)
  if (valSW == 1) {
    lcd.setCursor(0,0);
    lcd.print("Sw ON           ");
    lcd.setCursor(9,0);
    lcd.print(millis() / 1000);
    lcd.setCursor(0,1);
    //lcd.print("Nixies Flashing?");
    lcd.print(power_mW / 1000);
    lcd.setCursor(7,1);
    lcd.print("W");
    
    digitalWrite(nxeBL, HIGH);     // Turn on tube backlight LED
    analogWrite(psuSW, 160);       // Turn on tube power supply (relay) 0-255, 165 is about 3.3V
    
    tube1.Update();
    tube2.Update();
    tube3.Update();
    tube4.Update();
    tube5.Update();
    tube6.Update();
    tube7.Update();
    tube8.Update();
  }
  else {
    lcd.setCursor(0,0);
    lcd.print("Sw OFF          ");
    lcd.setCursor(9,0);
    lcd.print(millis() / 1000);
    lcd.setCursor(0,1);
    //lcd.print("Nixies Off?     ");
    lcd.print(power_mW / 1000);
    lcd.setCursor(7,1);
    lcd.print("W");
    
    digitalWrite(nxeBL, LOW);     // Turn off tube backlight LED
    analogWrite(psuSW, 0);        // Turn off tube power supply (relay)

    NixiesOff();
  }
}

//EOF
