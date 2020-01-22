#include <FastLED.h>

// LED
#define LED_PIN     3
#define BRIGHTNESS  255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS 61
#define BUTTON_PIN 2

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

double speedFactor = 0.8;
double speed = 6 * speedFactor; // speed is set dynamically once we've started up
double newspeed = speed;
double scaleFactor = 0.6; 
double scale = 6 * scaleFactor; // scale is set dynamically once we've started up
double newscale = scale;
uint8_t       colorLoop = 1;

 // for blending in palettes smoothly
uint8_t maxChanges = 48;
uint8_t countBlend = 0;
float lerpAmount = 0.2;
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
  if( speed < 10) {
    dataSmoothing = 200 - (speed * 4);
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
      index = scale8( oldindex, dataSmoothing) + scale8( index, 256 - dataSmoothing);
      uint8_t oldbri = inoise8(x + zoffset - speed / 2,y + xoffset + speed / 4,z + zoffset-speed);
      bri = scale8( oldbri, dataSmoothing) + scale8( index, 256 - dataSmoothing);
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
    if( secondHand == 0)  { targetPalette = LavaColors_p;            newspeed =  7 * speedFactor; newscale = 7 * scaleFactor; colorLoop = 0; }
    if( secondHand == 5)  { SetupBlackAndWhiteStripedPalette();       newspeed = 35 * speedFactor; newscale = 5 * scaleFactor; colorLoop = 1; }
    if( secondHand ==  10)  { SetupPurpleAndGreenPalette();             newspeed = 1 * speedFactor; newscale = 4 * scaleFactor; colorLoop = 1; }
    if( secondHand == 20)  { SetupRandomPalette();                     newspeed = 7 * speedFactor; newscale = 7 * scaleFactor; colorLoop = 1; }
//    if( secondHand == 15)  { currentPalette = ForestColors_p;          speed =  3; scale = 8 * scaleFactor; colorLoop = 0; }
    if( secondHand == 25)  { targetPalette = CloudColors_p;           newspeed =  8 * speedFactor; newscale = 7 * scaleFactor; colorLoop = 0; }
    if( secondHand == 30)  { targetPalette = RainbowColors_p;         newspeed = 12 * speedFactor; newscale = 5 * scaleFactor; colorLoop = 1; }
    if( secondHand == 35)  { SetupRandomPalette();                     newspeed = 7 * speedFactor; newscale = 15 * scaleFactor; colorLoop = 1; }
    if( secondHand == 40)  { targetPalette = OceanColors_p;           newspeed = 20 * speedFactor; newscale = 25 * scaleFactor; colorLoop = 0; }
    if( secondHand == 45)  { targetPalette = PartyColors_p;           newspeed = 12 * speedFactor; newscale = 4 * scaleFactor; colorLoop = 1; }
    if( secondHand == 50)  { SetupRandomPalette();                     newspeed = 20 * speedFactor; newscale = 6 * scaleFactor; colorLoop = 1; }
    if( secondHand == 55)  { targetPalette = RainbowStripeColors_p;   newspeed = 10 * speedFactor; newscale = 4 * scaleFactor; colorLoop = 1; }
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
//  targetPalette[9] = CRGB::White;
  targetPalette[11] = CRGB::White;
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
