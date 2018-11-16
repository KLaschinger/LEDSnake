#include "ArduinoLowPower.h"
#include <avr/io.h>
#include <Wire.h>

// Remote button functionality
#define BUT_U 10
#define BUT_L 11
#define BUT_D 12
#define BUT_R 13

// Remote control functionality
// Pin Definitions
#define BUT_CTRL 1
#define LED_RED 2
#define LED_GREEN 3

// Battery level logic
#define VBAT_PIN A7

#define VBAT_MIN_LIMIT 132

#define CTRL_RED 0
#define CTRL_GREEN 1
uint8_t ctrl_color = CTRL_GREEN;

/*
 * Setup pins and logic for the remote to work
 */
void setup() {
  // Setup pins for button input
  pinMode(BUT_U, INPUT);
  pinMode(BUT_L, INPUT);
  pinMode(BUT_D, INPUT);
  pinMode(BUT_R, INPUT);

  // Setup pins for LED output
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:

}

/*
 * Helper function to read the battery voltage and set the LED color to display
 */
void
read_battery() {
  uint8_t volt = analogRead(VBAT_PIN);

  if (volt < VBAT_MIN_LIMIT) {
    ctrl_color = CTRL_RED;
  } else {
    ctrl_color = CTRL_GREEN;
  }
}

/*
 * Function that is called whenever the ctlr button is pressed
 */
void
powerstate_trigger() {
  
}

