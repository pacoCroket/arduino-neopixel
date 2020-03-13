#include <FastLED.h>

// LED
#define LED_PIN     3
#define BRIGHTNESS  255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB // GRB for WS2812, BRG for WS2811
#define NUM_LEDS 48
#define BUTTON_PIN 2

// store leds in [x, y] cordinates
// const PROGMEM uint8_t lavaLedsArray[NUM_LEDS1][2] = {{30, 80}};
// cloud is 3D
const PROGMEM uint8_t ledsArray[NUM_LEDS][3] = {
{154, 90, 80}, {147, 90, 90}, {139, 90, 80},
{100, 110, 8}, {77, 93, 16}, {99, 72, 25},
{93, 23, 55}, {83, 6, 40}, {71, 23, 55},
{40, 44, 65}, {23, 27, 56}, {9, 51, 52},
{2, 87, 80}, {14, 102, 55}, {30, 88, 35},
{0, 132, 50}, {13, 135, 42}, {27, 153, 55},
{76, 164, 60}, {82, 179, 50}, {93, 164, 45},
{173, 165, 20}, {182, 183, 55}, {193, 156, 70},
{233, 155, 65}, {243, 182, 55}, {255, 164, 20},
{247, 101, 55}, {237, 90, 42}, {225, 75, 30},
{210, 84, 8}, {209, 75, 0}, {185, 68, 12},
{204, 44, 40}, {172, 32, 55}, {202, 22, 75},
{232, 5, 65}, {250, 27, 60}, {251, 3, 40},
{204, 0, 35}, {174, 3, 55}, {203, 7, 60},
{155, 8, 40}, {146, 31, 55}, {137, 8, 65},
{99, 63, 75}, {69, 92, 85}, {99, 118, 95}};

// The leds
CRGB leds[NUM_LEDS];

// The 8 bit version of our coordinates
static double x;
static double y;
static double z;

double speedFactor = 0.06;
double speed = 6 * speedFactor; // speed is set dynamically once we've started up
double newspeed = speed;
double scaleFactor = 0.6; 
double scale = 6 * scaleFactor; // scale is set dynamically once we've started up
double newscale = scale;
uint8_t       colorLoop = 1;

 // for blending in palettes smoothly
uint8_t maxChanges = 48;
uint8_t countBlend = 0;
float lerpAmount = 0.1;
CRGBPalette16 targetPalette( LavaColors_p );
CRGBPalette16 currentPalette( LavaColors_p );

boolean buttonState = HIGH;
boolean prevButtonState = HIGH;
uint8_t pressCount = 0;
unsigned long lastStatusSwitch = 999999;
uint8_t briScale = 255;
boolean isSwitchingPalette = true;

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
    dataSmoothing = 220 - (speed * 5);
  }

  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    // first value is the radius
    
    uint16_t xoffset = pgm_read_byte(&(ledsArray[i][0])) * scale;
    uint16_t yoffset = pgm_read_byte(&(ledsArray[i][1])) * scale;
    uint16_t zoffset = pgm_read_byte(&(ledsArray[i][2])) * scale;
    
    uint8_t index = inoise8(x + xoffset, y + yoffset, z + zoffset);
    uint8_t bri = inoise8(x + zoffset, y + xoffset, z + yoffset); // another random point for brightness

    if( dataSmoothing ) {
      uint8_t oldindex = inoise8(x + xoffset - speed / 2,y + yoffset + speed / 4,z + zoffset-speed);
      uint8_t oldbri = inoise8(x + zoffset - speed / 2,y + xoffset + speed / 4,z + yoffset-speed);
      index = scale8( oldindex, dataSmoothing) + scale8( index, 256 - dataSmoothing);
      bri = scale8( oldbri, dataSmoothing) + scale8( bri, 256 - dataSmoothing);
    }

    // if this palette is a 'loop', add a slowly-changing base value
    if( colorLoop) { 
      index += ihue;
    }

    bri = dim8_raw( scale8(bri, briScale) );

    CRGB color = ColorFromPalette( currentPalette, index, bri);
    leds[i] = color;
  }
  
  z += speed;
  x += speed / 2;
  y -= speed / 4;

  ihue+=1;
  
}

