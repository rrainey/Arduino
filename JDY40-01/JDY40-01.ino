//#include <wiring_private.h>

#ifdef ADAFRUIT_TRINKET_M0
//#include "Adafruit_DotStar.h"
#endif


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
// Spec sheets indicate that this board will draw 40mA in transmit mode and 24mA when receiving.
// Since this board draws from a 3.3V regulator on the MCU host device, you'll need to ensure
// that the regulator can source this extra current.
//
// See https://w.electrodragon.com/w/images/0/05/EY-40_English_manual.pdf
//
// On the Metro M4 Grand Central to JDY-40 connections
// (see https://learn.adafruit.com/adafruit-grand-central/pinouts)
// Metro 3V pin to VCC 
// Metro TX pin goes to RX (TX PIN 4 on Trinket M0)
// Metro RX pin goes to TX (RX PIN 3 on Trinket M0)
#ifdef ADAFRUIT_TRINKET_M0
#define PIN_SET 0 // Metro D2 to SET pin on JDY-40
#define PIN_CS  2 // Metro D3 to CS  pin on JDY-40
#endif
// Metro GND to GND

int rates[]= { 9600, 1200, 2400, 4800, 14400, 19200 };
int nRate = -1;
unsigned long gLastTry_ms;

void tryAnother();
void sendStateQueries();
bool configureChannel(unsigned short channel);

void setup() {

  delay(1000);

#ifdef ADAFRUIT_TRINKET_M0
  //strip.begin();
  //strip.setPixelColor(0, 0, 0, 0); strip.show();
#endif

  Serial.begin(115200);
  while (!Serial) { delay (10); }

  Serial1.begin(9600);
  while (!Serial1) delay(10);

  Serial.println("Initializing");

  pinMode(PIN_SET, OUTPUT);
  pinMode(PIN_CS, OUTPUT);
  pinMode(PIN_LED, OUTPUT);

  // Enable JDY-40 chip
  digitalWrite(PIN_CS, LOW);

  // AT-mode
  digitalWrite(PIN_SET, LOW);
  delay(50);

  //configureChannel( 2 );
  //delay(50);
  Serial1.write("AT+RFC005\r\n");
  delay(50);
  Serial1.write("AT+POWE8\r\n");
  delay(50);
  Serial1.write("AT+RFC\r\n");

  gLastTry_ms = millis();

  digitalWrite(PIN_LED, LOW);

#ifdef ADAFRUIT_TRINKET_M0
  //strip.setPixelColor(0, 0, 0, 0); strip.show();
#endif
}


void tryAnother() {
  if (++nRate > 5) {
    nRate = 0;
  }
  Serial1.begin(rates[nRate]);
  while (!Serial1) delay(10);

  Serial.print("Trying ");
  Serial.println(rates[nRate]);

  sendStateQueries();

  delay(100);
}

/*
  RFID = "Frequency ID?" 0000-FFFF

  DVID = "Device ID" 0000-FFFF

  RFC: "Channel" 001-128

  POWE values

  0：-25db
  1：-15db
  2：-5db
  3：0db
  4：+3db
  5：+6db
  6：+9db
  7：+10db
  8：+10db
  9：+12db

  CLSS values

  A0: Serial port two-way transparent transmission
  C0: Remote controller or IO key indicator light (at transmitting terminal)
  C1: Remote controller or IO key without indicator light (at transmitting terminal)
  C2: IO is low level at normal level, high level after 
      receiving signal and low level after delay 30ms
  C3: IO is high level at normal level, low level after 
      receiving signal and high level after delay 30ms
  C4: IO is low level at normal level, receives pressed 
      signal of high level and receives lift signal low level
  C5: The IO level is reversed when IO receives the pressed signal.

  Default Value:A0
*/

bool configure(unsigned short rfid, unsigned short dvid, unsigned short channel,
               unsigned char power, const char * clss)
{
  char rbuffer[10];

  sprintf(rbuffer, "+RFID%04x\r\n", rfid);
  Serial1.write("AT");
  Serial1.write(rbuffer);

  sprintf(rbuffer, "+DVID%04x\r\n", dvid);
  Serial1.write("AT");
  Serial1.write(rbuffer);

  sprintf(rbuffer, "+RFC%03d\r\n", channel);
  Serial1.write("AT");
  Serial1.write(rbuffer);

  sprintf(rbuffer, "+POWE%01d\r\n", power);
  Serial1.write("AT");
  Serial1.write(rbuffer);

  sprintf(rbuffer, "+CLSS%2.2s\r\n", clss);
  Serial1.write("AT");
  Serial1.write(rbuffer);

  return true;

  //Serial1.write("AT+RFID%04x\r\n");
  //Serial1.write("AT+DVID%04x\r\n");
  //Serial1.write("AT+RFC%03d\r\n");
  //Serial1.write("AT+POWE%01d\r\n");
  //Serial1.write("AT+CLSS%s\r\n");

}

void sendStateQueries() {

  digitalWrite(PIN_LED, HIGH);

  //Serial1.write("AT+BAUD\r\n");
  //delay(100);
  //Serial1.write("AT+RFID\r\n");
  //delay(100);
  //Serial1.write("AT+DVID\r\n");
  //delay(100);
  //Serial1.write("AT+RFC\r\n");
  //delay(100);
  //Serial1.write("AT+POWE\r\n");
  //Serial1.write("AT+CLSS\r\n");
  //Serial1.flush();

  digitalWrite(PIN_LED, LOW);
}

/*
 * 1 .. 128
 *
 * FREQ_MHz = 2400 + (channel - 1)
 */
bool configureChannel(unsigned short channel)
{
  char rbuffer[16];
  sprintf(rbuffer, "+RFC%03d\r\n", channel);
  Serial1.write("AT");
  Serial1.write(rbuffer);

  return true;
}

void loop() {

  if (Serial.available() > 0) {      
    Serial1.write(Serial.read());
  }

  if (Serial1.available() != 0) {
    if (false) {
      char buffer[40];
      unsigned char c = Serial1.read();
      if (c < 32 || c>127) { 
        sprintf(buffer,"%02x ", c);
      }
      else {
        sprintf(buffer,"%02x(%c) ", c, c);
      }
      Serial.write(buffer);
    }
    else {
      Serial.write(Serial1.read());
    }
  }

  if ( millis() - gLastTry_ms > 2000) {

    digitalWrite(PIN_LED, HIGH);
    //sendStateQueries();
    // Set Data Mode
    digitalWrite(PIN_SET, HIGH); 

    Serial1.write("ABCDEFGHIJ");
    gLastTry_ms = millis();

     digitalWrite(PIN_LED, LOW);
  }
}
