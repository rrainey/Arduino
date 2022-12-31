#include "SERCOM.h"
#include "Uart.h"
//#include <Adafruit_DotStar.h>

//Adafruit_DotStar strip = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);

// JDY-40 CONNECTIONS for Metro M4
// (left to right looking at the castellated edge; antenna oriented as the top)
//   VCC 
//   RX
//   TX
//   SET; LOW = AT mode; HIGH = OPERATE
//   CS;  LOW = CHIP ENABLE
//   GND
//
// Spec sheets incicate that this board will draw 40mA in transmit mode and 24mA when receiving.
// Since this board draw from a 3.3V regulator on the MCU host device, you'll need to ensure
// that regulator can source this extra current.
//
// See https://w.electrodragon.com/w/images/0/05/EY-40_English_manual.pdf
//
// On the Metro M4 Grand Central to JDY-40 connections
// (see https://learn.adafruit.com/adafruit-grand-central/pinouts)
// Metro 3V pin to VCC 
// Metro TX pin goes to RX
// Metro RX pin goes to TX
#define PIN_SET 2 // Metro D2 to SET pin on JDY-40
#define PIN_CS  3 // Metro D3 to CS  pin on JDY-40
// Metro GND to GND

int rates[]= { 9600, 1200, 2400, 4800, 14400, 19200 };
int nRate = -1;
unsigned long gLastTry_ms;

void tryAnother();

int value = HIGH;

void setup() {

  delay(1000);

  pinMode(13, OUTPUT);

  //strip.begin();
  //strip.setPixelColor(0, 0, 0, 0); strip.show();

  //while (!Serial) { delay (10); }

  //Serial.begin(115200);
  Serial1.begin(9600);

  //Serial.println("Initializing");

  //tryAnother();
  gLastTry_ms = millis();

  //strip.setPixelColor(0, 0, 0, 0); strip.show();
}

void tryAnother() {
  if (++nRate > 5) {
    nRate = 0;
  }
  Serial1.begin(rates[nRate]);

  Serial.print("Trying ");
  Serial.println(rates[nRate]);
}

void loop() {
  //if (Serial.available()) {         // If anything comes in Serial (USB),
  //  Serial1.write(Serial.read());   // read it and send it out Serial1 (pins 0 & 1)
  //}

  if (Serial1.available()) {
    char c = Serial1.read();
    Serial1.write(c);   // echo it
    digitalWrite(PIN_CS, value);
    value = value == HIGH ? LOW : HIGH;
  }

  if ( millis() - gLastTry_ms > 3000) {
    //tryAnother();
    gLastTry_ms = millis();
  }
}
