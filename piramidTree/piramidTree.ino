#include <FastLED.h>

// LED
#define LED_PIN1     3
#define LED_PIN2     5
#define BRIGHTNESS  180
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB // GRB for WS2812, BRG for WS2811
#define NUM_LEDS 300

// The leds
CRGB leds[NUM_LEDS];

// The 8 bit version of our coordinates
static double x;
static double z;

double speedFactor = 0.25;
double speed = 6 * speedFactor; // speed is set dynamically once we've started up
double scaleFactor = 2; 
double scale = 6 * scaleFactor; // scale is set dynamically once we've started up
uint8_t       colorLoop = 1;

CRGBPalette16 currentPalette( LavaColors_p );

void setup() {
//  Serial.begin(9600);
  
  LEDS.addLeds<LED_TYPE,LED_PIN1,COLOR_ORDER>(leds,NUM_LEDS);
  LEDS.addLeds<LED_TYPE,LED_PIN2,COLOR_ORDER>(leds,NUM_LEDS);
  LEDS.setBrightness(BRIGHTNESS);

  // Initialize our coordinates to some random values
  x = random8();
  z = random8();
}


void mapCoordToColor() {
  // If we're runing at a low "speed", some 8-bit artifacts become visible
  // from frame-to-frame.  In order to reduce this, we can do some fast data-smoothing.
  // The amount of data smoothing we're doing depends on "speed".

  static uint8_t ihue=0;
    
  uint8_t dataSmoothing = 0;
  if( speed < 50) {
    dataSmoothing = 200 - (speed * 4);
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    // first value is the radius
    
    uint16_t xoffset = i * scale;
    
    uint8_t index = inoise8(x + xoffset, z);
//
//    uint8_t index = inoise8(x + height, y + radius, xoffset);
//    uint8_t bri = 220; // another random point for brightness

//    uint8_t index = inoise8(x + xoffset, y + yoffset z);
    uint8_t bri = inoise8(z, x + xoffset); // another random point for brightness

//    if( dataSmoothing ) {
//      uint8_t olddata = inoise8(x + xoffset - speed / 8,z-speed);
//      uint8_t newdata = scale8( olddata, dataSmoothing) + scale8( index, 256 - dataSmoothing);
//      index = newdata;
//    }

    // if this palette is a 'loop', add a slowly-changing base value
      if( colorLoop) { 
        index += ihue;
      }

      // brighten up, as the color palette itself often contains the 
      // light/dark dynamic range desired
//      if( bri > 170 ) {
//        bri = 255;
//      } else {
        bri = dim8_raw( bri );
//      }

    CRGB color = ColorFromPalette( currentPalette, index, bri);
    leds[i] = color;
  }
  
  z += speed/2;
  
  // apply slow drift to X and Y, just for visual variation.
  x += speed / 8;

  ihue+=1;
  
}

void loop() {
  // Periodically choose a new palette, speed, and scale
  ChangePaletteAndSettingsPeriodically();

  // change 1D smooth closing noise array periodically
  // TODO > incrementally change the center of circle ingenerateNoiseLoop()

  mapCoordToColor();

  LEDS.show();
//   delay(20);
}

#define HOLD_PALETTES_X_TIMES_AS_LONG 8

void ChangePaletteAndSettingsPeriodically()
{
  uint8_t secondHand = ((millis() / 1000) / HOLD_PALETTES_X_TIMES_AS_LONG) % 60;
  static uint8_t lastSecond = 99;
  
  if( lastSecond != secondHand) {
    lastSecond = secondHand;
    if( secondHand == 0)  { currentPalette = LavaColors_p;            speed =  7 * speedFactor; scale = 7 * scaleFactor; colorLoop = 0; }
    if( secondHand == 5)  { SetupBlackAndWhiteStripedPalette();       speed = 35 * speedFactor; scale = 5 * scaleFactor; colorLoop = 1; }
    if( secondHand ==  10)  { SetupPurpleAndGreenPalette();             speed = 1 * speedFactor; scale = 4 * scaleFactor; colorLoop = 1; }
//    if( secondHand == 15)  { currentPalette = ForestColors_p;          speed =  3; scale = 8 * scaleFactor; colorLoop = 0; }
    if( secondHand == 15)  { currentPalette = CloudColors_p;           speed =  8 * speedFactor; scale = 7 * scaleFactor; colorLoop = 0; }
    if( secondHand == 25)  { currentPalette = RainbowColors_p;         speed = 15 * speedFactor; scale = 5 * scaleFactor; colorLoop = 1; }
    if( secondHand == 30)  { currentPalette = OceanColors_p;           speed = 20 * speedFactor; scale = 25 * scaleFactor; colorLoop = 0; }
    if( secondHand == 35)  { currentPalette = PartyColors_p;           speed = 15 * speedFactor; scale = 4 * scaleFactor; colorLoop = 1; }
    if( secondHand == 40)  { SetupRandomPalette();                     speed = 10 * speedFactor; scale = 7 * scaleFactor; colorLoop = 1; }
    if( secondHand == 45)  { SetupRandomPalette();                     speed = 7 * speedFactor; scale = 15 * scaleFactor; colorLoop = 1; }
    if( secondHand == 50)  { SetupRandomPalette();                     speed = 25 * speedFactor; scale = 6 * scaleFactor; colorLoop = 1; }
    if( secondHand == 55)  { currentPalette = RainbowStripeColors_p;   speed = 12 * speedFactor; scale = 4 * scaleFactor; colorLoop = 1; }
  }
}

// This function generates a random palette that's a gradient
// between four different colors.  The first is a dim hue, the second is 
// a bright hue, the third is a bright pastel, and the last is 
// another bright hue.  This gives some visual bright/dark variation
// which is more interesting than just a gradient of different hues.
void SetupRandomPalette()
{
  currentPalette = CRGBPalette16( 
                      CHSV( random8(), 255, 32), 
                      CHSV( random8(), 255, 255), 
                      CHSV( random8(), 128, 255), 
                      CHSV( random8(), 255, 255)); 
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
  // 'black out' all 16 palette entries...
  fill_solid( currentPalette, 16, CRGB::Black);
  // and set every fourth one to white.
  currentPalette[0] = CRGB::White;
//  currentPalette[9] = CRGB::White;
  currentPalette[11] = CRGB::White;
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
  CRGB purple = CHSV( HUE_PURPLE, 255, 255);
  CRGB green  = CHSV( HUE_GREEN, 255, 255);
  CRGB black  = CRGB::Black;
  
  currentPalette = CRGBPalette16( 
    green,  green,  black,  black,
    purple, purple, black,  black,
    green,  green,  black,  black,
    purple, purple, black,  black );
}
