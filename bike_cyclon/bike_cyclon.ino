#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 86
#define BRIGHTNESS 80

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806, define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 3
#define BUTTON_PIN 2

// Define the array of leds
CRGB leds[NUM_LEDS];

int8_t delayTime = 26;
boolean forward = true;
boolean isAutoBrightness = false;

int8_t i = 0;
static uint8_t hue = 0;

boolean buttonState = HIGH;
boolean prevButtonState = HIGH;
uint8_t hitCount = 0;
uint8_t briScale = 255;
uint8_t briStep = 8;
unsigned long lastStatusSwitch = 999999;

int8_t spotsOn = 1;
const int8_t maxSpotsOn = 4;

long glow = 0;
double glowRate = 1;

class Spot {
  public: 
    Spot() {
      setPos(random8(NUM_LEDS)); 
      clockwiseSign = random8(2) * 2 - 1; //  random true / false
      changeHue = random8(2) * 2 - 1; //  random true / false
      delayTime = random8(15, 70);
      hue = random8();
      brightness = random8(200, 255);
    };

    void setPos(int8_t pos_) {
      pos = pos_;
    }

    uint8_t getPos() {
      return pos;
    }

    void move() {
      if (clockwiseSign == 1) {
        stepClockwise();
      } else {
        stepCounterClockwise();
      }      
    }

    void stepClockwise() {
      pos = (pos + 1) % NUM_LEDS;
      setLastMove();
    }

    void stepCounterClockwise() {
      if (pos == 0) pos = NUM_LEDS;
      pos--;
      setLastMove();
    }

    void show() {
      leds[pos] = CHSV(hue, 255, dim8_raw( brightness ));
      if (changeHue == 1) hue++;
      // if (pos == 0) brightCycle();
    }

    void setLastMove() {
      lastMove = millis();
    }

    long getLastMove() {
      return lastMove;
    }

    void brightCycle() {
      brightness = brightness + briStep;
      // brightness = constrain(brightness, 100, 255);
}

    int8_t delayTime;
    int8_t hue;
    
  private:
    int8_t pos;
    long lastMove = 0;
    int8_t clockwiseSign;
    int8_t brightness;
    int8_t changeHue;
};

void setup() { 
  random16_set_seed(millis()*digitalRead(BUTTON_PIN));
	// Serial.begin(9600);
	// Serial.println("beggining!");

	// initialize the pushbutton pin as an input:
	pinMode(BUTTON_PIN, INPUT_PULLUP);

	LEDS.addLeds<WS2812B,DATA_PIN,GRB>(leds,NUM_LEDS);
	LEDS.setBrightness(BRIGHTNESS);
}

Spot spots[maxSpotsOn];

void fadeall() {  
  for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(190); }
}

void glowAll() {

  for (int i = 0; i < NUM_LEDS; i++) {
    int8_t bri = map(inoise8(i, glow), 0, 255, 50, 255);
    leds[i] = CHSV( map(i, 0, NUM_LEDS, 0, 255) + glow, 180, dim8_raw( bri ));
  }

  glow += glowRate;
}

void loop() { 
  ChangeSpotsPeriodically();
  showSpots();

  redBackLight();
  whiteFrontLight();
  
  FastLED.show(); 
  FastLED.delay(delayTime);
}

void whiteFrontLight() {
  for(int i = NUM_LEDS-20; i < NUM_LEDS; i++) {
    leds[i] = CRGB::GhostWhite;
  }
}

void redBackLight() {
  for(int i = 0; i < 20; i++) {
    leds[i] = CRGB::Red;
  }
}

void showSpots() {
  for(int i = 0; i < spotsOn; i++){
    spots[i].show();
    
    if(millis() - spots[i].getLastMove() > spots[i].delayTime) spots[i].move();
  }

  fadeall();  
}

void brightCycle() {
   briScale = briScale + briStep;
   briScale = constrain(briScale, briStep, 255);
}

void ChangeSpotsPeriodically()
{
  uint8_t secondHand = (millis() / 1000) % 60;
  static uint8_t lastSecond = 99;

  if (secondHand != lastSecond && secondHand % 30 == 0) {
    lastSecond = secondHand;
    // turn a new spot on or start from 1
    spotsOn = spotsOn % maxSpotsOn + 1;

    // reset the new spot
    spots[spotsOn-1] = Spot();
  }
}
