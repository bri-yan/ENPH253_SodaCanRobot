#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "Arduino.h"

// Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 	-1 // This display does not have a reset pin accessible
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Motors
#define BASESPEED 1100
#define MOTORFREQ 1000
#define MOTOR_RF PA_0 // Right motor, forwards input
#define MOTOR_RB PA_1 // Right motor, backwards input
#define MOTOR_LF PA_2 // Right motor, forwards input
#define MOTOR_LB PA_3 // Right motor, backwards input

// Sensors
#define REFLECTANCE_L PA4
#define REFLECTANCE_R PA5

// Functions
void set_motor(int value, PinName forward, PinName backward);
int get_error(int left, int right);
int slope();

// Variables
#define KP 1000
#define KI 0
#define KD 100
#define IMAX 1000
static int error, last_error = 0;
static int current_state = 0, last_state = 0;
static long start, mid, end; // m: time in previous error state, n: time in current error state
static bool last_left;
static int left_sensor, right_sensor;
static double p, i, d, g;

void setup() {
  Serial.begin(9600);

  // Motor setup
  pinMode(MOTOR_LF, OUTPUT); 
  pinMode(MOTOR_LB, OUTPUT);

  // Sensor setup
  pinMode(REFLECTANCE_L, INPUT_PULLUP);
  pinMode(REFLECTANCE_R, INPUT_PULLUP);
  start = millis(), mid = millis(), end = millis();

  // Display setup
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(500);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.display();
}

void loop() {
  left_sensor = digitalRead(REFLECTANCE_L);
  right_sensor = digitalRead(REFLECTANCE_R);
  error = get_error(left_sensor, right_sensor);
  p = KP * error;
  d = KD * slope();
  i = KI * error + i;
  if (i > IMAX) {
    i = IMAX;
  } else if (i < -IMAX) {
    i = -IMAX;
  }
  g = p + i + d;
  if (g < 0) {
    set_motor(BASESPEED + (int) g, MOTOR_RF, MOTOR_RB);
    set_motor(BASESPEED - (int) g, MOTOR_LF, MOTOR_LB);
  } else if (g > 0) {
    set_motor(BASESPEED + (int) g, MOTOR_RF, MOTOR_RB);
    set_motor(BASESPEED - (int) g, MOTOR_LF, MOTOR_LB);
  } else {
    set_motor(BASESPEED, MOTOR_RF, MOTOR_RB);
    set_motor(BASESPEED, MOTOR_LF, MOTOR_LB);
  }
  
  display.print("left_sensor: "); display.print(left_sensor); display.print("\n");
  display.print("right_sensor: "); display.print(right_sensor); display.print("\n");
  display.print("P: "); display.print(p); display.print("\n");
  display.print("I: "); display.print(i); display.print("\n");
  display.print("D: "); display.print(d); display.print("\n");
  display.print("G: "); display.print(g); display.print("\n");
  display.print("error: "); display.print(error); display.print("\n");
  display.print("last_error: "); display.print(last_error); display.print("\n");
  display.display();
  display.setCursor(0,0);
  display.clearDisplay();
}

void set_motor(int value, PinName forward, PinName backward) {
  int offset = 1250;
  if (value > 0) { // Motor turns forwards, backwards value set to 0
    pwm_start(forward, MOTORFREQ, value + offset, RESOLUTION_12B_COMPARE_FORMAT);
    pwm_start(backward, MOTORFREQ, 0, RESOLUTION_12B_COMPARE_FORMAT);
  } else if (value < 0) {  // Motor turns backwards, forwards value set to 0
    pwm_start(forward, MOTORFREQ, 0, RESOLUTION_12B_COMPARE_FORMAT);
    pwm_start(backward, MOTORFREQ, value + offset, RESOLUTION_12B_COMPARE_FORMAT);
  } else { // Motor stops
    pwm_start(forward, MOTORFREQ, 0, RESOLUTION_12B_COMPARE_FORMAT);
    pwm_start(backward, MOTORFREQ, 0, RESOLUTION_12B_COMPARE_FORMAT);
  }
}

int get_error(int left, int right) {
  if (left == 1 && right == 1) { // Both sensors on tape
    return 0;
  } else if (left == 1 && right == 0) { // Left sensor on tape, right sensor off
    return 1;
    last_left = true;
  } else if (left == 0 && right == 1) { // Right sensor on tape, left sensor off
    return -1;
    last_left = false;
  } else if (left == 0 && right == 0) {
    if (last_left) {
      return 3;
    } else {
      return -3;
    }
  }
}

int slope() {
  if (error == last_error) {
    end = millis();
  } else {
    start = mid;
    mid = millis();
  }
  double time = (double) end - (double) start;
  return (double) (error - last_error) / time;
}





