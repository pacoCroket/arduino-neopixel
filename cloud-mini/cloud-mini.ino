#include <FastLED.h>

// LED
#define LED_PIN     3
#define BRIGHTNESS  255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS 36
#define BUTTON_PIN 2

// the box is 29 x 24 x 38 cm
// store leds in x, y, z, cordinates
const PROGMEM int ledsArray[NUM_LEDS][3] = {
    {119, 128, 185}, {137, 128, 189}, {157, 127, 191}, {206, 110, 205}, {190, 99, 215}, {172, 86, 230},
    {134, 53, 215}, {134, 36, 190}, {133, 23, 158}, {168, 54, 100}, {186, 54, 92}, {207, 55, 86},
    {237, 7, 125}, {230, 0, 145}, {225, 10, 160}, {234, 49, 160}, {245, 60, 150}, {255, 73, 125},
    {187, 150, 118}, {169, 143, 109}, {150, 138, 86}, {79, 138, 120}, {61, 128, 125}, {46, 121, 130},
    {45, 101, 215}, {63, 88, 210}, {82, 77, 220}, {66, 46, 210}, {53, 31, 180}, {46, 27, 150},
    {0, 78, 120}, {7, 72, 103}, {22, 78, 84}, {92, 80, 84}, {91, 60, 75}, {90, 41, 88}
    };

// The leds
CRGB leds[NUM_LEDS];

// The 8 bit version of our coordinates
static double x;
static double y;
static double z;

double speedFactor = 0.08;
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
    if( secondHand == 5)  { SetupBlackAndWhiteStripedPalette();       speed = 50 * speedFactor; scale = 5 * scaleFactor; colorLoop = 1; }
    if( secondHand ==  10)  { SetupPurpleAndGreenPalette();             speed = 1 * speedFactor; scale = 4 * scaleFactor; colorLoop = 1; }
    if( secondHand == 40)  { SetupRandomPalette();                     speed = 10 * speedFactor; scale = 7 * scaleFactor; colorLoop = 1; }
//    if( secondHand == 15)  { currentPalette = ForestColors_p;          speed =  3; scale = 8 * scaleFactor; colorLoop = 0; }
    if( secondHand == 15)  { targetPalette = CloudColors_p;           speed =  8 * speedFactor; scale = 7 * scaleFactor; colorLoop = 0; }
    if( secondHand == 25)  { targetPalette = RainbowColors_p;         speed = 12 * speedFactor; scale = 5 * scaleFactor; colorLoop = 1; }
    if( secondHand == 45)  { SetupRandomPalette();                     speed = 7 * speedFactor; scale = 15 * scaleFactor; colorLoop = 1; }
    if( secondHand == 30)  { targetPalette = OceanColors_p;           speed = 18 * speedFactor; scale = 25 * scaleFactor; colorLoop = 0; }
    if( secondHand == 35)  { targetPalette = PartyColors_p;           speed = 12 * speedFactor; scale = 4 * scaleFactor; colorLoop = 1; }
    if( secondHand == 50)  { SetupRandomPalette();                     speed = 25 * speedFactor; scale = 6 * scaleFactor; colorLoop = 1; }
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
