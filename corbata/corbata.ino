#include <FastLED.h>

// LED
#define LED_PIN     3
#define BRIGHTNESS  255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS 50

CRGB leds[NUM_LEDS];

static double x;
static double y;
static double z;

// for each palette
double speedFactor = 0.3;
double speed = 6 * speedFactor; // speed is set dynamically once we've started up
double scaleFactor = 4; 
double scale = 6 * scaleFactor; // scale is set dynamically once we've started up
uint8_t       colorLoop = 1;

 // for blending in palettes smoothly
uint8_t maxChanges = 48;
uint8_t countBlend = 0;
CRGBPalette16 targetPalette( LavaColors_p );
CRGBPalette16 currentPalette( LavaColors_p );

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

  static uint8_t ihue=0;
  uint8_t xCoord;
    
  uint8_t dataSmoothing = 0;
  if( speed < 4) {
    dataSmoothing = 200 - (speed * 7);
  }

  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    if (i >= 25) {
      xCoord = 50 - i; 
    } else {
      // first stripe
      xCoord = i;
    }

    uint16_t xoffset = scale * xCoord;  
      
    uint8_t index = inoise8(x + xoffset, z);
    uint8_t bri = inoise8(x, z + xoffset); // another random point for brightness


    if( dataSmoothing ) {
      uint8_t olddata = inoise8(x + xoffset + speed,z-speed);
      uint8_t newdata = scale8( olddata, dataSmoothing) + scale8( index, 256 - dataSmoothing);
      index = newdata;
    }

    // if this palette is a 'loop', add a slowly-changing base value
      if( colorLoop) { 
        index += ihue;
      }

      if( bri > 180 ) { 
        bri = 220;
      } else {
        bri = dim8_raw( bri );
      }

      leds[i] = ColorFromPalette( currentPalette, index, bri);
  }
  
  z += speed;
  // apply slow drift to X and Y, just for visual variation.
  x -= speed;

  ihue+=1;
}

void loop() {
  ChangePaletteAndSettingsPeriodically();

  // run the blend function only every Nth frames
  if (countBlend == 3) {
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);
    countBlend = 0;
  }
  countBlend++;

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
    if( secondHand == 0)  { targetPalette = LavaColors_p;            speed =  8 * speedFactor; scale = 7 * scaleFactor; colorLoop = 0; }
    if( secondHand == 5)  { SetupBlackAndWhiteStripedPalette();       speed = 30 * speedFactor; scale = 14 * scaleFactor; colorLoop = 1; }
    if( secondHand ==  10)  { SetupPurpleAndGreenPalette();             speed = 1 * speedFactor; scale = 4 * scaleFactor; colorLoop = 1; }
    // if( secondHand == 15)  { targetPalette = ForestColors_p;          speed =  3 * speedFactor; scale = 8 * scaleFactor; colorLoop = 0; }
    if( secondHand == 20)  { targetPalette = CloudColors_p;           speed =  4 * speedFactor; scale = 7 * scaleFactor; colorLoop = 0; }
    if( secondHand == 25)  { targetPalette = RainbowColors_p;         speed = 10 * speedFactor; scale = 5 * scaleFactor; colorLoop = 1; }
    if( secondHand == 30)  { targetPalette = OceanColors_p;           speed = 10 * speedFactor; scale = 15 * scaleFactor; colorLoop = 0; }
    if( secondHand == 35)  { targetPalette = PartyColors_p;           speed = 15 * speedFactor; scale = 4 * scaleFactor; colorLoop = 1; }
    if( secondHand == 40)  { SetupRandomPalette();                     speed = 10 * speedFactor; scale = 7 * scaleFactor; colorLoop = 1; }
    if( secondHand == 45)  { SetupRandomPalette();                     speed = 8 * speedFactor; scale = 16 * scaleFactor; colorLoop = 1; }
    if( secondHand == 50)  { SetupRandomPalette();                     speed = 24 * speedFactor; scale = 6 * scaleFactor; colorLoop = 1; }
    if( secondHand == 55)  { targetPalette = RainbowStripeColors_p;   speed = 10 * speedFactor; scale = 4 * scaleFactor; colorLoop = 1; }
  }
}

void SetupRandomPalette()
{
  targetPalette = CRGBPalette16( 
                      CHSV( random8(), 255, 32), 
                      CHSV( random8(), 255, 255), 
                      CHSV( random8(), 128, 255), 
                      CHSV( random8(), 255, 255)); 
}

void SetupBlackAndWhiteStripedPalette()
{
  // 'black out' all 16 palette entries...
  fill_solid( targetPalette, 16, CRGB::Black);
  // and set every fourth one to white.
  targetPalette[0] = CRGB::White;
//  targetPalette[8] = CRGB::White;
  targetPalette[12] = CRGB::White;

}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
  CRGB purple = CHSV( HUE_PURPLE, 255, 255);
  CRGB green  = CHSV( HUE_GREEN, 255, 255);
  CRGB black  = CRGB::Black;
  
  targetPalette = CRGBPalette16( 
    green,  green,  black,  black,
    purple, purple, black,  black,
    green,  green,  black,  black,
    purple, purple, black,  black );
}

