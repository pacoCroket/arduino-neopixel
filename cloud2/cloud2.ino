#include <FastLED.h>

// LED
#define LED_PIN     3
#define BRIGHTNESS  250
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS 61

// the box is 29 x 24 x 38 cm
// store leds in x, y, z, cordinates
const PROGMEM int ledsArray[NUM_LEDS][3] = {{29, 20, 4}, {29, 19, 7}, {29, 18, 10}, {29, 17, 13}, {29, 6, 23}, {29, 4, 21}, {29, 2, 19}, {20, 0, 10}, {19, -1, 10}, {19, -5, 10}, {19, -9, 10}, {17, -9, 10}, {17, -5, 10}, {17, -1, 10}, {14, 0, 10}, {17, 0, 23}, {18, 0, 25}, {19, 0, 27},
{25, 0, 37}, {27, 2, 38}, {29, 4, 37}, {21, 16, 38}, {19, 19, 38}, {17, 23, 38}, {15, 24, 37}, {13, 24, 35}, 
{1, 24, 36}, {0, 22, 37}, {1, 20, 38}, {6, 6, 38}, {2, 6, 38}, {0, 5, 36}, {0, 12, 26}, {-3, 12, 25}, {-7, 12, 24}, {-7, 12, 23}, {-3, 12, 22}, {0, 12, 21},
{0, 5, 11}, {0, 3, 8}, {1, 1, 6}, {6, 5, 0}, {9, 5, 0}, {12, 5, 0}, {15, 22, 0}, {12, 22, 0}, {9, 22, 0}, {0, 21, 10}, {1, 24, 12}, {3, 24, 14},
{6, 26, 20}, {13, 30, 18}, {14, 30, 16}, {17, 26, 14}, {26, 24, 22}, {29, 26, 22}, {31, 29, 22}, {32, 29, 22}, {30, 24, 22}, {29, 22, 22}};

// The leds
CRGB leds[NUM_LEDS];

// The 8 bit version of our coordinates
static double x;
static double y;
static double z;

double speedFactor = 0.125;
double speed = 6 * speedFactor; // speed is set dynamically once we've started up
double scaleFactor = 4; 
double scale = 6 * scaleFactor; // scale is set dynamically once we've started up
uint8_t       colorLoop = 1;

CRGBPalette16 currentPalette( PartyColors_p );

void setup() {
//  Serial.begin(9600);
  
  LEDS.addLeds<LED_TYPE,LED_PIN,COLOR_ORDER>(leds,NUM_LEDS);
  LEDS.setBrightness(BRIGHTNESS);

  // Initialize our coordinates to some random values
  x = random8();
  y = random8();
  z = random8();
}

void mapCoordToColor() {
  // If we're runing at a low "speed", some 8-bit artifacts become visible
  // from frame-to-frame.  In order to reduce this, we can do some fast data-smoothing.
  // The amount of data smoothing we're doing depends on "speed".

  static uint8_t ihue=0;
    
  uint8_t dataSmoothing = 0;
  if( speed < 4) {
    dataSmoothing = 200 - (speed * 4);
  }

  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    // first value is the radius
    
    uint16_t xoffset = pgm_read_byte(&(ledsArray[i][0])) * scale;
    uint16_t yoffset = pgm_read_byte(&(ledsArray[i][1])) * scale;
    uint16_t zoffset = pgm_read_byte(&(ledsArray[i][2])) * scale;
    
    uint8_t index = inoise8(x + xoffset, y + yoffset, z + zoffset);
//
//    uint8_t index = inoise8(x + height, y + radius, xoffset);
//    uint8_t bri = 220; // another random point for brightness

//    uint8_t index = inoise8(x + xoffset, y + yoffset z);
    uint8_t bri = inoise8(x + yoffset, y + xoffset, z + zoffset); // another random point for brightness

    if( dataSmoothing ) {
      uint8_t olddata = inoise8(x + xoffset - speed / 8,y + yoffset + speed / 16,z-speed);
      uint8_t newdata = scale8( olddata, dataSmoothing) + scale8( index, 256 - dataSmoothing);
      index = newdata;
    }

    // if this palette is a 'loop', add a slowly-changing base value
      if( colorLoop) { 
        index += ihue;
      }

      // brighten up, as the color palette itself often contains the 
      // light/dark dynamic range desired
      if( bri > 180 ) {
        bri = 220;
      } else {
        bri = dim8_raw( bri );
      }

    CRGB color = ColorFromPalette( currentPalette, index, bri);
    leds[i] = color;
  }
  
  z += speed / 4;
  
  // apply slow drift to X and Y, just for visual variation.
  x += speed / 8;
  y -= speed / 16;

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
    if( secondHand == 0)  { currentPalette = LavaColors_p;            speed =  8 * speedFactor; scale = 8 * scaleFactor; colorLoop = 1; }
    if( secondHand == 5)  { SetupBlackAndWhiteStripedPalette();       speed = 40 * speedFactor; scale = 16 * scaleFactor; colorLoop = 1; }
    if( secondHand ==  10)  { SetupPurpleAndGreenPalette();             speed = 1 * speedFactor; scale = 4 * scaleFactor; colorLoop = 1; }
//    if( secondHand == 15)  { currentPalette = ForestColors_p;          speed =  3; scale = 8 * scaleFactor; colorLoop = 0; }
    if( secondHand == 20)  { currentPalette = CloudColors_p;           speed =  8 * speedFactor; scale = 8 * scaleFactor; colorLoop = 1; }
    if( secondHand == 25)  { currentPalette = RainbowColors_p;         speed = 16 * speedFactor; scale = 4 * scaleFactor; colorLoop = 1; }
    if( secondHand == 30)  { currentPalette = OceanColors_p;           speed = 20 * speedFactor; scale = 25 * scaleFactor; colorLoop = 1; }
    if( secondHand == 35)  { currentPalette = PartyColors_p;           speed = 16 * speedFactor; scale = 4 * scaleFactor; colorLoop = 1; }
    if( secondHand == 40)  { SetupRandomPalette();                     speed = 10 * speedFactor; scale = 8 * scaleFactor; colorLoop = 1; }
    if( secondHand == 45)  { SetupRandomPalette();                     speed = 8 * speedFactor; scale = 16 * scaleFactor; colorLoop = 1; }
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
//  currentPalette[0] = CRGB::White;
  currentPalette[9] = CRGB::White;
//  currentPalette[11] = CRGB::White;
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
