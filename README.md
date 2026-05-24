# LED DISPLAY

DIY LED Display to express your emotions, made especially during driving purposes, because well sometimes I just want to flip someone off but I also need to be a responsible driver.

## Initial Simulation

Wokwi -> ESP32 -> Add MAX7219.
For connections, refer to Wokwi files.

## Components


## Dev Setup

I shifted from Wokwi web to PlatformIO + Wokwi VS Code extension midway. So now my code lives in `src/main.cpp` and `src/webpage.h`, wiring is in `diagram.json`, build config in `platformio.ini`, and simulation config in `wokwi.toml`. I still edit the diagram on Wokwi web and copy-paste it over because it's easier. Don't need `libraries.txt` locally because PlatformIO reads `lib_deps` straight from `platformio.ini`.

## Iteraion 1

I initially began with the design for WS2812 LED Matrix, but I switched it MAX7219 later.
I was using the FastLED library, and here is what I did:

Font array: byte font[128][5]; 
### Helper function to convert (x,y) into 1D index:
```
int XY(int x, int y) {
if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return -1;  // for invalid values
return x + WIDTH * y;
}
```
Every letter is stored like this:
 ```
byte letterA[8] = {
0b00011000,  // ...##...
0b00111100,  // ..####..
0b01100110,  // .##..##.
0b01100110,  // .##..##.
0b01111110,  // .######.
0b01100110,  // .##..##.
0b01100110,  // .##..##.
0b00000000   // ........
};
```
 So this is how the function works:
 ```
for (int y = 0; y < 8; y++) {
for (int x = 0; x < 8; x++) {
if (letterA[y] has a 1 at column x) {
leds[XY(x, y)] = CRGB::Red;
}
}
}
```
To check if bit x is a 1 in a byte:
```
if (row & (1 << (7 - x))) {
// bit at column x is 1
}
```
A little explanation:
- 1 << n means "take the number 1 and shift it left by n positions." So 1 << 3 is 0b00001000 (a byte with a 1 only in the 3rd position from the right).
(7 - x) is because in the pattern, column 0 is the leftmost bit (the highest-value position in the byte), and column 7 is the rightmost. So if we want to check column 0, we shift by 7. Column 1, shift by 6. And so on.
- row & mask is a bitwise AND. It returns non-zero only if both sides have a 1 in the same position.
  
### Create a drawLetter Function:
```
void drawLetter(char c, int xOffset, CRGB color){
for(int col=0;col<5;col++){
for(int row=0;row<HEIGHT;row++){
if(font[(byte)c][col] & (1<<row)){
leds[XY(xOffset+col,row)] = color;
}
}
}
}
```

## I was not able to get the original panel so I had to switch to MAX7219 LED Modules

## Iteration 2: MAX7219 LED Modules
### refer to its github repo : super helpful

<img width="656" height="351" alt="Screenshot 2026-05-23 at 14 05 34" src="https://github.com/user-attachments/assets/f8916cec-827a-413e-9a85-2ae022b942b6" />

Now setting up this module was fairly simple and it has its own library, so the repo helped a lot. I used the generic module. Now, the most challenging part in setup was figuring out the configuration and indices.
Initially I thought it was (row, col) but I was wrong. I had to do multiple sweeps for debugging. Here is what I found:
In the successful debugging sweep, this was my code:
```
for (int a = 0; a < 10; a++) {
for (int i=0;i<32;i++){
mx.clear();
mx.setPoint(a, i, true);
mx.update();
Serial.print("setPoint("); Serial.print(a); Serial.println(",");Serial.println(i);
delay(400);
}
}
```
So from this code I found that variable 'a' only defines the columns of one module. So its values are between 0 and 7 (including both) ONLY. and variable i actually goes from 0 to 32 and it defines the rows as well as the LED block.
 So i=0 to 7 is block 1 (rightmost), 8 to 15 is the next block and so on.

 Using this information I was able to create the setPixel function to convert (row,col) requests to ones the simulator understands:
 ```
void setPixel(int x, int y, bool on) {
if (x < 0 || x >= 32 || y < 0 || y >= 8) return;
int block = (31 - x) / 8;
int a = 7 - (x % 8);
int i = block * 8 + y;
mx.setPoint(a, i, on);
}
```

Next I created the scrolling logic:
```
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
```
and the drawLetter function, which is similar to the one in iteration 1.
```
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
```
<img width="685" height="388" alt="Screenshot 2026-05-23 at 14 10 16" src="https://github.com/user-attachments/assets/1d764a23-a6ef-4955-b628-3633cd2a4476" />

This gave me the initial scrolling logic.

(I forgot to save the rest and restarted my laptop so I lost everything 😭😭😭, so the next part is shortened now).
Next up, I decided to switch things up a little to display everything and switched to the MD_MAX72XX library for letters and stuff instead. 

## Wifi and Web Server
ESP32 hosts its own WiFi network, so basically connect phone → open a webpage → type a message → it scrolls on the matrix.

### Why host everything on the ESP32 instead of a normal website

I actually considered hosting the webpage online (like on GitHub Pages) and having it talk to the ESP32 somehow. But I went with the "ESP32 hosts the whole thing" approach because:
- Works completely offline : no cell signal, no home WiFi needed. Drive anywhere, it still works.
- Zero infrastructure : no domain to renew, no server to maintain.
- Privacy: the message goes phone → ESP32, never touches the internet.
- Lower latency : local WiFi is sub-100ms, cloud round-trips would be 300–800ms.

