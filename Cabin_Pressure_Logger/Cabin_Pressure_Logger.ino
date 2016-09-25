// Cabin Pressure Logger

// This code is written for the Adalogger M0 Feather
// https://www.adafruit.com/products/2796
// Log DS3231 RTC time and MPL3115A2 pressure and temperature to SD card once per second
// Change file name every 10 mins to avoid accidental data corruption
// 1Hz timing is provided by DS3231 RTC interrupt

//#define NoLights // Uncomment this line to disable LEDs during logging to save power

// Sparkfun MPL3115A2 Pressure Sensor
// https://www.sparkfun.com/products/11084
// https://github.com/sparkfun/SparkFun_MPL3115A2_Breakout_Arduino_Library
// Shares the I2C bus with the RTC
// Power is provided by Adalogger pins D5&D6
// Solder a 4-pin header onto the back of the PCB, ignoring the INT connections, and slot
// into the DS3231 Featherwing Bottom-Right four pins: SDA, SCL, D5 and D6
#include <Wire.h>
#include "SparkFunMPL3115A2.h"
MPL3115A2 myPressure;

// DS3231 Precision RTC Featherwing
// https://www.adafruit.com/products/3028
// Solder a 2-pin header onto the featherwing to link DS3231 INT to Adalogger pin 16
// to provide 1Hz interrupts
//#include <Wire.h>
#include "RTClib.h"
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
volatile bool RTCflag = false; // RTC interrupt flag

// SD card logging
// Ensure SD card is formatted as FAT32!
#include <SPI.h>
#include <SD.h>
#define cardSelect 4
File dataFile;

void setup()
{
  pinMode(6, OUTPUT); // MPL3115A2 GND
  digitalWrite(6, LOW);
  pinMode(5, OUTPUT); // MPL3115A2 VCC
  digitalWrite(5, HIGH);
  
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
  pinMode(16, INPUT_PULLUP); // Pull up is required as DS3231 output is open drain
  attachInterrupt(16, RTCint, FALLING); // Attach the interrupt service routine
  
  delay(10000); // Allow 10 sec to open serial monitor
  
  Serial.begin(9600);
  Serial.println("Log RTC Time and MPL3115A2 Pressure to SD card");
  Serial.println("Green LED Flash = RTC Interrupt");
  Serial.println("Red LED Flash = SD Write");
  Serial.println("Continuous Red indicates an SD problem or low battery!");

  // MPL3115A2 Init
  Wire.begin();        // Join i2c bus
  myPressure.begin(); // Get sensor online

  // Configure the pressure sensor
  //myPressure.setModeAltimeter(); // Measure altitude above sea level in meters
  myPressure.setModeBarometer(); // Measure pressure in Pascals from 20 to 110 kPa
  myPressure.setOversampleRate(7); // Set Oversample to the recommended 128
  myPressure.enableEventFlags(); // Enable all three pressure and temp event flags
  delay(3000); // Allow 3 sec for MPL3115A2 to initialise
  // Read and discard Pressure and Temperature
  float pressure = myPressure.readPressure();
  float temperature = myPressure.readTemp();

  // flash the red LED during RTC and SD initialisation
#ifndef NoLights
  digitalWrite(13, HIGH);
#endif

  // Initialise the DS3231 RTC
  Serial.println("Initializing RTC...");
  if (! rtc.begin()) { // rtc.begin just does wire.begin so unlikely to fail!
    Serial.println("Panic!! Couldn't find RTC!");
    while(1);
  }
  if (rtc.lostPower()) {
    Serial.println("Panic!! RTC lost power!");
    //while(1); // Might as well keep going?
  }
  Serial.println("RTC initialized!");
  
  // Initialise SD card
  Serial.println("Initializing SD card...");
  // See if the SD card is present and can be initialized:
  if (!SD.begin(cardSelect)) {
    Serial.println("Panic!! SD Card Init failed, or not present!");
    // don't do anything more:
    while(1);
  }
  Serial.println("SD Card initialized!");

  // turn red LED off
  digitalWrite(13, LOW);

  Serial.println("Here we go...");
  delay(1000);

  Serial.end(); // Stop serial comms to save power
  USBDevice.detach(); // Detach USB

  RTCflag = false; // Clear RTCflag for a fresh start
}
     
