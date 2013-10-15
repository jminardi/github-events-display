#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

#define PIN 6
#define WIDTH 16
#define HEIGHT 8

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//Adafruit_NeoPixel strip = Adafruit_NeoPixel(WIDTH*HEIGHT, PIN, NEO_GRB + NEO_KHZ800);

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(16, 8, PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
  NEO_MATRIX_ROWS    + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800);
 
//defining the Pins for TX and RX serial communication
SoftwareSerial electricimpSerial = SoftwareSerial(9,10);

char character;
String content;

char eventSeparator = ',';
char itemSeparator = '.';
String lastActor;
String first = "^";
String last = "$"; 

byte ghEvents[30];

void setup() {
    Serial.begin(9600);
    electricimpSerial.begin(9600);
    
    electricimpSerial.print("l");  //load giithub data
    delay(500);
    electricimpSerial.print("c");  //prepare first chunk
    
    matrix.begin();
    matrix.setTextWrap(false);
    matrix.setBrightness(255);
 }

void loop() {
  while(electricimpSerial.available() > 0) {
     character = electricimpSerial.read();
     content += character;
  }
  if (character == '$') {
    electricimpSerial.print("l");
    delay(500);
    Serial.println(content);
    processContent();
    content = "";
  }
  electricimpSerial.print("c");
  delay(500);
 }
 
void processContent() {
  if (!content.startsWith(first) || !content.endsWith(last))
    return;  // bail if content string is invalid
    
  int start_idx = 0;
  int end_idx = 0;
  int loop_idx = 0;
  while (end_idx != -1) {
    
    if (loop_idx > 0) {
      String sub = content.substring(start_idx+1, end_idx);      
      int item_break = sub.indexOf(itemSeparator);
      String actor = sub.substring(0, item_break);
      String event = sub.substring(item_break+1);
      Serial.println(actor + "***" + event);
      
      byte x = (loop_idx-1) % (WIDTH/2);
      byte y = (loop_idx-1) / (WIDTH/2);
      
      if (loop_idx-1 < sizeof(ghEvents))
        ghEvents[loop_idx-1] = (byte)event[0];
      if (loop_idx == 1) lastActor = actor;
    }
    
    start_idx = end_idx;
    end_idx = content.indexOf(eventSeparator, end_idx+1);
    loop_idx++;

  }
  
  drawText();
}

void drawText() {
  
  for (int i; i < 100; i++) {
    drawData();  
    matrix.setCursor(WIDTH-i, 0);
    matrix.print(lastActor);
    matrix.setTextColor(matrix.Color(255, 255, 255));
    matrix.show();
    delay(70);
  }
}

void drawData() {
  int len = sizeof(ghEvents);
  matrix.fillScreen((ghEvents[0]*10)<<8);
  for (int i = 0; i < len; i++) {
    byte val = ghEvents[i];
    byte x = (i+2) % (WIDTH/2);
    byte y = (i+2) / (WIDTH/2);
    set2x2(x, y, val);
  }
  matrix.drawPixel(15, 0, matrix.Color(0, 255, 0));
}

void set2x2(byte x, byte y, byte e) {
  x = 2*x;
  y = 2*y;
  matrix.drawRect(x, y, 2, 2, (e*10)<<8); 
}