The only downside is that the HTML lives inside the C++ code as a big string, which feels gross. But it's only aesthetic — there's no performance cost because the string is read once at compile time. And I fixed the gross factor later by moving it to its own header file (see WebPage.h section).

### Setting up the Access Point

- Used WiFi.h to put the ESP32 into Access Point (AP) mode instead of connecting to an existing WiFi network -> ESP32 becomes the router.
- SSID: "CarDisplay", password:setup an 8 digit password.
- On boot, prints the AP's IP address to serial (defaults to 192.168.4.1).

### Setting up the Web Server

- Used WebServer.h — created a WebServer server(80) listening on port 80 (the standard HTTP port).
- Registered two routes:
server.on("/", handleRoot) — serves the form page
server.on("/submit", handleSubmit) — processes the submitted message
server.begin() in setup() to start the server.

The polling pattern-> server.handleClient()

- Called server.handleClient() at the top of every loop() iteration.
>[!NOTE]
>Key implication: loop() must never block. The 80ms scroll delay is fine, but anything heavier would make the web page feel laggy. Every long-running task on a microcontroller has to be split into tiny non-blocking chunks taking turns inside loop().

The handlers:

- handleRoot() — returns an HTML page containing a form with a text input and a Send button. Used single quotes for HTML attributes inside the C++ string to avoid escaping every ".
- handleSubmit() — reads server.arg("msg") (the WebServer library auto-parses POST bodies into server.arg() lookups), assigns it to the global message variable, resets scrollPos = 0 so the new message starts fresh from the right edge, and sends back a "Got it!" confirmation page with a link back to /.
The bridge: the global message variable bridges the web handler and the scroll loop. The scroll engine just reads message every iteration and renders whatever's there — it doesn't need to be told it changed.

### Beautifying the UI
Next up I beautified the UI, added some emoticon buttons, textbox, two options: loop or single display, and integrated it all in my ino file. Also created a WebPage.h file.

### WebPage.h
Writing all my HTML+CSS+JS as one giant escaped string inside the .ino was unreadable, so I moved it to its own header file `webpage.h`. The trick is C++ raw string literals:
```
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
... code ...
</html>
)rawliteral";
```
- `R"rawliteral(...)rawliteral"` means everything between the markers is treated literally — no need to escape quotes or newlines, I can just write HTML the normal way.
- `PROGMEM` tells the compiler to store the string in flash instead of RAM. ESP32 has plenty of both but it's good practice for big strings.
Then in the .ino I just `#include "webpage.h"` and `handleRoot` becomes a one-liner:
```
void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
}
```
Two things that wasted me a chunk of time
1. Every emoji button needs type="button"
By default any <button> inside a <form> acts as a submit button. So tapping :) was submitting the form prematurely instead of just inserting the emoticon into the textbox. Setting type="button" on each emoji button tells the browser "this is just a button, don't submit anything." Only the SEND button keeps type="submit".
2. Custom-looking radio buttons via hidden inputs
The Loop / Show Once mode selector looks like two styled pills, but underneath each pill is an actual <input type="radio"> hidden with display:none. A tiny bit of JS toggles which one is checked when you tap a pill. This way the form still naturally submits mode=loop or mode=once without any extra parsing — the prettiness is just on top, the underlying form submission is boring and standard.
Two booleans for mode handling, not one
I needed two globals to handle the loop-vs-once logic:

loopMessage — the user's intent (loop or once). Persists across messages.
messageActive — the current state (is the message still scrolling, or has it finished?). Flips to false once a "once" message has scrolled all the way off, which tells loop() to stop redrawing.

## HARDWARE

First up I daisy chained my LED modules together. Here is how it is supposed to be chained:
- VCC to VCC
- GND to GND
- DIN to DOUT
- CS to CS
- CLK to CLK

I did this and connected all 4 modules. The output side of the last module remains bare.
Next, connecting the ESP32 to the first module. (FORMAT: module pin to esp32 pin)
- VCC to VIN/5V
- GND to GND
- DIN to GPIO23
- CS to GPIO5
- CLK to GPIO18

Right now this is going to be powered by my laptop. I do not have the necessary power supply, so will update here once I do.
Also, I migrated this whole codebase to VSC with PlatformIO and Wokwi for VSC, so you can just copy the codes from Wokwi web to the files in VSC. Also, the WebPage.h goes in src with main.cpp. Here is the folder structure.

<img width="204" height="402" alt="Screenshot 2026-05-24 at 20 23 42" src="https://github.com/user-attachments/assets/ea62a49d-6b3a-4757-a5bd-5ca00415e4cc" />

Here is what goes in the PlatformIO.ino:
```
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_port = /dev/cu.SLAB_USBtoUART 
monitor_port = /dev/cu.SLAB_USBtoUART
lib_deps = 
    majicdesigns/MD_MAX72XX@^3.5.1
```
>[!NOTE]
>For upload_port and monitor_port you need to find out the name for your specific device, which can be found by running this code:
>ls /dev/cu.*

Nextup run 
```
pio device monitor
```

to check the Serial Activity. Additionally, since the esp32 is connected you can connect your phone its wifi and send text messages.
To upload your code to the esp32 and run it, use these commands:
```
pio run -t upload 
pio device monitor
```

The project is basically done now. Check out README2.md to add more functionalities!