// RTC interrupt service routine - set RTC flag and turn green LED on
void RTCint() {
  RTCflag = true; // Set RTCflag
#ifndef NoLights
  digitalWrite(8, HIGH); // Turn the green LED on
#endif
}

void loop() // run over and over again
{
  while (RTCflag == false) { // Wait for RTC flag to be set
    __DSB(); // Data Synchronization Barrier
    __WFI(); // Snooze (not deep sleep) until next interrupt
  }
  
  RTCflag = false; // Clear RTCflag

  // Read time
  DateTime now = rtc.now();

  // Read Pressure, Temperature and Battery Voltage
  float pressure = myPressure.readPressure();
  float temperature = myPressure.readTemp();
  float vbat = analogRead(A7) * (2.0 * 3.3 / 1023.0);

  // Turn the green LED off to indicate a successful read
  digitalWrite(8, LOW);

  // string for log data assembly
  String dataString = "";

  // divides to turn times into chars
  char hourT = now.hour()/10 + '0';
  char hourU = now.hour()%10 + '0';
  char minT = now.minute()/10 + '0';
  char minU = now.minute()%10 + '0';
  char secT = now.second()/10 + '0';
  char secU = now.second()%10 + '0';
  char yearT = (now.year()-2000)/10 +'0';
  char yearU = (now.year()-2000)%10 +'0';
  char monT = now.month()/10 +'0';
  char monU = now.month()%10 +'0';
  char dayT = now.day()/10 +'0';
  char dayU = now.day()%10 +'0';

  // assemble dataString
  dataString += hourT; dataString += hourU; dataString += ':';
  dataString += minT; dataString += minU; dataString += ':';
  dataString += secT; dataString += secU;
  dataString += ' '; // separate time and date with a space (not a ',') to make life easier for numpy.loadtxt
  dataString += dayT; dataString += dayU; dataString += '/';
  dataString += monT; dataString += monU; dataString += "/20";
  dataString += yearT; dataString += yearU; dataString += ',';
  dataString += String(pressure, 2); dataString += ',';
  dataString += String(temperature, 2); dataString += ',';
  dataString += String(vbat, 2); //dataString += '\n';

  // flash red LED to indicate SD write (leave on if an error occurs)
#ifndef NoLights
  digitalWrite(13, HIGH);
#endif

  // check if voltage is >= 3.55V, if not then don't try to write! (Could corrupt the SD card...)
  if (vbat >= 3.55) {
    
    // filename is limited to 8.3 characters so use format: YYYYMMDD/HHMM.csv
    char filename[] = "20000000/0000.csv";
    filename[2] = yearT;
    filename[3] = yearU;
    filename[4] = monT;
    filename[5] = monU;
    filename[6] = dayT;
    filename[7] = dayU;
    filename[9] = hourT;
    filename[10] = hourU;
    filename[11] = minT; // Comment this line and the next line out for 1 hour file boundaries.
    //filename[12] = minU; // Comment this line out for 10 minute file boundaries. Default is 1 minute.
    
    char dirname[] = "20000000";
    dirname[2] = yearT;
    dirname[3] = yearU;
    dirname[4] = monT;
    dirname[5] = monU;
    dirname[6] = dayT;
    dirname[7] = dayU;
  
    // try to create subdirectory (even if it exists already)
    SD.mkdir(dirname);
  
    // Open the file for writing
    dataFile = SD.open(filename, FILE_WRITE);
    
    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      digitalWrite(13, LOW);  // Turn the red LED off
    }
    // if the file isn't open or the battery voltage is low, leave the red LED on
  }
}

