
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>

#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW
#define MAX_DEVICES   4
#define CS_PIN        5
#define WIDTH         32
#define HEIGHT        8

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);


String message = "ARWQ HI! 123 :))";
bool messageActive = true; 

WebServer server(80);


void handleRoot() {
  String html = "<h1>Car Display</h1>"
                "<form action='/submit' method='POST'>"
                "<input type='text' name='msg' placeholder='Enter message'>"
                "<button type='submit'>Send</button>"
                "</form>";
  server.send(200, "text/html", html);
}

void handleSubmit() {
  String newMessage = server.arg("msg");
  Serial.print("Received: ");
  Serial.println(newMessage);
  
  message = newMessage;       // update the global!
  scrollPos = 0;              // reset scroll so new message starts from the right
  
  server.send(200, "text/html", "<h1>Got it!</h1><a href='/'>Back</a>");
}

void setPixel(int x, int y, bool on) {
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
  int block = (WIDTH - 1 - x) / 8;
  int a = 7 - (x % 8);
  int i = block * 8 + y;
  mx.setPoint(a, i, on);
}

int drawLetter(char c, int xOffset) {
  uint8_t buf[8];
  uint8_t width = mx.getChar(c, 8, buf);
  for (int col = 0; col < width; col++) {
    for (int row = 0; row < HEIGHT; row++) {
      if (buf[col] & (1 << row)) {
        setPixel(col + xOffset, row, true);
      }
    }
  }
  return width;
}

void setup() {
  Serial.begin(115200);
  if (WiFi.softAP("CarDisplay", "12345678")) {
  Serial.println("AP started successfully");
} else {
  Serial.println("AP failed to start");
}

IPAddress ip = WiFi.softAPIP();
Serial.print("IP address: ");
Serial.println(ip);

server.on("/", handleRoot);
server.on("/submit", handleSubmit);
server.begin();      
Serial.println("Web server started.");
  mx.begin();
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);  // batch updates
  mx.clear();
}


void loop() {
  server.handleClient();
  mx.clear();
  int xCursor = WIDTH - scrollPos;
  for (int i = 0; i < message.length(); i++) {
    int width = drawLetter(message[i], xCursor);
    xCursor += width + 1;
  }
  mx.update();
  delay(80);
  scrollPos++;
  if (scrollPos > WIDTH + (message.length() * 7)) {
    scrollPos = 0;
  }
}
