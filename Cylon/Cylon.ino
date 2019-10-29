#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 300 
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
boolean isAutoBrightness = true;

int i = 0;
int skipAtEnds = 12;
static uint8_t hue = 0;

boolean buttonState = HIGH;
boolean prevButtonState = HIGH;
uint8_t hitCount = 0;
uint8_t briScale = 128;
uint8_t briStep = 32;
unsigned long lastStatusSwitch = 999999;

void setup() { 
	// Serial.begin(9600);
	// Serial.println("beggining!");

	// initialize the pushbutton pin as an input:
	pinMode(BUTTON_PIN, INPUT_PULLUP);

	LEDS.addLeds<WS2812,DATA_PIN,RGB>(leds,NUM_LEDS);
	LEDS.setBrightness(BRIGHTNESS);
}

void fadeall() {  
  for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(252); }
}



void loop() { 
 handleButton();

  if (briScale > briStep) {
    leds[i] = CHSV(hue++, 255, dim8_raw( briScale ));
  }
  FastLED.show(); 
  fadeall();

  if (i == NUM_LEDS - 1) {
    forward = false;
	endReached();
  } else if (i == 0) {
    forward = true;
	endReached();
    // increase brightness at every complete cycle
    if(isAutoBrightness) brightCycle();
  }
 
  if (forward) {
    i++;
  } else {
    i--;
  }
  
  delay(delayTime);

}

void endReached() {
	for (int j = 0; j < skipAtEnds; j++) {
		FastLED.show(); 	
		fadeall();
		delay(delayTime);
	}
}

void brightCycle() {
   briScale = briScale + briStep;
   briScale = constrain(briScale, briStep, 255);
}

void handleButton() {
  buttonState = digitalRead(BUTTON_PIN);
  
  // event of button pressed or released
  if (buttonState != prevButtonState) {	  
   	hitCount++;
	// button released
	if (hitCount % 2 == 0 && millis() - lastStatusSwitch < 1000) {
		isAutoBrightness = !isAutoBrightness;
	}
	lastStatusSwitch = millis(); 
  }

  if (buttonState == LOW && millis()-lastStatusSwitch >= 1000) {
    // increase or decrease brightness
    if (hitCount % 4 == 1 && briScale < 255) {
      briScale++;
    } else if (hitCount % 4 == 3 && briScale > 0) {  
      briScale--;
    }
  }
    
  prevButtonState = buttonState;
}
