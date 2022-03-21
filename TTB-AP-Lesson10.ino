// Using an Arduino with Python LESSON 10: Passing Data from Python to Arduino.
// https://www.youtube.com/watch?v=dbZZlq1_M4o
// https://toptechboy.com/

// https://www.arduino.cc/en/Tutorial/BuiltInExamples/SerialEvent
// https://www.best-microcontroller-projects.com/arduino-strtok.html

#include <DHT.h>              // DHT11/22 sensor library.

//Debugging & testing defines.
//#define TESTSRX               // Enable serial received command testing serial prints.

// DHT11/22 sensor defines.
#define DHTTYPE11 DHT11       // Blue module, DHT11 defined as 11 in <DHT.h>.
#define DHTTYPE22 DHT22       // White module, DHT22 defined as 22 in <DHT.h>.

// Arduino analog I/O pin defines.
#define POT1PIN A0            // Also known as IO pin 21.

// Arduino digital I/O pin defines.
#define DHT11PIN 3
#define DHT22PIN 4
#define BLLEDPIN 5
#define GNLEDPIN 6
#define RDLEDPIN 7
#define HBLEDPIN LED_BUILTIN  // Usually digital pin 13.

// Some (re)defines to make LED control easier to follow.
#define ON HIGH
#define OFF LOW

// Function prototypes - this allows the definition of default values.
void bgrLEDBank(int action = 0);

//Code loop job defines.
#define JOB1CYCLE 100         // Job 1 execution cycle: 0.1s  - Get the data: Read the potentiometers.
#define JOB2CYCLE 1000        // Job 2 execution cycle: 1s    - Get the data: Read the DHT11 sensor.
#define JOB3CYCLE 2000        // Job 3 execution cycle: 2s    - Get the data: Read the DHT22 sensor.
#define JOB4CYCLE 100         // Job 4 execution cycle: 0.1s  - Share the results: Output data to the serial console.
#define JOB5CYCLE 10          // Job 5 execution cycle: 0.01s - Action commands: Parse and action any received serial commands.
#define JOB9CYCLE 500         // Job 9 execution cycle: 0.5s  - Toggle the heartbeat LED.

#define CMDBUFFERMAX 32               // The maximum size of the command buffer in which received command characters are stored. 
char commandBuffer[CMDBUFFERMAX + 1]; // A charater array variable to hold incoming command character data.
const char cmdDelimiter[] = " ,=:~!"; // The delimiter for the command subject and command action can be any of these characters.
bool commandReady = false;            // A flag to indicate that the current command is ready to be actioned.

// Sensor object initialisations.
DHT myDHT11(DHT11PIN, DHTTYPE11);     // Initialize the DHT11 sensor for normal 16mhz Arduino (default delay = 6).
DHT myDHT22(DHT22PIN, DHTTYPE22);     // Initialize the DHT22 sensor for normal 16mhz Arduino (default delay = 6).

void setup() {
  // Initialise the potentiometer pins.
  pinMode(POT1PIN, INPUT);
  // Start the DHT11 sensor.
  myDHT11.begin();
  // Start the DHT22 sensor.
  myDHT22.begin();
  // Initialise the LED pins.
  pinMode(BLLEDPIN, OUTPUT);
  pinMode(GNLEDPIN, OUTPUT);
  pinMode(RDLEDPIN, OUTPUT);
  pinMode(HBLEDPIN, OUTPUT);
  // Start the serial port.
  Serial.begin(115200);
  while(!Serial); // Wait for the serial I/O to start.
}

