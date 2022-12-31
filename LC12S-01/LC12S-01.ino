//#include <wiring_private.h>

#ifdef ADAFRUIT_TRINKET_M0
//#include "Adafruit_DotStar.h"
#endif

#define LC12S_BAUD_600    0
#define LC12S_BAUD_1200   1
#define LC12S_BAUD_2400   2
#define LC12S_BAUD_4800   3
#define LC12S_BAUD_9600   4
#define LC12S_BAUD_19200  5
#define LC12S_BAUD_38400  6

#define LC12S_XMIT_12_DBM  0
#define LC12S_XMIT_10_DBM  1
#define LC12S_XMIT_9_DBM   2
#define LC12S_XMIT_8_DBM   3
#define LC12S_XMIT_6_DBM   4
#define LC12S_XMIT_3_DBM   5
#define LC12S_XMIT_0_DBM   6
#define LC12S_XMIT_MINUS_2_DBM    7
#define LC12S_XMIT_MINUS_5_DBM    8
#define LC12S_XMIT_MINUS_10_DBM   9
#define LC12S_XMIT_MINUS_15_DBM   10
#define LC12S_XMIT_MINUS_20_DBM   11
#define LC12S_XMIT_MINUS_25_DBM   12
#define LC12S_XMIT_MINUS_30_DBM   13
#define LC12S_XMIT_MINUS_35_DBM   14

#define CHANNEL_START 01
#define CHANNEL_UPPER 05

unsigned short g_usChannel = CHANNEL_START;


//Adafruit_DotStar strip = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);

// LC12S exerciser sketch
//
// Tested with both the Adafruit Metro M4 Grand Centra and Trinket M0
//
// On the Metro M4 Grand Central to LC12 connections
// (see https://learn.adafruit.com/adafruit-grand-central/pinouts)
// Metro / Trinket 3.3V pin to VCC 
// Metro TX pin goes to LC12S RX (TX PIN 4 on Trinket M0)
// Metro RX pin goes to LC12S TX (RX PIN 3 on Trinket M0)
// Metro / Trinket GND to GND
#ifdef ADAFRUIT_TRINKET_M0
#define PIN_SET 0 // D0 on Trinket; Metro D2 to SET pin on LC12S
#define PIN_CS  2 // D2 on Trinket; Metro D3 to CS  pin on LC12S (or just tie CS to GND)
#endif

unsigned long gLastTry_ms;
bool g_bResponsesAsHex = true;
bool g_bAddPrintable = false;

void configureLC12S(unsigned short usChan,
                    unsigned short usNetID,
                    unsigned short usRFPower = LC12S_XMIT_12_DBM,
                    unsigned short usBaud = LC12S_BAUD_9600)
{

  unsigned short checksum = 0;
  const char * bytes = "\xAA\x5A\x00\x01\x00\x55\x00\x00\x00\x04\x00\x01\x00\x00\x00\x12\x00";
  //                              SS  SS  NN  NN  00  RF  00  BB  00  CC  00  00  00  LL  00

  char nbytes[32];
  int i;

  for(i=0; i<17; ++i) {
    nbytes[i] = bytes[i];
  }

  nbytes[4] = usNetID >> 8;
  nbytes[5] = usNetID & 0xFF;

  nbytes[7] = usRFPower & 0xff;
  nbytes[11] = usChan & 0x7F;

  digitalWrite(PIN_SET, LOW);
  delay(10);

  for(i=0; i<17; ++i) {
    checksum += nbytes[i];
    Serial1.write(nbytes[i]);
  }
  Serial1.write(checksum & 0xff );
  delay(20);
20;
  digitalWrite(PIN_SET, HIGH);
}

void queryLC12S(unsigned short *pusChan,
                unsigned short *pusNetID,
                unsigned short *pusRFPower)
{

  unsigned short checksum = 0;
  const char * bytes = "\xAA\x5C\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x12\x00";

  // tested PCB responds with :aa :5d :06 :4b :00 :55 :00 :00 :00 :04 :00 :01 :00 :00 :00 :12 :00 :c4

  char nbytes[32];
  int i;

  digitalWrite(PIN_SET, LOW);
  delay(10);

  for(i=0; i<17; ++i) {
    checksum += bytes[i];
    Serial1.write(bytes[i]);
  }
  Serial1.write(checksum & 0xff );
  delay(20);

  digitalWrite(PIN_SET, HIGH);

  bool bResponseComplete = false;
  int nChar = 0;
  char buffer[8];

  while (bResponseComplete == false) {

    if (Serial1.available() != 0) {
      unsigned char c = Serial1.read();
      nbytes[nChar] = c;
      sprintf(buffer,":%02x ", c);
      Serial.write(buffer);
      if (nChar == 0 && c == '\xAA') {
        nChar = 1;
      }
      else {
        nChar++;
        if (nChar == 18) {
          bResponseComplete = true;
        }
      }
      
    }
  }

  *pusNetID = (nbytes[4] << 8) | nbytes[5];
  *pusRFPower = nbytes[7];
  *pusChan = nbytes[11];
}

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

  digitalWrite(PIN_CS, LOW);

  delay(200);

  unsigned short usChan;
  unsigned short usNetID;
  unsigned short usRFPower;

  queryLC12S(&usChan,
             &usNetID,
             &usRFPower);

  delay(200);

  configureLC12S( g_usChannel, 0x0055, LC12S_XMIT_12_DBM );

  gLastTry_ms = millis();

  digitalWrite(PIN_LED, LOW);

#ifdef ADAFRUIT_TRINKET_M0
  //strip.setPixelColor(0, 0, 0, 0); strip.show();
#endif
}

void loop() {

  if (Serial.available() > 0) {      
    Serial1.write(Serial.read());
  }

  if (Serial1.available() != 0) {
    if (g_bResponsesAsHex) {
      char buffer[40];
      unsigned char c = Serial1.read();
      sprintf(buffer,"%02x ", c);
      Serial.write(buffer);
    }
    else {
      Serial.write(Serial1.read());
    }
  }

  /*
   * Once per second, increment the channel and transmit
   */
  if ( millis() - gLastTry_ms > 1000 ) {

    // March across first eight channels

    if (++g_usChannel > CHANNEL_UPPER) {
      g_usChannel = CHANNEL_START;
    }
    configureLC12S( g_usChannel, 0x0055, LC12S_XMIT_12_DBM );
    

    digitalWrite(PIN_LED, HIGH);


    const char * bytes = "ABCDEFGHIJABCDEFGHIJ";
    int i;
    for(i=0; i<20; ++i) {
      Serial1.write(bytes[i]);
    }

    gLastTry_ms = millis();

    digitalWrite(PIN_LED, LOW);
  }
}
