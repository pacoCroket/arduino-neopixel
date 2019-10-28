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
int delayTime = 22;

boolean buttonState = HIGH;
boolean prevButtonState = HIGH;
uint8_t hitCount = 0;
uint8_t briScale = 100;

void setup() { 
//	Serial.begin(57600);
//	Serial.println("resetting");
	LEDS.addLeds<WS2812,DATA_PIN,RGB>(leds,NUM_LEDS);
	LEDS.setBrightness(BRIGHTNESS);
}

void fadeall() {  
//  handleButton();
  for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); }
}

void loop() { 
  handleButton();
  
	static uint8_t hue = 0;
//	Serial.print("x");
	// First slide the led in one direction
	for(int i = 0; i < NUM_LEDS; i++) {
		// Set the i'th led to red 
		leds[i] = CHSV(hue++, 255, dim8_raw( briScale ));
		// Show the leds
		FastLED.show(); 
		// now that we've shown the leds, reset the i'th led to black
		// leds[i] = CRGB::Black;
		fadeall();
		// Wait a little bit before we loop around and do it again
		delay(delayTime);
	}
//	Serial.print("x");

//  for(int i = 0; i < outFrameSpace; i++) {
//    fadeall();
//    FastLED.show();
//    delay(delayTime);
//  }

	// Now go in the other direction.  
	for(int i = (NUM_LEDS)-1; i >= 0; i--) {
		// Set the i'th led to red 
   
		leds[i] = CHSV(hue++, 255, dim8_raw( briScale ));
		// Show the leds
		FastLED.show();
		// now that we've shown the leds, reset the i'th led to black
		// leds[i] = CRGB::Black;
		fadeall();
		// Wait a little bit before we loop around and do it again
		delay(delayTime);
	}

//   for(int i = 0; i < outFrameSpace; i++) {
//    fadeall();
//    FastLED.show();
//    delay(delayTime);
//  }
}



void handleButton() {
  buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == LOW) {    
     briScale = (briScale + 30) % 256;
  }
  
  // event of button pressed or released
//  if (buttonState != prevButtonState) {
//     briScale = (briScale + 50) % 256;
////     hitCount = (hitCount + 1 ) % 4;
//  }
    
  prevButtonState = buttonState;
}
