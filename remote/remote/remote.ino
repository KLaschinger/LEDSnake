#include <ArduinoLowPower.h>
#include <RTCZero.h>
#include <WiFi101.h>

// Uncomment this line for serial debugging
// #define SERIAL_DEBUG

// Remote button functionality
#define BUT_U 14
#define BUT_L 15
#define BUT_D 16
#define BUT_R 17

#define DIR_UP 'u'
#define DIR_LEFT 'l'
#define DIR_DOWN 'd'
#define DIR_RIGHT 'r'
#define DIR_NONE 4

volatile char dir = DIR_NONE;

// Remote Control Pin Definitions
#define BUT_CTRL 12
#define LED_RED 10
#define LED_GREEN 11

uint8_t ctrl_color = LED_GREEN;
bool ctrl_onoff = false;
volatile bool deep_sleep = false;

// Battery level logic
#define VBAT_PIN A7

#define VBAT_MIN_LIMIT 500
#define VBAT_REFRESH 500 //Number of milliseconds between color refreshes

// Restart things
RTCZero rtc;

// Wifi things
WiFiClient client;
IPAddress server(192,168,1,1);
const char ssid[] = "CSE321_SNAKE";
const char pass[] = "kurtandpaul";
uint8_t wifi_status = WL_IDLE_STATUS;

// Function definitions
inline void read_battery();

void wifi_startup();
void wifi_shutdown();
bool wifi_isconnected();
bool wifi_transmit();
void wifi_wait_for_connection();

void sleep_cpu();

void isr_up();
void isr_left();
void isr_down();
void isr_right();
void isr_shutdown();
void isr_wake();
void isr_dummy();

/*
 * Setup pins and logic for the remote to work
 */
void setup() {
  // Setup pins for button input
  // Buttons are set as exteral interrupts. Debouncing is done through hardware.
  attachInterrupt(BUT_U, isr_up, FALLING);
  attachInterrupt(BUT_L, isr_left, FALLING);
  attachInterrupt(BUT_D, isr_down, FALLING);
  attachInterrupt(BUT_R, isr_right, FALLING);
  attachInterrupt(BUT_CTRL, isr_shutdown, FALLING);

  // Setup pins for LED output
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);

  // Reset some variables
  deep_sleep = false;

  // Start sleep functionality
  // Something is funky with with ArduinoLowPower library where sleeping automatically returns
  // Ref here: https://github.com/arduino-libraries/ArduinoLowPower/issues/5
  rtc.begin();
  LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP,isr_dummy,CHANGE);

#ifdef SERIAL_DEBUG
  Serial.begin(9600);
  while(!Serial);
  while(true) {
    if (Serial.read() == 'a') {
      break;
    }
  }
#endif

  // Gets the initial battery reading to show
  read_battery();

  // Begin wifi
  wifi_startup();

  // Wait for the connection to be established
  wifi_wait_for_connection();
}

void loop() {
  // Loops sleeping the CPU forever if setup to sleep
  if (deep_sleep) {
    sleep_cpu();
  } else {
    LowPower.idle(VBAT_REFRESH);

    read_battery();

    // Sets the interrupt color
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(ctrl_color, HIGH);

    if (dir != DIR_NONE) wifi_transmit();
  }
}

/*
 * Helper function to read the battery voltage and set the LED color to display
 */
inline void
read_battery() {
  ctrl_color = (analogRead(VBAT_PIN) < VBAT_MIN_LIMIT) ? LED_RED : LED_GREEN;

#ifdef SERIAL_DEBUG
  float measuredvbat = analogRead(VBAT_PIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  Serial.print("VBat: ");
  Serial.print(measuredvbat);
  Serial.print("Val: ");
  Serial.println(analogRead(VBAT_PIN));
#endif
}

/*
 * Activates wifi after waking up (or starting up)
 */
void wifi_startup() {
  
#ifdef SERIAL_DEBUG
  Serial.println("Starting wifi");
#endif

  WiFi.setPins(8,7,4,2);

  if (WiFi.status() == WL_NO_SHIELD) {
#ifdef SERIAL_DEBUG
    Serial.println("No Wifi Shield:  Halting execution");
#endif

    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, HIGH);
    while(true);
  }

  while (wifi_status != WL_CONNECTED) {
    wifi_status = WiFi.begin(ssid);
  }
}

