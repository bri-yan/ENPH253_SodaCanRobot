#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "Arduino.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 	-1 // This display does not have a reset pin accessible
#define OBJECT_SENSOR PA0
#define DETECT_THRESHOLD PA1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

volatile int i = 0;
void handle_interrupt();

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
 
  // Displays Adafruit logo by default. call clearDisplay immediately if you don't want this.
  display.display();

  // Displays "Hello world!" on the screen
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.display();
  pinMode(OBJECT_SENSOR, INPUT); 
  pinMode(DETECT_THRESHOLD, INPUT);
}

void loop() {
  double reflectance = analogRead(OBJECT_SENSOR);
  double threshold = analogRead(DETECT_THRESHOLD);
  if (reflectance > threshold) {
    display.println("No object here");
  } else {
    display.println("Object detected");
  }
  display.print("reflectance: ");
  display.print(reflectance);
  display.print("\n");
  display.print("threshold: ");
  display.print(threshold);
  display.print("\n");
  display.display();
  delay(100);
  display.setCursor(0,0);
  
  display.clearDisplay();
};


