//Sample using LiquidCrystal library
#include <LiquidCrystal.h>
#include <Buttons.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2

const int DEBOUNCE_MILLIS = 50;
const int REPEAT_INITIAL_DELAY_MILLIS = 500;
const int REPEAT_DELAY_MILLIS = 100;
const byte NBR_OF_BUTTONS = 5;                   // Number of buttons
const int READING_INTERVAL_MILLIS = 5000;
const byte RELAY_PIN = 3;
const float TEMP_SPREAD = 0.05;

Button* button[] = { new AnalogButton(0, 0, 50, DEBOUNCE_MILLIS, REPEAT_INITIAL_DELAY_MILLIS, REPEAT_DELAY_MILLIS, &handleClickRight, &handleRelease),
                     new AnalogButton(0, 50, 195, DEBOUNCE_MILLIS, REPEAT_INITIAL_DELAY_MILLIS, REPEAT_DELAY_MILLIS, &handleClickUp, &handleRelease),
                     new AnalogButton(0, 195, 380, DEBOUNCE_MILLIS, REPEAT_INITIAL_DELAY_MILLIS, REPEAT_DELAY_MILLIS, &handleClickDown, &handleRelease),
                     new AnalogButton(0, 380, 555, DEBOUNCE_MILLIS, REPEAT_INITIAL_DELAY_MILLIS, REPEAT_DELAY_MILLIS, &handleClickLeft, &handleRelease),
                     new AnalogButton(0, 555, 790, DEBOUNCE_MILLIS, REPEAT_INITIAL_DELAY_MILLIS, REPEAT_DELAY_MILLIS, &handleClickSelect, &handleRelease)
};

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress insideThermometer = { 0x28, 0xD0, 0x19, 0x15, 0x06, 0x00, 0x00, 0x3D };

// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

float currentTemp = 0;
int presetTempTenths = 240;
long lastReadingTime = 0;
boolean tempDisplayed = false;

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  sensors.begin();
  if (!sensors.getAddress(insideThermometer, 0)) {
    lcd.print("Unable to find address for Device 0"); 
  } else {
    lcd.print("Found therm");
    lcd.setCursor(0,1);
    printAddress(insideThermometer);
  }
  // set the resolution to 12 bit
  sensors.setResolution(insideThermometer, 12);
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Current  Set"));
  displayPreset();
}

void loop() {
  for (int i = 0; i < NBR_OF_BUTTONS; i++) {   // Read the state of all buttons
    button[i]->read();
  }
  if (millis() > lastReadingTime + READING_INTERVAL_MILLIS) {
    readTemp();
    lastReadingTime = millis();
    tempDisplayed = false;
  }
  if (!tempDisplayed && millis() > lastReadingTime + 100) {
    getTemp();
    displayTemp();
    checkTemp();
    tempDisplayed = true;
  }
}

void readTemp() {
  sensors.setWaitForConversion(false);  // makes it async
  sensors.requestTemperatures(); // Send the command to get temperatures
  sensors.setWaitForConversion(true);  // makes it async
}

void getTemp() {
  currentTemp = sensors.getTempC(insideThermometer);
}

void displayPreset() {
  lcd.setCursor(9, 1);
  lcd.print(presetTempTenths / 10);
  lcd.print(F("."));
  lcd.print(presetTempTenths % 10);
  lcd.write(B11011111);
  lcd.print(F(" "));
}

void displayTemp() {
    lcd.setCursor(0, 1);
    lcd.print((int) currentTemp);
    lcd.print(F("."));
    lcd.print((int) (((currentTemp + 0.05) * 10)) % 10);
    lcd.write(B11011111);
}

void checkTemp() {
  float temp = presetTempTenths / 10.0;
  float lowTemp = temp - TEMP_SPREAD;
  float highTemp = temp + TEMP_SPREAD;
  
  if (currentTemp <  lowTemp) {
    digitalWrite(RELAY_PIN, HIGH);
  } else if (currentTemp >= lowTemp && currentTemp <= highTemp) {
    // Do nothing, we're in the butter zone
  } else {
    digitalWrite(RELAY_PIN, LOW);
  }
}

void handleClickRight(Button* button) {
  // Do nothing
}

void handleClickLeft(Button* button) {
  // Do nothing
}

void handleClickUp(Button* button) {
  presetTempTenths++;
  displayPreset();
}

void handleClickDown(Button* button) {
  presetTempTenths--;
  displayPreset();
}

void handleClickSelect(Button* button) {
  // Do nothing
}

void handleRelease(Button* button) {
  // Do nothing
}

void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) lcd.print("0");
    lcd.print(deviceAddress[i], HEX);
  }
}

