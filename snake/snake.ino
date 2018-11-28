#include <SPI.h>

#include <WiFi101.h>

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

CRGB ledMatrix[16][16]; // grid of leds
vector<int> x; // x values of each led in the snake, in order
vector<int> y; // y values of each led in the snake, in order

vector<int> unoccupiedX;
vector<int> unoccupiedY;

char currentDirection = 'r';
int currentRow = 0;
int currentColumn = 2;

int randomIndex = 0;

CRGB food = ledMatrix[0][8];
int foodX = 0;
int foodY = 8;
bool snakeHasEaten = false;

WiFiServer server(80);
int status = WL_IDLE_STATUS;
char ssid[] = "CSE321_SNAKE";
//char password[] = "kurtandpaul";

void setup() {

  initializeLedMatrix();
  initializeVectors();
  ledMatrix[0][8] = CRGB::Green; // initial spot for the food
  FastLED.show();

  WiFi.setPins(8,7,4,2); // necessary for the Feather board

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  if (WiFi.status() == WL_NO_SHIELD) {while(true);} // check for presence of WiFi

  status = WiFi.beginAP(ssid); // create network
  if (status != WL_AP_LISTENING) {while(true);} // if creation of access point failed

  delay(10000); // wait for connection
  server.begin(); // start server when connected
  
}

void loop() {

  WiFiClient client = server.available(); // check for incoming clients
  if(client){ // if there is a client
    if(client.available()){ // if there is something to read from the client
      if((char)client.read() != 0x00){currentDirection = (char)client.read();}
       // change the direction of the snake based on the remote input
    }
  }

  // update the position of where the head will be based on the direction of the snake
  if(currentDirection == 'u'){currentRow--;}
  if(currentDirection == 'd'){currentRow++;} // up and down are inverted because of the way the board was wired
  if(currentDirection == 'l'){currentColumn--;}
  if(currentDirection == 'r'){currentColumn++;}

  if(x.size() == 256){while(true);} // if the snake is as big as the entire game board

  if(currentRow < 0 || currentRow > 15 || currentColumn < 0 || currentColumn > 15){turnOffAllLeds();} // if the snake hits the boundaries
  checkIfSnakeHitsItself(); // if the snake hits itself
  
  // add the the led in front of the snake to the snake's head
  x.insert(x.begin(), currentRow);
  y.insert(y.begin(), currentColumn);

  ledMatrix[x.at(0)][y.at(0)] = CRGB::Blue; // turn on the led that was just added to the head of the snake
  FastLED.show();

  unoccupiedX.erase(remove(unoccupiedX.begin(), unoccupiedX.end(), currentRow), unoccupiedX.end()); // remove the led that was just added to the head of the snake from the list of unoccupied leds
  unoccupiedY.erase(remove(unoccupiedY.begin(), unoccupiedY.end(), currentColumn), unoccupiedY.end());

  if(currentRow == foodX && currentColumn == foodY){generateFood();} // to grow the snake when it touches the food
  
  if(!snakeHasEaten){ // if the snake doesn't come in contact with the food (which is most of the time)
    
    ledMatrix[x.back()][y.back()] = CRGB::Black; // turn off the led that will be removed from the back of the snake
    FastLED.show();
    
    unoccupiedX.push_back(x.back()); // at the back led that will be removed to the list of unoccupied leds
    unoccupiedY.push_back(y.back());
    
    x.erase( x.begin() + (x.size() - 1) ); // take out the back led of the snake
    y.erase( y.begin() + (y.size() - 1) );
    
  }

  snakeHasEaten = false;
  int randomIndex = random(0, unoccupiedX.size() - 1);
  delay(200);

} // end of main loop

void generateFood(){ // getting a random led that is currently not lit up

  snakeHasEaten = true;
  
  foodX = unoccupiedX.at(randomIndex);
  foodY = unoccupiedY.at(randomIndex);

  ledMatrix[foodX][foodY] = CRGB::Green;
  FastLED.show();
  
}

void checkIfSnakeHitsItself(){

  for(int i = 0; i < x.size(); i++){ // look through all leds of the snake

    if(currentRow == x.at(i) && currentColumn == y.at(i)){turnOffAllLeds();} // if the snake's head is already part of the snake
    
  }
  
}

void turnOffAllLeds(){

    ledMatrix[foodX][foodY] = CRGB::Black; // turn off food led
    FastLED.show();
    
    for(int i = 0; i < x.size(); i++){ // turn off snake leds
      
      ledMatrix[x.at(i)][y.at(i)] = CRGB::Black;
      FastLED.show();
      
      }
      
    while(true);
  
}

void initializeVectors(){

  x.push_back(0);
  y.push_back(2);

  x.push_back(0);
  y.push_back(1);

  x.push_back(0);
  y.push_back(0);
  
  ledMatrix[0][2] = CRGB::Blue; // turn the snake leds on
  ledMatrix[0][1] = CRGB::Blue;
  ledMatrix[0][0] = CRGB::Blue;

  for(int row = 0; row < 16; row++){ // put the rest of the leds into the list of unoccupied leds
    for(int col = 0; col < 16; col++){
      if( !(row == 0 && col <= 2) ){ // if the led is not initially part of the snake
        
        unoccupiedX.push_back(row);
        unoccupiedY.push_back(col);
        
        ledMatrix[row][col] = CRGB::Black;
        FastLED.show();
        
      }
    }
  }
  
}

void initializeLedMatrix(){

  FastLED.addLeds<NEOPIXEL, 14>(ledMatrix[0], 16); // initialize each strip of leds
  FastLED.addLeds<NEOPIXEL, 15>(ledMatrix[1], 16);
  FastLED.addLeds<NEOPIXEL, 16>(ledMatrix[2], 16);
  FastLED.addLeds<NEOPIXEL, 17>(ledMatrix[3], 16);
  FastLED.addLeds<NEOPIXEL, 18>(ledMatrix[4], 16);
  FastLED.addLeds<NEOPIXEL, 19>(ledMatrix[5], 16);
  FastLED.addLeds<NEOPIXEL, 0>(ledMatrix[6], 16);
  FastLED.addLeds<NEOPIXEL, 1>(ledMatrix[7], 16);
  FastLED.addLeds<NEOPIXEL, 20>(ledMatrix[8], 16);
  FastLED.addLeds<NEOPIXEL, 13>(ledMatrix[9], 16);
  FastLED.addLeds<NEOPIXEL, 12>(ledMatrix[10], 16);
  FastLED.addLeds<NEOPIXEL, 11>(ledMatrix[11], 16);
  FastLED.addLeds<NEOPIXEL, 10>(ledMatrix[12], 16);
  FastLED.addLeds<NEOPIXEL, 9>(ledMatrix[13], 16);
  FastLED.addLeds<NEOPIXEL, 6>(ledMatrix[14], 16);
  FastLED.addLeds<NEOPIXEL, 5>(ledMatrix[15], 16);

}
