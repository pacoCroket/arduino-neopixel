#include <FastLED.h>
#include <MPU6050_tockn.h>
#include <Wire.h>

// LED
#define LED_PIN     3
#define BRIGHTNESS  255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS 54

CRGB leds[NUM_LEDS];

static double x;
static double y;
static double z;

// for each palette
double speedFactor = 0.25;
double speed = 6 * speedFactor; // speed is set dynamically once we've started up
double scaleFactor = 2; 
double scale = 6 * scaleFactor; // scale is set dynamically once we've started up
uint8_t       colorLoop = 1;

 // for blending in palettes smoothly
uint8_t maxChanges = 48;
uint8_t countBlend = 0;
CRGBPalette16 targetPalette( LavaColors_p );
CRGBPalette16 currentPalette( LavaColors_p );

// motion sensitivity
MPU6050 mpu6050(Wire);
float accFactor = 1; // current acceleration, smoothed
float accThreshold = 2; // threshold to trigger something
long accLastEvent = 0; // timestamp in millis of last threshold
float accEventDelay = 1000; // duration of accThreshold event

int angleFactor = 0;
float angleThreshold = 2; // threshold to trigger something
long angleLastEvent = 0; // timestamp in millis of last angle change
float angleEventDelay = 1000; // duration of accThreshold event

void setup() {
//  Serial.begin(9600);

  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  
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
  if( speed < 50) {
    dataSmoothing = 200 - (speed * 4);
  }

  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    if (i >= 50 && i <= 52) {
      xCoord = -4;
    } else if (i >= 48) {
      // head starts at led 48 
      xCoord = - 2;
    } else if (i >= 24) {
      xCoord = 47 - i; 
    } else {
      // first stripe
      xCoord = i;
    }

    if (xCoord == angleFactor && millis() - angleLastEvent < angleEventDelay) {
      leds[i] = CHSV( HUE_PURPLE, 255, 255);
      continue;
    }

    uint16_t xoffset = scale * xCoord;  
      
    uint8_t index = inoise8(x + xoffset, z);
    uint8_t bri = inoise8(x, z + xoffset); // another random point for brightness


    if( dataSmoothing ) {
      uint8_t olddata = inoise8(x + xoffset - speed / 8,z-speed);
      uint8_t newdata = scale8( olddata, dataSmoothing) + scale8( index, 256 - dataSmoothing);
      index = newdata;
    }

    // if this palette is a 'loop', add a slowly-changing base value
      if( colorLoop) { 
        index += ihue;
      }

      bri = dim8_raw( bri );

      leds[i] = ColorFromPalette( currentPalette, index, bri);
  }
  
  z += speed;
  
  // apply slow drift to X and Y, just for visual variation.
  x += speed / 8;
  y -= speed / 16;

  ihue+=1;
}

void loop() {
  mpu6050.update();
  updateMPUFactors();

  ChangePaletteAndSettingsPeriodically();

  // run the blend function only every Nth frames
  if (countBlend == 5) {
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);
    countBlend = 0;
  }
  countBlend++;

  mapCoordToColor();
  LEDS.show();
//   delay(20);
}

void updateMPUFactors() {
  // total acceleration
  accFactor = 0.9*accFactor + 0.1*pow(pow(mpu6050.getAccX(), 2) + pow(mpu6050.getAccY(), 2) + pow(mpu6050.getAccZ(), 2), 0.5);
  if (accFactor > accThreshold) {
    accLastEvent = millis();
  }
  // angles
  int newAngleFactor = (int)(mpu6050.getAngleX() / 180 * 24) + 11; // 11 is middle LED
  if (newAngleFactor != angleFactor) {
    angleLastEvent = millis();
    angleFactor = newAngleFactor;
    if (angleFactor > 23) angleFactor = 47 - angleFactor; 
    else if (angleFactor < 0) angleFactor *= -1;
  }
}


#define HOLD_PALETTES_X_TIMES_AS_LONG 5

void ChangePaletteAndSettingsPeriodically()
{
  uint8_t secondHand = ((millis() / 1000) / HOLD_PALETTES_X_TIMES_AS_LONG) % 60;
  static uint8_t lastSecond = 99;
  
  if( lastSecond != secondHand) {
    lastSecond = secondHand;
    if( secondHand == 0)  { targetPalette = LavaColors_p;            speed =  8; scale = 7; colorLoop = 0; }
    if( secondHand ==  5)  { SetupPurpleAndGreenPalette();             speed = 5; scale = 3; colorLoop = 1; }
    if( secondHand == 10)  { SetupBlackAndWhiteStripedPalette();       speed = 8; scale = 5; colorLoop = 1; }
    if( secondHand == 15)  { targetPalette = ForestColors_p;          speed =  3; scale = 8; colorLoop = 0; }
    if( secondHand == 20)  { targetPalette = CloudColors_p;           speed =  4; scale = 5; colorLoop = 0; }
    if( secondHand == 25)  { targetPalette = RainbowColors_p;         speed = 10; scale = 5; colorLoop = 1; }
    if( secondHand == 30)  { targetPalette = OceanColors_p;           speed = 10; scale = 10; colorLoop = 0; }
    if( secondHand == 35)  { targetPalette = PartyColors_p;           speed = 7; scale = 4; colorLoop = 1; }
    if( secondHand == 40)  { SetupRandomPalette();                     speed = 10; scale = 4; colorLoop = 1; }
    if( secondHand == 45)  { SetupRandomPalette();                     speed = 25; scale = 6; colorLoop = 1; }
    if( secondHand == 50)  { SetupRandomPalette();                     speed = 10; scale = 15; colorLoop = 1; }
    if( secondHand == 55)  { targetPalette = RainbowStripeColors_p;   speed = 8; scale = 3; colorLoop = 1; }
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

