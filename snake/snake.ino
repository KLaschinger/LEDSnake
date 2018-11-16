#include <ArduinoSTL.h>

#include <bitswap.h>
#include <chipsets.h>
#include <color.h>
#include <colorpalettes.h>
#include <colorutils.h>
#include <controller.h>
#include <cpp_compat.h>
#include <dmx.h>
#include <FastLED.h>
#include <fastled_config.h>
#include <fastled_delay.h>
#include <fastled_progmem.h>
#include <fastpin.h>
#include <fastspi.h>
#include <fastspi_bitbang.h>
#include <fastspi_dma.h>
#include <fastspi_nop.h>
#include <fastspi_ref.h>
#include <fastspi_types.h>
#include <hsv2rgb.h>
#include <led_sysdefs.h>
#include <lib8tion.h>
#include <noise.h>
#include <pixelset.h>
#include <pixeltypes.h>
#include <platforms.h>
#include <power_mgt.h>

using namespace std;

CRGB ledMatrix[16][16];
vector<CRGB> snake(3);
vector<CRGB> unoccupiedLeds(16*16-3);

char currentDirection;
int currentRow;
int currentColumn;

CRGB food;
bool snakeHasEaten;

void setup() {

  currentDirection = 'r';
  snakeHasEaten = false;
  
  currentRow = 0;
  currentColumn = 2;
  
  initializeLedMatrix();
  initializeVectors();
  generateFood();
  
}

void loop() {

  if(currentDirection == 'u'){currentRow++;} // update the position of where the head will be based on the direction of the snake
  if(currentDirection == 'd'){currentRow--;}
  if(currentDirection == 'l'){currentColumn--;}
  if(currentDirection == 'r'){currentColumn++;}
  
  snake.insert(snake.begin(), ledMatrix[currentRow][currentColumn]); // add the the led in front of the snake to the snake's head
  unoccupiedLeds.erase(remove(unoccupiedLeds.begin(), unoccupiedLeds.end(), snake.at(0)), unoccupiedLeds.end()); // remove the led of the snake's new head from the vector of unoccupied leds
  snake.at(0) = CRGB::Blue; // turn on the led that was just added to the front

  if(snake.at(0) == food){generateFood();} // to grow the snake
  
  if(!snakeHasEaten){
    
    unoccupiedLeds.push_back(snake.back()); // add the led that was just removed to the list of unoccupied leds
    snake.erase( snake.begin() + (snake.size() - 1) ); // take out the back led of the snake
    snake.back() = CRGB::Black; // turn off the led that was just removed from the back
    
  }
  
  snakeHasEaten = false;
  FastLED.show();

}

void initializeVectors(){

  snake.push_back(ledMatrix[0][2]); // put the first 3 leds in the snake vector
  snake.push_back(ledMatrix[0][1]);
  snake.push_back(ledMatrix[0][0]);

  ledMatrix[0][2] = CRGB::Blue; // turn the snake leds on
  ledMatrix[0][1] = CRGB::Blue;
  ledMatrix[0][0] = CRGB::Blue;

  for(int row = 0; row <= 16; row++){ // put the rest of the leds into the vector of unoccupied leds
    for(int col = 0; col <= 16; col++){
      if( !(row == 0 && col <= 2) ){
        unoccupiedLeds.push_back(ledMatrix[row][col]);
        ledMatrix[row][col] = CRGB::Black;
      }
    }
  }
  
}

void generateFood(){

  snakeHasEaten = true;
  int randomIndex = rand() % unoccupiedLeds.size();
  food = unoccupiedLeds.at(randomIndex);
  food = CRGB::Green;
  
}

void initializeLedMatrix(){

  CRGB leds0[16]; // allocate memory for each led strip
  memcpy(ledMatrix[0], leds0, 16); // copy them into the array of arrays
  CRGB leds1[16];
  memcpy(ledMatrix[1], leds1, 16);
  CRGB leds2[16];
  memcpy(ledMatrix[2], leds2, 16);
  CRGB leds3[16];
  memcpy(ledMatrix[3], leds3, 16);
  CRGB leds4[16];
  memcpy(ledMatrix[4], leds4, 16);
  CRGB leds5[16];
  memcpy(ledMatrix[5], leds5, 16);
  CRGB leds6[16];
  memcpy(ledMatrix[6], leds6, 16);
  CRGB leds7[16];
  memcpy(ledMatrix[7], leds7, 16);
  CRGB leds8[16];
  memcpy(ledMatrix[8], leds8, 16);
  CRGB leds9[16];
  memcpy(ledMatrix[9], leds9, 16);
  CRGB leds10[16];
  memcpy(ledMatrix[10], leds10, 16);
  CRGB leds11[16];
  memcpy(ledMatrix[11], leds11, 16);
  CRGB leds12[16];
  memcpy(ledMatrix[12], leds12, 16);
  CRGB leds13[16];
  memcpy(ledMatrix[13], leds13, 16);
  CRGB leds14[16];
  memcpy(ledMatrix[14], leds14, 16);
  CRGB leds15[16];
  memcpy(ledMatrix[15], leds15, 16);

  FastLED.addLeds<NEOPIXEL, 1>(ledMatrix[0], 16); // initialize each strip of leds
  FastLED.addLeds<NEOPIXEL, 2>(ledMatrix[1], 16);
  FastLED.addLeds<NEOPIXEL, 3>(ledMatrix[2], 16);
  FastLED.addLeds<NEOPIXEL, 4>(ledMatrix[3], 16);
  FastLED.addLeds<NEOPIXEL, 5>(ledMatrix[4], 16);
  FastLED.addLeds<NEOPIXEL, 6>(ledMatrix[5], 16);
  FastLED.addLeds<NEOPIXEL, 7>(ledMatrix[6], 16);
  FastLED.addLeds<NEOPIXEL, 8>(ledMatrix[7], 16);
  FastLED.addLeds<NEOPIXEL, 9>(ledMatrix[8], 16);
  FastLED.addLeds<NEOPIXEL, 10>(ledMatrix[9], 16);
  FastLED.addLeds<NEOPIXEL, 11>(ledMatrix[10], 16);
  FastLED.addLeds<NEOPIXEL, 12>(ledMatrix[11], 16);
  FastLED.addLeds<NEOPIXEL, 13>(ledMatrix[12], 16);
  FastLED.addLeds<NEOPIXEL, 14>(ledMatrix[13], 16);
  FastLED.addLeds<NEOPIXEL, 15>(ledMatrix[14], 16);
  FastLED.addLeds<NEOPIXEL, 16>(ledMatrix[15], 16);

}
