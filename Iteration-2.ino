#include <MD_MAX72xx.h>   
#include <SPI.h>          // needed because we're using hardware SPI
#include <string>
// Configuration
#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW    // for HW-109 modules 
#define MAX_DEVICES 4                            // 4 chained matrices
#define CS_PIN 5                   // CS on GPIO 5
#define WIDTH 32

byte font[128][5];
// Create the library object
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, 4);

void initFont() {
  // Letter 'A'
  font['A'][0] = 0b01111110;
  font['A'][1] = 0b00010001;
  font['A'][2] = 0b00010001;
  font['A'][3] = 0b00010001;
  font['A'][4] = 0b01111110;

  // Letter 'Q'
  font['Q'][0] = 0b00111110;
  font['Q'][1] = 0b01000001;
  font['Q'][2] = 0b01010001;
  font['Q'][3] = 0b00100001;
  font['Q'][4] = 0b01011110;

  // Letter 'R'
  font['R'][0] = 0b01111111;
  font['R'][1] = 0b00001001;
  font['R'][2] = 0b00001001;
  font['R'][3] = 0b00001001;
  font['R'][4] = 0b01110110;

  // Letter 'W'
  font['W'][0] = 0b00111111;
  font['W'][1] = 0b01000000;
  font['W'][2] = 0b00111100;
  font['W'][3] = 0b01000000;
  font['W'][4] = 0b00111111;
}

void setPixel(int x, int y, bool on) {
  if (x < 0 || x >= 32 || y < 0 || y >= 8) return;
  int block = (31 - x) / 8;
  int a = 7 - (x % 8);
  int i = block * 8 + y;
  mx.setPoint(a, i, on);
}

void drawLetter(char c, int xOffset) {
  for (int col = 0; col < 5; col++) {       // 5 columns per letter
    for (int row = 0; row < 8; row++) {     // 8 rows
      if (font[(byte)c][col] & (1 << row)) {
        // Light up the pixel at (xOffset + col, row) on the matrix
        setPixel(col+xOffset,row,true);
      }
    }
  }
}


void setup() {
  mx.begin();
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);  // ← add this line here
  initFont();
  mx.clear();
}
String message = "ARWQ";
int scrollPos = 0;
void loop() {
  mx.clear();
  int textWidth = message.length() * 6;
  for (int i = 0; i < message.length(); i++) {
    int xPos = WIDTH - scrollPos + (i * 6);
    drawLetter(message[i], xPos);
  }
  mx.update();
  delay(80);
  scrollPos++;
  if (scrollPos > WIDTH + textWidth) {
    scrollPos = 0;
  }
}
