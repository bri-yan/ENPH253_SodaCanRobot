#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "Arduino.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 	-1 // This display does not have a reset pin accessible
#define MOTOR_A PA_1 // Motor A pin
#define MOTOR_B PA_2 // Motor B pin
#define MOTORFREQ 1000
#define OBJECT_SENSOR PA0 // Reflectance sensor pin
#define TARGET PA3 // Potentiometer pin
#define KP 3.75 // Proportional constant 1.8
#define KD 3.75 // Derivative constant 1.1
#define KI 0 // Integration constant
#define IMAX 1000 // Integration Maximum
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
static double last_error = 0;

void setup() {
  pinMode(MOTOR_A, OUTPUT); 
  pinMode(MOTOR_B, OUTPUT);
  pinMode(OBJECT_SENSOR, INPUT); 
  pinMode(TARGET, INPUT);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(1000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.display();
}

void loop() {
  // Main loop

  // Define and display reflectance and target reflectance
  double reflectance = analogRead(OBJECT_SENSOR);
  double target = analogRead(TARGET);
  display.print("reflectance: "); display.print(reflectance); display.print("\n");
  display.print("target: "); display.print(target); display.print("\n");

  // Define and display PID
  double error = target - reflectance;
  double p = KP * error;
  double d = KD * (error - last_error);
  double i = KI * error + i;
  if (i > IMAX) {
    i = IMAX;
  } else if (i < -IMAX) {
    i = -IMAX;
  }
  double g = p + i + d;
  if (g >= 0) {
    pwm_start(MOTOR_A,MOTORFREQ,0,RESOLUTION_12B_COMPARE_FORMAT);
    pwm_start(MOTOR_B,MOTORFREQ,g,RESOLUTION_12B_COMPARE_FORMAT);
  } else {
    pwm_start(MOTOR_A,MOTORFREQ,-g,RESOLUTION_12B_COMPARE_FORMAT);
    pwm_start(MOTOR_B,MOTORFREQ,0,RESOLUTION_12B_COMPARE_FORMAT);
  }
  last_error = error;

  display.print("P: "); display.print(p); display.print("\n");
  display.print("I: "); display.print(i); display.print("\n");
  display.print("D: "); display.print(d); display.print("\n");
  display.print("G: "); display.print(g); display.print("\n");
  display.print("error: "); display.print(error); display.print("\n");
  display.print("last_error: "); display.print(last_error); display.print("\n");
  display.display();
  display.setCursor(0,0);
  display.clearDisplay();
};


