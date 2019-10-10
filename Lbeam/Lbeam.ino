#include <FastLED.h>

// LED
#define LED_PIN     3
#define BRIGHTNESS  250
#define LED_TYPE    WS2812
#define COLOR_ORDER BRG // GRB for WS2812, BRG for WS2811
#define NUM_LEDS 100

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
static double x;
static double z;

// We're using the x/y dimensions to map to the x/y pixels on the matrix.  We'll
// use the z-axis for "time".  speed determines how fast time moves forward.  Try
// 1 for a very slow moving effect, or 60 for something that ends up looking like
// water.

// Scale determines how far apart the pixels in our noise matrix are.  Try
// changing these values around to see how it affects the motion of the display.  The
// higher the value of scale, the more "zoomed out" the noise iwll be.  A value
// of 1 will be so zoomed in, you'll mostly see solid colors.

double scale = 15; // scale is set dynamically once we've started up
double scaleFactor = 2; 
double speed = 20; // speed is set dynamically once we've started up
double speedFactor = 0.3; 

CRGBPalette16 currentPalette( PartyColors_p );
uint8_t       colorLoop = 1;

void setup() {
//  Serial.begin(9600);
  
  LEDS.addLeds<LED_TYPE,LED_PIN,COLOR_ORDER>(leds,NUM_LEDS);
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
    
    uint16_t index = inoise8(x + xoffset, z);
//
//    uint8_t index = inoise8(x + height, y + radius, xoffset);
//    uint8_t bri = 220; // another random point for brightness

//    uint8_t index = inoise8(x + xoffset, y + yoffset z);
    uint16_t bri = inoise8(x + xoffset*5, z + xoffset); // another random point for brightness

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
#define HOLD_PALETTES_X_TIMES_AS_LONG 10

void ChangePaletteAndSettingsPeriodically()
{
  uint8_t secondHand = ((millis() / 1000) / HOLD_PALETTES_X_TIMES_AS_LONG) % 60;
  static uint8_t lastSecond = 99;
  
  if( lastSecond != secondHand) {
    lastSecond = secondHand;
    if( secondHand == 0)  { currentPalette = LavaColors_p;            speed =  8 * speedFactor; scale = 7 * scaleFactor; colorLoop = 0; }
    if( secondHand == 5)  { SetupBlackAndWhiteStripedPalette();       speed = 7 * speedFactor; scale = 5 * scaleFactor; colorLoop = 1; }
    if( secondHand ==  15)  { SetupPurpleAndGreenPalette();             speed = 4 * speedFactor; scale = 5 * scaleFactor; colorLoop = 1; }
//    if( secondHand == 15)  { currentPalette = ForestColors_p;          speed =  3; scale = 8 * scaleFactor; colorLoop = 0; }
    if( secondHand == 20)  { currentPalette = CloudColors_p;           speed =  5 * speedFactor; scale = 6 * scaleFactor; colorLoop = 0; }
    if( secondHand == 25)  { currentPalette = RainbowColors_p;         speed = 9 * speedFactor; scale = 5 * scaleFactor; colorLoop = 1; }
    if( secondHand == 30)  { currentPalette = OceanColors_p;           speed = 10 * speedFactor; scale = 12 * scaleFactor; colorLoop = 0; }
    if( secondHand == 35)  { currentPalette = PartyColors_p;           speed = 8 * speedFactor; scale = 4 * scaleFactor; colorLoop = 1; }
    if( secondHand == 40)  { SetupRandomPalette();                     speed = 10 * speedFactor; scale = 4 * scaleFactor; colorLoop = 1; }
    if( secondHand == 45)  { SetupRandomPalette();                     speed = 8 * speedFactor; scale = 15 * scaleFactor; colorLoop = 1; }
    if( secondHand == 50)  { SetupRandomPalette();                     speed = 25 * speedFactor; scale = 6 * scaleFactor; colorLoop = 1; }
    if( secondHand == 55)  { currentPalette = RainbowStripeColors_p;   speed = 8 * speedFactor; scale = 4 * scaleFactor; colorLoop = 1; }
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