void loop() {
  // Initialise the heartbeat status variable - OFF = LOW = 0.
  static bool hbStatus = OFF;
  // Initialise the potentiometer variable to something that indicates an invalid reading.
  static int pot1Value = -1;
  // Initialise the DHT variables to something that indicates invalid readings.
  static float temperatureDHT11 = NAN;
  static float humidityDHT11    = NAN;
  static float temperatureDHT22 = NAN;
  static float humidityDHT22    = NAN;
  // Record the current time. When a single timeNow is used for all jobs it ensures they are synchronised.
  unsigned long timeNow = millis();
  // Job variables. Set to timeNow so that jobs do not start immediately - this allows the sensors to settle.
  static unsigned long timeMark1 = timeNow; // Last time Job 1 was executed.
  static unsigned long timeMark2 = timeNow; // Last time Job 2 was executed.
  static unsigned long timeMark3 = timeNow; // Last time Job 3 was executed.
  static unsigned long timeMark4 = timeNow; // Last time Job 4 was executed.
  static unsigned long timeMark5 = timeNow; // Last time Job 5 was executed.
  static unsigned long timeMark9 = timeNow; // Last time Job 9 was executed.
  // Job 1 - Get the data: Read the potentiometers.
  if (timeNow - timeMark1 >= JOB1CYCLE) {
    timeMark1 = timeNow;
    // Do something...
    pot1Value = analogRead(POT1PIN);
  }
  // Job 2 - Get the data: Read the DHT11 sensor.
  if (timeNow - timeMark2 >= JOB2CYCLE) {
    timeMark2 = timeNow;
    // Do something...   
    temperatureDHT11 = myDHT11.readTemperature();
    humidityDHT11 = myDHT11.readHumidity();
  }
  // Job 3 - Get the data: Read the DHT22 sensor.
  if (timeNow - timeMark3 >= JOB3CYCLE) {
    timeMark3 = timeNow;
    // Do something...
    temperatureDHT22 = myDHT22.readTemperature();
    humidityDHT22 = myDHT22.readHumidity();
  }
  // Job 4 - Share the results: Output CSV data to the serial console.
  if (timeNow - timeMark4 >= JOB4CYCLE) {
    timeMark4 = timeNow;
    // Do something...
    #ifndef TESTSRX
      Serial.print(pot1Value);
      Serial.print(",");
      Serial.print(temperatureDHT11);
      Serial.print(",");
      Serial.print(humidityDHT11);
      Serial.print(",");
      Serial.print(temperatureDHT22);
      Serial.print(",");
      Serial.print(humidityDHT22);
      Serial.println();
    #endif
  }
  // Job 5 - Action commands: Parse and action any received serial commands.
  if (timeNow - timeMark5 >= JOB5CYCLE) {
    timeMark5 = timeNow;
    // Do something...
    if (commandReady) {
      // Parse the command - NULL is returned if nothing is found by strtok().
      // strtok() is an interesting command! It is quite clever and is reenterant.
      char *subject = strtok(commandBuffer, cmdDelimiter);  // A pointer to a NULL terminated part of the command buffer.
      char *action = strtok(NULL, cmdDelimiter);            // A pointer to another NULL terminated part of the command buffer.
      #ifdef TESTSRX
        Serial.print("Cmd Received: ");
        if (subject != NULL) {
          Serial.print(subject);
        }
        else {
          Serial.print("S-NULL");
        }
        Serial.print(" = ");
        if (subject != NULL) {
          Serial.println(action);
        }
        else {
          Serial.print("A-NULL");
        }
      #endif
      // Lets action the command.
      if (strcmp(subject, "bgrLEDs") == 0 and action != NULL) {
        bgrLEDBank(atoi(action));   // We have a recognised subject and an action for it. 
      }
      // All done, so clear the ready flag for the next command to be received.
      commandReady = false;
    }
  }
  // Job 9 - Toggle the heartbeat LED.
  if (timeNow - timeMark9 >= JOB9CYCLE) {
    timeMark9 = timeNow;
    // Do something...
    hbStatus = !hbStatus; // Toggle the heartbeat status.
    digitalWrite(HBLEDPIN, hbStatus);
  }
}

// SerialEvent automatically executes after each run of main loop if new serial data has been received.
void serialEvent() {
  // We must preserve the command buffer index between calls as this function may not collect a whole command in a single call.
  static byte bufferIndex = 0;
  while (Serial.available() and not commandReady) {
    // Get the new byte of data from the serial rx buffer.
    char rxChar = (char)Serial.read();
    // If we have received the end of command delimiter or reached the end of the buffer, finish the string and set a flag for the main loop to action the command.
    if (rxChar == '\n' or bufferIndex == CMDBUFFERMAX) {
      commandBuffer[bufferIndex] = '\0';  // Terminate the string.
      commandReady = true;                // Set the command redy flag for the main loop.
      bufferIndex = 0;                    // Reset the buffer index in readyness for the next command.
    }
    // Otherwise, if we are builiding a new command, add the data to the command buffer and increment the buffer index.
    if (not commandReady) {
      commandBuffer[bufferIndex++] = rxChar;
    }
  }
}

// Turn ON/OFF the blue/green/red LEDs as per the command action.
void bgrLEDBank(int action) {
  // Valid action values are 0 - 7 using bits 0 - 2, otherwise the requetsed action is ignored.
  if (action <= 7){
    // Action value bit 0 controls the blue LED.
    if (bitRead(action, 0) == 1) {
      digitalWrite(BLLEDPIN, ON);
      #ifdef TESTSRX
        Serial.println("Blue ON");
      #endif
    }
    else {
      digitalWrite(BLLEDPIN, OFF);
      #ifdef TESTSRX
        Serial.println("Blue OFF");
      #endif
    }
    // Action value bit 1 controls the green LED.
    if (bitRead(action, 1) == 1) {
      digitalWrite(GNLEDPIN, ON);
      #ifdef TESTSRX
        Serial.println("Green ON");
      #endif
    }
    else {
      digitalWrite(GNLEDPIN, OFF);
      #ifdef TESTSRX
        Serial.println("Green OFF");
      #endif
    }
    // Action value bit 2 controls the red LED.
    if (bitRead(action, 2) == 1) {
      digitalWrite(RDLEDPIN, ON);
      #ifdef TESTSRX
        Serial.println("Red ON");
      #endif
    }
    else {
      digitalWrite(RDLEDPIN, OFF);
      #ifdef TESTSRX
        Serial.println("Red OFF");
      #endif
    }
  }
}

// EOF
