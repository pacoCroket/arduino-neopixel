#include <FastLED.h>

// LED
#define LED_PIN     3
#define BRIGHTNESS  255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS 63
#define BUTTON_PIN 2

// the box is 29 x 24 x 38 cm
// store leds in x, y, z, cordinates
const PROGMEM uint8_t ledsArray[NUM_LEDS][3] = {{30, 80, 10}, {30, 80, 13}, {30, 80, 16}, // 0, 1, 2
{9, 80, 16}, {7, 80, 14}, {5, 80, 12},
{4, 60, 20}, {6, 60, 22}, {9, 60, 24},
{10, 37, 24}, {8, 36, 22}, {6, 32, 20},
{4, 22, 16}, {5, 22, 20}, {9, 22, 20}, 
{28, 10, 20}, {29, 8, 17}, {30, 6, 14},
{50, 9, 19}, {52, 9, 17}, {54, 9, 15}, 
{55, 30, 15}, {53, 30, 13}, {51, 30, 14},
{51, 55, 14}, {53, 55, 12}, {55, 55, 10},
{52, 70, 10}, {54, 73, 8}, {52, 70, 6},
{28, 70, 2}, {25, 70, 2}, {22, 70, 2},
{10, 60, 3}, {8, 58, 2}, {10, 56, 1},
{11, 40, 0}, {14, 40, 1}, {17, 40, 2},
{40, 49, 8}, {40, 46, 8}, {40, 43, 8},
{53, 26, 8}, {53, 23, 8}, {53, 20, 8},
{40, 10, 8}, {43, 10, 7}, {46, 10, 8},
{25, 23, 10}, {25, 26, 10}, {25, 29, 10},
{10, 18, 2}, {10, 15, 2}, {10, 12, 2},
{10, 1, 13}, {10, 3, 16}, {10, 5, 19},
{32, 30, 22}, {31, 33, 21}, {30, 36, 20},
{40, 60, 18}, {40, 63, 19}, {40, 66, 20}}; // 60, 61, 62
// The leds
CRGB leds[NUM_LEDS];

static double x;
static double y;
static double z;

double speedFactor = 0.1;
double speed = 6 * speedFactor; // speed is set dynamically once we've started up
double scaleFactor = 0.4; 
double scale = 6 * scaleFactor; // scale is set dynamically once we've started up
uint8_t       colorLoop = 1;

 // for blending in palettes smoothly
uint8_t maxChanges = 48;
uint8_t countBlend = 0;
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
  static uint8_t ihue=0;
    
  uint8_t dataSmoothing = 0;
  // TODO what speed?
  if( speed < 2) {
    dataSmoothing = 200 - (speed * 4);
  }

  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    // first value is the radius
    
    uint16_t xoffset = pgm_read_byte(&(ledsArray[i][0])) * scale;
    uint16_t yoffset = pgm_read_byte(&(ledsArray[i][1])) * scale;
    uint16_t zoffset = pgm_read_byte(&(ledsArray[i][2])) * scale;
    
    uint8_t index = inoise8(x + xoffset, y + yoffset, z + zoffset);
    uint8_t bri = inoise8(x + yoffset, y + xoffset, z + zoffset); // another random point for brightness

    if( dataSmoothing ) {
        uint8_t oldIndex = inoise8(x + xoffset - speed/2, y + yoffset + speed/4, z-speed);
        uint8_t oldBri = inoise8(x + yoffset - speed/2, y + xoffset + speed/4, z-speed);
        index = scale8( oldIndex, dataSmoothing) + scale8( oldIndex, 256 - dataSmoothing);
        bri = scale8( oldBri, dataSmoothing) + scale8( oldBri, 256 - dataSmoothing);
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
  
  // apply slow drift to X and Y, just for visual variation.
  x += speed / 4;
  y -= speed / 8;

  ihue+=1;
  
}

void loop() {
  // handleButton();

  // Periodically choose a new palette, speed, and scale
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
    if( secondHand == 5)  { SetupBlackAndWhiteStripedPalette();       speed = 35 * speedFactor; scale = 5 * scaleFactor; colorLoop = 1; }
    if( secondHand ==  10)  { SetupPurpleAndGreenPalette();             speed = 1 * speedFactor; scale = 4 * scaleFactor; colorLoop = 1; }
//    if( secondHand == 15)  { currentPalette = ForestColors_p;          speed =  3; scale = 8 * scaleFactor; colorLoop = 0; }
    if( secondHand == 15)  { targetPalette = CloudColors_p;           speed =  8 * speedFactor; scale = 7 * scaleFactor; colorLoop = 0; }
    if( secondHand == 25)  { targetPalette = RainbowColors_p;         speed = 15 * speedFactor; scale = 5 * scaleFactor; colorLoop = 1; }
    if( secondHand == 30)  { targetPalette = OceanColors_p;           speed = 20 * speedFactor; scale = 25 * scaleFactor; colorLoop = 0; }
    if( secondHand == 35)  { targetPalette = PartyColors_p;           speed = 15 * speedFactor; scale = 4 * scaleFactor; colorLoop = 1; }
    if( secondHand == 40)  { SetupRandomPalette();                     speed = 10 * speedFactor; scale = 7 * scaleFactor; colorLoop = 1; }
    if( secondHand == 45)  { SetupRandomPalette();                     speed = 7 * speedFactor; scale = 15 * scaleFactor; colorLoop = 1; }
    if( secondHand == 50)  { SetupRandomPalette();                     speed = 25 * speedFactor; scale = 6 * scaleFactor; colorLoop = 1; }
    if( secondHand == 55)  { targetPalette = RainbowStripeColors_p;   speed = 12 * speedFactor; scale = 4 * scaleFactor; colorLoop = 1; }
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