/*
 * Activates wifi after waking up (or starting up)
 */
void wifi_shutdown() {
  // TODO - Shutdown up the wifi

#ifdef SERIAL_DEBUG
  Serial.println("Shutting down wifi");
#endif
}

/*
 * Checks if there is an active wifi connection to the board
 * 
 * returns - If the connection with the board is active
 */
bool wifi_isconnected(){
  // TODO - Check if wifi in connected
#ifdef SERIAL_DEBUG
  Serial.println("Checking Wifi Connection");
#endif

  return client.connect(server, 80);
}

/*
 * Transmits the direction for the snake to move to the board.
 * Resets the direction back to NONE
 * 
 * returns  - If the transmission succeeds.
 *            Failiure conditions TDB (likely from disconnecting to the board)
 */
bool wifi_transmit() {
  // TODO - Transmit signal over wifi
#ifdef SERIAL_DEBUG
  Serial.print("Transmitting over wifi");
  Serial.println(dir);
#endif

  if (dir != DIR_NONE) {
    client.println(dir);
  }

  dir = DIR_NONE;
  return true;
}

/*
 * Waits for a connection to be established
 * Blinks the LED while waiting
 */
void
wifi_wait_for_connection(){
  digitalWrite(LED_GREEN, LOW);

  // Blink the LED until the connection is established
  while (!wifi_isconnected()) {
    ctrl_onoff = ~ctrl_onoff;

    if (ctrl_onoff) digitalWrite(ctrl_color, HIGH);
    else digitalWrite(ctrl_color, LOW);

    delay(500);
  }

  digitalWrite(LED_RED, LOW);
}

/*
 * Functionality of putting the CPU to sleep
 */
void
sleep_cpu(){
  
#ifdef SERIAL_DEBUG
  Serial.println("Putting the CPU to sleep");
#endif

  // Detach interrupts from the pins
  detachInterrupt(BUT_U);
  detachInterrupt(BUT_L);
  detachInterrupt(BUT_D);
  detachInterrupt(BUT_R);
  detachInterrupt(BUT_CTRL);

  // Turn off LED's
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);

  // Disable the interrupt from battery polling
  rtc.begin();
  rtc.disableAlarm();

  // Shutdown the wifi connection
  wifi_shutdown();

  while (digitalRead(BUT_CTRL) == LOW);

  delay(10);

  // Attach wakeup interrupt
  attachInterrupt(BUT_CTRL, isr_wake, LOW);

#ifdef SERIAL_DEBUG
  Serial.end();
#endif

  // Sleep the CPU
  LowPower.deepSleep();
}

/*
 * Set of ISR's to be called whenever a button is pressed
 */
void
isr_up() {
  dir = DIR_UP;
}

void
isr_left() {
  dir = DIR_LEFT;
}

void
isr_down() {
  dir = DIR_DOWN;
}

void
isr_right() {
  dir = DIR_RIGHT;
}

/*
 * Interrupt called when the ctrl button is pressed to sleep
 */
void
isr_shutdown() {
  
#ifdef SERIAL_DEBUG
  Serial.println("Trigger sleep");
#endif

  deep_sleep = true;
}

/*
 * Interrupt called when the ctrl button is pressed to wake up
 */
void
isr_wake() {
  
#ifdef SERIAL_DEBUG
  Serial.begin(9600);
  Serial.println("Waking the CPU");
  Serial.end();
#endif
  // setup();
  // Waking up the processor will completely reset the program
  // This allows for a single button to act like a switch
  NVIC_SystemReset();
}

/*
 * Dummy isr for waking the processor
 */
void
isr_dummy() {
  return;
}

