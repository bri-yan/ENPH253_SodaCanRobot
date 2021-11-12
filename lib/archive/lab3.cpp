#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "Arduino.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 	-1 // This display does not have a reset pin accessible
#define MOTOR_A PA_1
#define MOTOR_B PA_2
#define MOTORFREQ 1000
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(1000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.display();
  pinMode(MOTOR_A, OUTPUT); 
  pinMode(MOTOR_B, OUTPUT);
}

void loop() {
  //Main loop
  pwm_start(MOTOR_A,MOTORFREQ,2000,RESOLUTION_12B_COMPARE_FORMAT);
  pwm_start(MOTOR_B,MOTORFREQ,0,RESOLUTION_12B_COMPARE_FORMAT);
};


