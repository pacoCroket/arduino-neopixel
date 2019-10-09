#include <FastLED.h>

// LED
#define LED_PIN     3
#define BRIGHTNESS  250
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
//#define NUM_SLICES 12
//#define NUM_SLICES 7
#define NUM_LEDS 61
#define NUM_SKIP_LEDS 5

// the box is 29 x 24 x 38 cm
// store leds in x, y, z, cordinates
const PROGMEM int ledsArray[NUM_LEDS][3] = {{29, 20, 4}, {29, 19, 7}, {29, 18, 10}, {29, 17, 13}, {29, 6, 23}, {29, 4, 21}, {29, 2, 19}, {20, 0, 10}, {19, -1, 10}, {19, -5, 10}, {19, -9, 10}, {17, -9, 10}, {17, -5, 10}, {17, -1, 10}, {14, 0, 10}, {17, 0, 23}, {18, 0, 25}, {19, 0, 27},
{25, 0, 37}, {27, 2, 38}, {29, 4, 37}, {21, 16, 38}, {19, 19, 38}, {17, 23, 38}, {15, 24, 37}, {13, 24, 35}, 
{1, 24, 36}, {0, 22, 37}, {1, 20, 38}, {6, 6, 38}, {2, 6, 38}, {0, 5, 36}, {0, 12, 26}, {-3, 12, 25}, {-7, 12, 24}, {-7, 12, 23}, {-3, 12, 22}, {0, 12, 21},
{0, 5, 11}, {0, 3, 8}, {1, 1, 6}, {6, 5, 0}, {9, 5, 0}, {12, 5, 0}, {15, 22, 0}, {12, 22, 0}, {9, 22, 0}, {0, 21, 10}, {1, 24, 12}, {3, 24, 14},
{6, 26, 20}, {13, 30, 18}, {14, 30, 16}, {17, 26, 14}, {26, 24, 22}, {29, 26, 22}, {31, 29, 22}, {32, 29, 22}, {30, 24, 22}, {29, 22, 22}};

// This example combines two features of FastLED to produce a remarkable range of
// effects from a relatively small amount of code.  This example combines FastLED's 
// color palette lookup functions with FastLED's Perlin/simplex noise generator, and
// the combination is extremely powerful.
//
// You might want to look at the "ColorPalette" and "Noise" examples separately
// if this example code seems daunting.
//
// 
// The basic setup here is that for each frame, we generate a new array of 
// 'noise' data, and then map it onto the LED matrix through a color palette.
//
// Periodically, the color palette is changed, and new noise-generation parameters
// are chosen at the same time.  In this example, specific noise-generation
// values have been selected to match the given color palettes; some are faster, 
// or slower, or larger, or smaller than others, but there's no reason these 
// parameters can't be freely mixed-and-matched.
//
// In addition, this example includes some fast automatic 'data smoothing' at 
// lower noise speeds to help produce smoother animations in those cases.
//
// The FastLED built-in color palettes (Forest, Clouds, Lava, Ocean, Party) are
// used, as well as some 'hand-defined' ones, and some proceedurally generated
// palettes.


// The leds
CRGB leds[NUM_LEDS];

// The 8 bit version of our coordinates
static uint16_t x;
static uint16_t y;
static uint16_t z;
static uint8_t theta0;

// We're using the x/y dimensions to map to the x/y pixels on the matrix.  We'll
// use the z-axis for "time".  speed determines how fast time moves forward.  Try
// 1 for a very slow moving effect, or 60 for something that ends up looking like
// water.
uint8_t speed = 20; // speed is set dynamically once we've started up

// Scale determines how far apart the pixels in our noise matrix are.  Try
// changing these values around to see how it affects the motion of the display.  The
// higher the value of scale, the more "zoomed out" the noise iwll be.  A value
// of 1 will be so zoomed in, you'll mostly see solid colors.

double scale = 15; // scale is set dynamically once we've started up
double scaleFactor = 2; 
uint8_t interval = 10; // spaxe between slices

CRGBPalette16 currentPalette( PartyColors_p );
uint8_t       colorLoop = 1;

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
  if( speed < 50) {
    dataSmoothing = 200 - (speed * 4);
  }

  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    // first value is the radius
    
    uint8_t xoffset = pgm_read_byte(&(ledsArray[i][0])) * scale;
    uint8_t yoffset = pgm_read_byte(&(ledsArray[i][1])) * scale;
    uint8_t zoffset = pgm_read_byte(&(ledsArray[i][2])) * scale;
    
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



// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.

// 1 = 5 sec per palette
// 2 = 10 sec per palette
// etc
#define HOLD_PALETTES_X_TIMES_AS_LONG 5

void ChangePaletteAndSettingsPeriodically()
{
  uint8_t secondHand = ((millis() / 1000) / HOLD_PALETTES_X_TIMES_AS_LONG) % 60;
  static uint8_t lastSecond = 99;
  
  if( lastSecond != secondHand) {
    lastSecond = secondHand;
    if( secondHand == 0)  { currentPalette = LavaColors_p;            speed =  8; scale = 7 * scaleFactor; colorLoop = 0; }
    if( secondHand == 5)  { SetupBlackAndWhiteStripedPalette();       speed = 7; scale = 5 * scaleFactor; colorLoop = 1; }
    if( secondHand ==  10)  { SetupPurpleAndGreenPalette();             speed = 4; scale = 5 * scaleFactor; colorLoop = 1; }
//    if( secondHand == 15)  { currentPalette = ForestColors_p;          speed =  3; scale = 8 * scaleFactor; colorLoop = 0; }
    if( secondHand == 20)  { currentPalette = CloudColors_p;           speed =  5; scale = 6 * scaleFactor; colorLoop = 0; }
    if( secondHand == 25)  { currentPalette = RainbowColors_p;         speed = 9; scale = 5 * scaleFactor; colorLoop = 1; }
    if( secondHand == 30)  { currentPalette = OceanColors_p;           speed = 10; scale = 12 * scaleFactor; colorLoop = 0; }
    if( secondHand == 35)  { currentPalette = PartyColors_p;           speed = 8; scale = 4 * scaleFactor; colorLoop = 1; }
    if( secondHand == 40)  { SetupRandomPalette();                     speed = 10; scale = 4 * scaleFactor; colorLoop = 1; }
    if( secondHand == 45)  { SetupRandomPalette();                     speed = 8; scale = 15 * scaleFactor; colorLoop = 1; }
    if( secondHand == 50)  { SetupRandomPalette();                     speed = 25; scale = 6 * scaleFactor; colorLoop = 1; }
    if( secondHand == 55)  { currentPalette = RainbowStripeColors_p;   speed = 8; scale = 4 * scaleFactor; colorLoop = 1; }
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
  currentPalette[9] = CRGB::White;
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
