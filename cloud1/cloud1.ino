#include <FastLED.h>

// LED
#define LED_PIN     3
#define BRIGHTNESS  250
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define NUM_SLICES 12
#define NUM_LEDS 159

// each LED has 3 coordinates, [radius, height, angle]

// [id, h, r], starting form one before and ending one after
//const uint8_t zeroPlane[NUM_SLICES][3] = {{0, 0, 8}, {23, 14, 13}, {55, 33, 16}, {88, 51, 15}, {113, 64, 15}, {139, 73, 12}, {159, 81, 6}};

const uint8_t zeroPlane[NUM_SLICES][3] = {{0, 0, 8}, {23, 14, 13}, {55, 33, 16}, {88, 51, 15}, {113, 64, 15}, {139, 73, 12}, {160, 81, 6}};
const uint8_t halfPlane[NUM_SLICES][3] = {{0, 0, 8}, {12, 7, 9}, {23, 14, 13}, {30, 25, 15}, {55, 33, 16}, {70, 42, 16}, {88, 51, 15}, {113, 64, 15}, {121, 68, 14}, {139, 73, 12}, {146, 76, 10}, {159, 81, 6}};
static uint8_t ledsArray[NUM_LEDS][4]; // Populated in setup [x, y, z, theta]

// The leds
CRGB leds[NUM_LEDS];

// The 8 bit version of our coordinates
static double x;
static double y;
static double z;
static uint8_t theta0;

double speedFactor = 0.2;
double speed = 6 * speedFactor; // speed is set dynamically once we've started up
double scaleFactor = 0.5; 
double scale = 6 * scaleFactor; // scale is set dynamically once we've started up
uint8_t       colorLoop = 1;

CRGBPalette16 currentPalette( LavaColors_p );

bool initSync = true;
static uint8_t initSeconds = 5;

void setup() {
//  Serial.begin(9600);
  
  LEDS.addLeds<LED_TYPE,LED_PIN,COLOR_ORDER>(leds,NUM_LEDS);
  LEDS.setBrightness(BRIGHTNESS);

  // Initialize our coordinates to some random values
  x = random8();
  y = random8();
  z = random8();
  theta0 = random8();

  ChangePaletteAndSettingsPeriodically();
  generateCoords();
}

void generateCoords() {
  for (int i = 0; i <= NUM_LEDS; i++) {    
    for (int j = 0; j <= NUM_SLICES - 1; j++) {
//      if (i >= zeroPlane[j][0]) {
      if (i >= halfPlane[j][0]) {
        uint8_t h = map(i, zeroPlane[j][0], zeroPlane[j+1][0], zeroPlane[j][1], zeroPlane[j+1][1]);
        uint8_t r = map(i, zeroPlane[j][0], zeroPlane[j+1][0], zeroPlane[j][2], zeroPlane[j+1][2]);
        uint8_t theta;
        if (i % 2 == 0) {
          theta = map(i, zeroPlane[j][0], zeroPlane[j+1][0], 0, 127);          
        } else {
          theta = map(i, zeroPlane[j][0], zeroPlane[j+1][0], 128, 255);          
        }

        ledsArray[i][0] = r*(cos8(theta)-127); // cos8() has range [0-255]
        ledsArray[i][1] = r*(sin8(theta)-127);
        ledsArray[i][2] = h;
        ledsArray[i][3] = theta;
        break;
      }
    }    
  }
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

  for (int i = 0; i < NUM_LEDS; i++) {
    // first value is the radius
    uint16_t xoffset = ledsArray[i][0] * scale;
    uint16_t yoffset = ledsArray[i][1] * scale;
    uint16_t zoffset = ledsArray[i][2] * scale / 2;
    
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
//      if( bri > 180 ) {
//        bri = 220;
//      } else {
        bri = dim8_raw( bri );
//      }

    CRGB color = ColorFromPalette( currentPalette, index, bri);
    leds[i] = color;
  }
  
  z += speed;
  
  // apply slow drift to X and Y, just for visual variation.
  x += speed / 4;
  y -= speed / 8;

  ihue+=1;
  
}

void loop() {
  // Periodically choose a new palette, speed, and scale
  ChangePaletteAndSettingsPeriodically();

  // change 1D smooth closing noise array periodically
  // TODO > incrementally change the center of circle ingenerateNoiseLoop()

  mapCoordToColor();

//  sameAngle();

  LEDS.show();
//   delay(20);
}

void sameAngle() {
  
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    int index = 10;
    int bri = 0;
    if (ledsArray[i][3] < theta0 + 8 && ledsArray[i][3] > theta0 - 8) {
//    for (int j = 0; j <= NUM_SLICES; j++) {
//      if (i == zeroPlane[j][0]) {
        bri = 220;
//      }
    }
    
    CRGB color = ColorFromPalette( currentPalette, index, bri);
    leds[i] = color;
  }
  theta0 = round(theta0 + speed*0.05) % 255;  
   delay(10);
}

#define HOLD_PALETTES_X_TIMES_AS_LONG 8

void ChangePaletteAndSettingsPeriodically()
{
  uint8_t secondHand = ((millis() / 1000) / HOLD_PALETTES_X_TIMES_AS_LONG) % 60;
  static uint8_t lastSecond = 99;
  
  if( lastSecond != secondHand) {
    lastSecond = secondHand;
    if( secondHand == 0)  { currentPalette = LavaColors_p;            speed =  7 * speedFactor; scale = 7 * scaleFactor; colorLoop = 1; }
    if( secondHand == 5)  { SetupBlackAndWhiteStripedPalette();       speed = 35 * speedFactor; scale = 5 * scaleFactor; colorLoop = 1; }
    if( secondHand ==  10)  { SetupPurpleAndGreenPalette();             speed = 1 * speedFactor; scale = 4 * scaleFactor; colorLoop = 1; }
//    if( secondHand == 15)  { currentPalette = ForestColors_p;          speed =  3; scale = 8 * scaleFactor; colorLoop = 0; }
    if( secondHand == 20)  { currentPalette = CloudColors_p;           speed =  8 * speedFactor; scale = 7 * scaleFactor; colorLoop = 1; }
    if( secondHand == 25)  { currentPalette = RainbowColors_p;         speed = 15 * speedFactor; scale = 5 * scaleFactor; colorLoop = 1; }
    if( secondHand == 30)  { currentPalette = OceanColors_p;           speed = 20 * speedFactor; scale = 25 * scaleFactor; colorLoop = 1; }
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
  currentPalette[5] = CRGB::White;
//  currentPalette[10] = CRGB::White;
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
