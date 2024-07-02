/*
   Board Sparkfun Redboard

   2    D2    PD2   INT0
   3    D3    PD3   INT1

   8255
   Port A D0-7 <--> Arduino PC0-3,PD4-7 (A0-3, D4-7)

   8255 to Redboard
   PC7  /OBF   output to  Arduino pin 2 (INT0), falling edge
   PC6  /ACK   input from Arduino pin A5 (PC5)
   read data

   Redboard to 8255
   write data
   PC4  /STB   input from Arduino pin A4 (PC4)
   PC5  IBF    output to  Arduino pin 3 (INT1), falling edge

   SoftwareSerial
   Tx   9
   Rx   8

   microSD shield
   CS   8
   MOSI 11
   MISO 12
   SCK  13
*/

#ifdef USE_SOFTWARE_SERIAL
#include <SoftwareSerial.h>
#endif

#include "intf_8255.h"

#ifdef USE_SOFTWARE_SERIAL
const byte rxPin = 8;
const byte txPin = 9;

// Set up a new SoftwareSerial object
SoftwareSerial mySerial (rxPin, txPin);
#endif

void setup() {
#ifdef USE_SOFTWARE_SERIAL
  // Define pin modes for TX and RX
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);

  // Set the baud rate for the SoftwareSerial object
  mySerial.begin(9600);
  mySerial.println("\033[2JHello world!");
#else
  Serial.begin(115200);
  Serial.println("\033[2JHello world!");
#endif

  intf_8255_init();
  delay(500);
}

void loop() {
  byte value = 0;

  if (In8255Avail())
  {
    value = ReadData();
#ifdef USE_SOFTWARE_SERIAL
  mySerial.write(value);
#else
  Serial.write(value);
#endif
  }
  
  if (Serial.available() > 0) {
    WriteTo8255(Serial.read());
  }

//  do
//  {
//    WriteTo8255(value);
//    value = value+1;
//    delay(100);
//  } while(true);
}
