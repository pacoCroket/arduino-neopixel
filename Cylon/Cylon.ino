#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 150 
#define BRIGHTNESS 150

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806, define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 3
#define BUTTON_PIN 2
#define CLOCK_PIN 13

// Define the array of leds
CRGB leds[NUM_LEDS];

int outFrameSpace = 2; // virtual LEDs to add time between waves of color from the sides
int delayTime = 28;
boolean forward = true;

int i = 0;
static uint8_t hue = 0;

boolean buttonState = HIGH;
boolean prevButtonState = HIGH;
uint8_t hitCount = 0;
uint8_t briScale = 128;
uint8_t briStep = 32;

void setup() { 
  Serial.begin(9600);
  Serial.println("beggining!");
	LEDS.addLeds<WS2812,DATA_PIN,RGB>(leds,NUM_LEDS);
	LEDS.setBrightness(BRIGHTNESS);
}

void fadeall() {  
  for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(252); }
}



void loop() { 
//  handleButton();

  if (briScale > briStep) {
    leds[i] = CHSV(hue++, 255, dim8_raw( briScale ));
  }
  FastLED.show(); 
  fadeall();

  if (i == NUM_LEDS - 1) {
    forward = false;
  } else if (i == 0) {
    forward = true;
    // increase brightness at every complete cycle
    brightCycle();
  }
 
  if (forward) {
    i++;
  } else {
    i--;
  }
  
  delay(delayTime);

}

void brightCycle() {
   briScale = briScale + briStep;
   briScale = constrain(briScale, briStep, 255);
}

void handleButton() {
  buttonState = digitalRead(BUTTON_PIN);
  Serial.println(buttonState);

  if (buttonState == LOW) {    
//     briScale = briScale + briStep;
  }
  
  // event of button pressed or released
  if (buttonState != prevButtonState && hitCount == 1) {
    Serial.print("new button state :");
    Serial.println(buttonState);
   briScale = briScale + briStep;
   hitCount = (hitCount + 1 ) % 2;
  }
    
  prevButtonState = buttonState;
}
