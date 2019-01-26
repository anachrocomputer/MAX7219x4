/* MAX7219x4 --- test 4 MAX7219 chips and displays as 16x16 2018-02-09 */
/* Copyright (c) 2018 John Honniball. All rights reserved.             */

/* Released under the GNU Public Licence (GPL) */

#include <SPI.h>

// Direct port I/O defines for Arduino with ATmega328
// Change these if running on Mega Arduino
#define LEDOUT PORTB
#define CS     0x04
#define SDA    0x08
#define SCLK   0x20

// Connections to MAX7219 via SPI port on AVR chip
// I'm actually using a ready-made MAX7219 board and red LED
// matrix display, Deal Extreme (dx.com) SKU #184854
#define slaveSelectPin 10  // CS pin
#define SDAPin 11          // DIN pin
#define SCLKPin 13         // CLK pin

// Registers in MAX7219
#define NOOP_REG        (0x00)
// Display registers 1 to 8
#define DECODEMODE_REG  (0x09)
#define INTENSITY_REG   (0x0A)
#define SCANLIMIT_REG   (0x0B)
#define SHUTDOWN_REG    (0x0C)
#define DISPLAYTEST_REG (0x0F)

// Size of LED matrix
#define MAXX 16
#define MAXY 16
#define MAXROWS 16


// The pixel buffer, 32 bytes
unsigned short FrameBuffer[MAXY];


#define A (1 << 0)
#define B (1 << 1)
#define C (1 << 2)
#define D (1 << 3)
#define E (1 << 4)
#define F (1 << 5)
#define G (1 << 6)
#define DP (1 << 7)

// Table of seven-segment digits 0-9
unsigned char Segtab[10] = {
  A | B | C | D | E | F,     // 0
  D | E,                     // 1
  A | C | D | F | G,         // 2
  C | D | E | F | G,         // 3
  B | D | E | G,             // 4
  B | C | E | F | G,         // 5
  A | B | C | E | F | G,     // 6
  C | D | E,                 // 7
  A | B | C | D | E | F | G, // 8
  B | C | D | E | F | G      // 9
};


int Brightness = 7; // LED matrix display brightness


void setup(void)
{
  Serial.begin(9600);
  
  Serial.println("MAX7219x4");
  Serial.println("John Honniball, Feb 2018");

  // Initialise LED matrix controller chips
  max7219_begin();

  // Clear frame buffer and LED matrix (all pixels off)
  clrFrame();
  
  updscreen();
  
  // Wait one second
  delay(1000);
}


void loop(void)
{
  int x;

  for (x = 1; x < 15; x++) {
    clrFrame();
    
    setRect(0, 0, 15, 15);
    
    setVline(x, 1, 14);
    setHline(1, 14, x);
  
    updscreen();

    delay(100);
  }

  for (x = 14; x > 0; x--) {
    clrFrame();
    
    setRect(0, 0, 15, 15);
    
    setVline(x, 1, 14);
    setHline(1, 14, x);
  
    updscreen();

    delay(100);
  }
}


/* clrFrame --- clear the entire frame (all LEDs off) */

void clrFrame(void)
{
  memset(FrameBuffer, 0, sizeof (FrameBuffer));
}


/* setVline --- draw vertical line */

void setVline(const unsigned int x, const unsigned int y1, const unsigned int y2)
{
  unsigned int y;
  
  for (y = y1; y <= y2; y++)
    setPixel(x, y);
}


/* clrVline --- draw vertical line */

void clrVline(const unsigned int x, const unsigned int y1, const unsigned int y2)
{
  unsigned int y;
  
  for (y = y1; y <= y2; y++)
    clrPixel(x, y);
}


/* setHline --- set pixels in a horizontal line */

void setHline(const unsigned int x1, const unsigned int x2, const unsigned int y)
{
  unsigned int x;
  
  for (x = x1; x <= x2; x++)
    setPixel(x, y);
}


/* clrHline --- clear pixels in a horizontal line */

void clrHline(const unsigned int x1, const unsigned int x2, const unsigned int y)
{
  unsigned int x;

  if (y < MAXY) {
    for (x = x1; x <= x2; x++)
      clrPixel(x, y);
  }
}


/* setRect --- set pixels in a (non-filled) rectangle */

void setRect(const int x1, const int y1, const int x2, const int y2)
{
  setHline(x1, x2, y1);
  setVline(x2, y1, y2);
  setHline(x1, x2, y2);
  setVline(x1, y1, y2);
}


/* setPixel --- set a single pixel in the frame buffer */

void setPixel(const int x, const int y)
{
  FrameBuffer[y] |= (1 << x);
}


/* clrPixel --- clear a single pixel in the frame buffer */

void clrPixel(const int x, const int y)
{
  FrameBuffer[y] &= ~(1 << x);
}


/* updscreen --- update the physical screen from the frame buffer */

void updscreen(void)
{
// About 40us on 16MHz Arduino
//  unsigned long int before, after;
  int r;
  
//  before = micros();
  
  for (r = 0; r < 8; r++) {
    LEDOUT &= ~CS;    //  digitalWrite(slaveSelectPin, LOW);
    SPI.transfer(r + 1);
    SPI.transfer(FrameBuffer[r + 8] >> 8);
    SPI.transfer(r + 1);
    SPI.transfer(FrameBuffer[r + 8] & 0xff);
    SPI.transfer(r + 1);
    SPI.transfer(FrameBuffer[r] >> 8);
    SPI.transfer(r + 1);
    SPI.transfer(FrameBuffer[r] & 0xff);
    LEDOUT |= CS;     //  digitalWrite(slaveSelectPin, HIGH);
  }

//  after = micros();
  
//  Serial.print(after - before);
//  Serial.println("us updscreen");
}


/* max7219_begin --- initialise the MAX219 LED driver */

void max7219_begin(void)
{
  int i;

  /* Configure I/O pins on Arduino */
  pinMode(slaveSelectPin, OUTPUT);
  pinMode(SDAPin, OUTPUT);
  pinMode(SCLKPin, OUTPUT);
  
  digitalWrite(slaveSelectPin, HIGH);
  digitalWrite(SDAPin, HIGH);
  digitalWrite(SCLKPin, HIGH);

  SPI.begin();
  // The following line fails on arduino-0021 due to a bug in the SPI library
  // Compile with arduino-0022 or later
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  
  /* Start configuring the MAX7219 LED controller */
  max7219write(DISPLAYTEST_REG, 0); // Switch off display test mode
  
  max7219write(SHUTDOWN_REG, 1 | (1 << 8));    // Exit shutdown

  max7219write(INTENSITY_REG, 7 | (7 << 8));   // Brightness half

  max7219write(DECODEMODE_REG, 0);  // No decoding; we don't have a 7-seg display

  max7219write(SCANLIMIT_REG, 7 | (7 << 8));   // Scan limit 7 to scan entire display

  for (i = 0; i < 8; i++) {
    max7219write(i + 1, 0);
  }
}


/* max7219write --- write a command to the MAX7219 */

void max7219write(const unsigned char reg, const unsigned short val)
{
// Use direct port I/O and hardware SPI for speed

  LEDOUT &= ~CS;    //  digitalWrite(slaveSelectPin, LOW);
  SPI.transfer(reg);
  SPI.transfer(val >> 8);
  SPI.transfer(reg);
  SPI.transfer(val & 0xff);
  LEDOUT |= CS;     //  digitalWrite(slaveSelectPin, HIGH);
}

