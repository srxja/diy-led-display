# LED DISPLAY

DIY LED Display to express your emotions, made especially during driving purposes, because well sometimes I just want to flip someone off but I also need to be a responsible driver.

## Initial Simulation

Wokwi -> ESP32 -> Add MAX7219.
For connections, refer to Wokwi files.

## Components


## Iteraion 1

I initially began with the design for WS2812 LED Matrix, but I switched it MAX7219 later.
I was using the FastLED library, and here is what I did:

- Font array: byte font[128][5]; 
- Helper function to convert (x,y) into 1D index:
  int XY(int x, int y) {
     if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return -1;  // for invalid values
     return x + WIDTH * y;
   }
-  Every letter is stored like this:
  ``` byte letterA[8] = {
  0b00011000,  // ...##...
  0b00111100,  // ..####..
  0b01100110,  // .##..##.
  0b01100110,  // .##..##.
  0b01111110,  // .######.
  0b01100110,  // .##..##.
  0b01100110,  // .##..##.
  0b00000000   // ........
  }; ```
 So this is how drawLetter function works:
for (int y = 0; y < 8; y++) {
  for (int x = 0; x < 8; x++) {
    if (letterA[y] has a 1 at column x) {
      leds[XY(x, y)] = CRGB::Red;
    }
  }
}
To check if bit x is a 1 in a byte: 
if (row & (1 << (7 - x))) {
  // bit at column x is 1
}  
A little explanation:
1 << n means "take the number 1 and shift it left by n positions." So 1 << 3 is 0b00001000 (a byte with a 1 only in the 3rd position from the right).
(7 - x) is because in the pattern, column 0 is the leftmost bit (the highest-value position in the byte), and column 7 is the rightmost. So if we want to check column 0, we shift by 7. Column 1, shift by 6. And so on.
row & mask is a bitwise AND. It returns non-zero only if both sides have a 1 in the same position.
- Create a drawLetter Function:
  void drawLetter(char c, int xOffset, CRGB color){
for(int col=0;col<5;col++){
for(int row=0;row<HEIGHT;row++){
if(font[(byte)c][col] & (1<<row)){
leds[XY(xOffset+col,row)] = color;
      }
    }
  }
}  

## I was not able to get the original panel so I had to switch to MAX7219 LED Modules

## Iteration 2: MAX7219 LED Modules
### refer to its github repo : super helpful


<img width="656" height="351" alt="Screenshot 2026-05-23 at 14 05 34" src="https://github.com/user-attachments/assets/f8916cec-827a-413e-9a85-2ae022b942b6" />

Now setting up this module was fairly simple and it has its own library, so the repo helped a lot. I used the generic module. Now, the most challenging part in setup was figuring out the configuration and indices.
Initially I thought it was (row, col) but I was wrong. I had to do multiple sweeps for debugging. Here is what I found:
In the successful debugging sweep, this was my code:
for (int a = 0; a < 10; a++) {
    for (int i=0;i<32;i++){
    mx.clear();
    mx.setPoint(a, i, true);
    mx.update();
    Serial.print("setPoint("); Serial.print(a); Serial.println(",");Serial.println(i);
    delay(400);
    }
 }
 So from this code I found that variable 'a' only defines the columns of one module. So its values are between 0 and 7 (including both) ONLY. and variable i actually goes from 0 to 32 and it defines the rows as well as the LED block.
 So i=0 to 7 is block 1 (rightmost), 8 to 15 is the next block and so on.

 Using this information I was able to create the setPixel function to convert (row,col) requests to ones the simulator understands:
 
 void setPixel(int x, int y, bool on) {
  if (x < 0 || x >= 32 || y < 0 || y >= 8) return;
  int block = (31 - x) / 8;
  int a = 7 - (x % 8);
  int i = block * 8 + y;
  mx.setPoint(a, i, on);
}     

Next I created the scrolling logic:
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

and the drawLetter function, which is similar to the one in iteration 1.
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

<img width="685" height="388" alt="Screenshot 2026-05-23 at 14 10 16" src="https://github.com/user-attachments/assets/1d764a23-a6ef-4955-b628-3633cd2a4476" />

This gave me the initial scrolling logic
