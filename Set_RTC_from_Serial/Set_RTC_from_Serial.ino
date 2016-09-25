// Set the Feather DS3231 Precision RTC Wing via Serial Monitor
// This code is written for the Adalogger M0 Feather
     
// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
volatile boolean RTCflag = false; // RTC interrupt flag

void setup()
{
  // initialize digital pins 13 and 8 as outputs.
  pinMode(13, OUTPUT); // Red LED
  pinMode(8, OUTPUT); // Green LED
  // flash red and green LEDs on reset
  for (int i=0; i <= 4; i++) {
    digitalWrite(13, HIGH);
    delay(200);
    digitalWrite(13, LOW);
    digitalWrite(8, HIGH);
    delay(200);
    digitalWrite(8, LOW);
  }

  // Configure Pin 16 (EXTINT9 / A2) to receive DS3231 interrupts
  pinMode(16, INPUT_PULLUP);
  // Attach the interrupt service routine
  attachInterrupt(16, RTCint, FALLING); // need to check if second edge is falling or rising?

  Serial.begin(115200); // // Connect at 115200
  while (!Serial) ; // Wait until user opens serial monitor
  Serial.println("Set DS3231 RTC using Serial");
  
  // Begin RTC
  if (! rtc.begin()) {
    Serial.println("Panic!! Couldn't find RTC!");
    while (1);
  }

  // Get date and time from serial
  Serial.println("Enter the date and time separated by spaces (YY MM DD HH MM):");
  Serial.println("(Enter the year as two digits)");
  Serial.println("(Enter the hour in 24 hour format)");
  Serial.println("(Wait until seconds reaches zero!)");
  while(Serial.available()==0) {
    ;
  }
  int year = Serial.parseInt();
  int month = Serial.parseInt();
  int day = Serial.parseInt();
  int hour = Serial.parseInt();
  int minute = Serial.parseInt();

  // Set the RTC
  Serial.println("\nSetting RTC...");
  rtc.adjust(DateTime((uint16_t)(year + 2000), (uint8_t)month, (uint8_t)day, (uint8_t)hour, (uint8_t)minute, 0));
  rtc.writeSqwPinMode(DS3231_SquareWave1Hz);

  // clear interrupt flag for a fresh start
  RTCflag = false;
}
     
// RTC interrupt service routine - set RTCflag
void RTCint() {
  RTCflag = true;
}

void loop() // run over and over again
{
  // Wait for interrupt
  while (!RTCflag) {
    ;
  }
  RTCflag = false;

  // Read time
  DateTime now = rtc.now();

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
}