void loop() {
  // Periodically choose a new palette, speed, and scale
  ChangePaletteAndSettingsPeriodically();

  // run the blend function only every Nth frames
  if (countBlend == 3) {
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);
    countBlend = 0;
  } else if (countBlend == 1 && speed != newspeed) {
    speed = (1-lerpAmount)*speed + newspeed*lerpAmount;
    scale = (1-lerpAmount)*scale + newscale*lerpAmount;
  }

  countBlend++;
  mapCoordToColor();

  LEDS.show();
//   delay(20);
}


void handleButton() {
  buttonState = digitalRead(BUTTON_PIN);

  if (buttonState != prevButtonState) {
    pressCount++;
    
    // button released
    if (pressCount % 2 == 0) {
      // change pallete if pressed and released within 1 sec
      if (millis() - lastStatusSwitch < 1000) {
        isSwitchingPalette = !isSwitchingPalette;
      }

    // button pressed
    } 
    // else {}
    // record last state change
    lastStatusSwitch = millis();
  } 
  
  if (buttonState == LOW && millis()-lastStatusSwitch >= 1000) {
    // increase or decrease brightness
    if (pressCount % 4 == 1 && briScale < 255) {
      briScale++;
    } else if (pressCount % 4 == 3 && briScale > 0) {  
      briScale--;
    }

  }

  prevButtonState = buttonState;
}

#define HOLD_PALETTES_X_TIMES_AS_LONG 8

void ChangePaletteAndSettingsPeriodically()
{
  uint8_t secondHand = ((millis() / 1000) / HOLD_PALETTES_X_TIMES_AS_LONG) % 60;
  static uint8_t lastSecond = 99;
  
  if( lastSecond != secondHand && isSwitchingPalette) {
    lastSecond = secondHand;
    if( secondHand == 0)  { targetPalette = LavaColors_p;            speed =  7 * speedFactor; scale = 7 * scaleFactor; colorLoop = 0; }
    if( secondHand == 6)  { SetupBlackAndWhiteStripedPalette();       speed = 50 * speedFactor; scale = 5 * scaleFactor; colorLoop = 1; }
    if( secondHand ==  15)  { SetupPurpleAndGreenPalette();             speed = 1 * speedFactor; scale = 4 * scaleFactor; colorLoop = 1; }
    if( secondHand == 25)  { SetupRandomPalette();                     speed = 10 * speedFactor; scale = 7 * scaleFactor; colorLoop = 1; }
//    if( secondHand == 15)  { currentPalette = ForestColors_p;          speed =  3; scale = 8 * scaleFactor; colorLoop = 0; }
    // if( secondHand == 15)  { targetPalette = CloudColors_p;           speed =  8 * speedFactor; scale = 7 * scaleFactor; colorLoop = 0; }
    if( secondHand == 35)  { targetPalette = RainbowColors_p;         speed = 12 * speedFactor; scale = 5 * scaleFactor; colorLoop = 1; }
    if( secondHand == 40)  { SetupRandomPalette();                     speed = 7 * speedFactor; scale = 15 * scaleFactor; colorLoop = 1; }
    // if( secondHand == 30)  { targetPalette = OceanColors_p;           speed = 18 * speedFactor; scale = 25 * scaleFactor; colorLoop = 0; }
    if( secondHand == 45)  { targetPalette = PartyColors_p;           speed = 12 * speedFactor; scale = 4 * scaleFactor; colorLoop = 1; }
    if( secondHand == 53)  { SetupRandomPalette();                     speed = 25 * speedFactor; scale = 6 * scaleFactor; colorLoop = 1; }
    // if( secondHand == 55)  { targetPalette = RainbowStripeColors_p;   speed = 10 * speedFactor; scale = 4 * scaleFactor; colorLoop = 1; }
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
//  targetPalette[0] = CRGB::White;
  targetPalette[9] = CRGB::White;
//  targetPalette[11] = CRGB::White;
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
