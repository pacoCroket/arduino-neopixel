#include <FastLED.h>

// LED
// #define LED_PIN1     3
#define LED_PIN2     3
#define BRIGHTNESS  255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB // GRB for WS2812, BRG for WS2811
// #define NUM_LEDS1 240
#define NUM_LEDS2 42
#define BUTTON_PIN 2

// CRGB lavaLeds[NUM_LEDS1];
CRGB cloudLeds[NUM_LEDS2];

// store leds in [x, y] cordinates
// const PROGMEM uint8_t lavaLedsArray[NUM_LEDS1][2] = {{30, 80}};
// cloud is 3D
const PROGMEM uint8_t cloudLedsArray[NUM_LEDS2][3] = {
{136, 0, 60}, {147, 7, 55}, {158, 15, 60}, //0
{205, 16, 15}, {221, 17, 22}, {236, 18, 35},
{181, 78, 120}, {181, 88, 100}, {181, 98, 80}, // 6
{242, 83, 60}, {255, 95, 60}, {244, 105, 60},
{153, 120, 0}, {155, 70, 15}, {202, 118, 30}, // 12
{184, 163, 35}, {184, 174, 43}, {184, 188, 50},
{130, 170, 75}, {117, 170, 85}, {103, 170, 82}, // 18
{36, 149, 60}, {23, 165, 45}, {37, 185, 60},
{34, 129, 55}, {14, 120, 55}, {37, 104, 55}, // 24
{47, 103, 70}, {44, 94, 85}, {43, 84, 100},
{2, 35, 60}, {0, 10, 60}, // 30
{46, 36, 48}, {47, 48, 35}, // 32
{16, 82, 0}, {53, 91, 15}, {76, 76, 30}, // 34
{82, 92, 30}, {96, 92, 20}, {108, 93, 10}, // 37
{80, 16, 35}, {92, 15, 45}}; // 40

// static double x1;
// static double y1;
// static double z1;
static double x2;
static double y2;
static double z2;

double speedFactor = 0.1;
double speed = 6 * speedFactor; // speed is set dynamically once we've started up
double scaleFactor = 0.3; 
double scale = 6 * scaleFactor; // scale is set dynamically once we've started up
double flow = 5;
uint8_t colorLoop = 1;

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
  
  // lava LEDs
  // LEDS.addLeds<LED_TYPE,LED_PIN1,COLOR_ORDER>(lavaLeds,NUM_LEDS1);
  // cloud LEDs
  LEDS.addLeds<LED_TYPE,LED_PIN2,COLOR_ORDER>(cloudLeds,NUM_LEDS2);
  LEDS.setBrightness(BRIGHTNESS);

  // Initialize our coordinates to some random values
  x2 = random8();
  y2 = random8();
  z2 = random8();

  // initialize the pushbutton pin as an input:
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void cloudMapping() {
    static uint8_t ihue=0;

  for (int i = 0; i < NUM_LEDS2; i++) {
    // first value is the radius
    
    uint16_t xoffset = pgm_read_byte(&(cloudLedsArray[i][0])) * scale;
    uint16_t yoffset = pgm_read_byte(&(cloudLedsArray[i][1])) * scale;
    uint16_t zoffset = pgm_read_byte(&(cloudLedsArray[i][2])) * scale;
    
    uint8_t index = inoise8(x2 + xoffset, y2 + yoffset, z2 + zoffset);
    uint8_t bri = inoise8(x2 + zoffset, y2 + xoffset, z2 + yoffset); // another random point for brightness

    // if this palette is a 'loop', add a slowly-changing base value
    if( colorLoop) { 
      index += ihue;
    }

    bri = dim8_raw( scale8(bri, briScale) );

    CRGB color = ColorFromPalette( currentPalette, index, bri);
    cloudLeds[i] = color;
  }
  
  z2 += speed;
  
  // apply slow drift to X and Y, just for visual variation.
  x2 += speed / 4;
  y2 -= speed / 8;

  ihue++;
}

void loop() {
//   handleButton();

  // Periodically choose a new palette, speed, and scale
  ChangePaletteAndSettingsPeriodically();

  // run the blend function only every Nth frames
  if (countBlend == 3) {
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);
    countBlend = 0;
  }
  countBlend++;

  cloudMapping();
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
