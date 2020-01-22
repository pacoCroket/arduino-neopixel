#include <FastLED.h>

// LED
// #define LED_PIN1     3
#define LED_PIN2     3
#define BRIGHTNESS  255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB // GRB for WS2812, BRG for WS2811
// #define NUM_LEDS1 240
#define NUM_LEDS2 48
#define BUTTON_PIN 2

// CRGB lavaLeds[NUM_LEDS1];
CRGB cloudLeds[NUM_LEDS2];

// store leds in [x, y] cordinates
// const PROGMEM uint8_t lavaLedsArray[NUM_LEDS1][2] = {{30, 80}};
// cloud is 3D
const PROGMEM uint8_t cloudLedsArray[NUM_LEDS2][3] = {
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

// static double x1;
// static double y1;
// static double z1;
static double x2;
static double y2;
static double z2;

double speedFactor = 0.08;
double speed = 6 * speedFactor; // speed is set dynamically once we've started up
double newspeed = speed;
double scaleFactor = 0.3; 
double scale = 6 * scaleFactor; // scale is set dynamically once we've started up
double newscale = scale;
double flow = 5;
uint8_t colorLoop = 1;

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
  } else if (countBlend == 1 && speed != newspeed) {
    speed = (1-lerpAmount)*speed + newspeed*lerpAmount;
    scale = (1-lerpAmount)*scale + newscale*lerpAmount;
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
    if( secondHand == 0)  { targetPalette = LavaColors_p;            newspeed =  7 * speedFactor; newscale = 7 * scaleFactor; colorLoop = 0; }
    if( secondHand == 5)  { SetupBlackAndWhiteStripedPalette();       newspeed = 35 * speedFactor; newscale = 5 * scaleFactor; colorLoop = 1; }
    if( secondHand ==  10)  { SetupPurpleAndGreenPalette();             newspeed = 1 * speedFactor; newscale = 4 * scaleFactor; colorLoop = 1; }
    if( secondHand == 20)  { SetupRandomPalette();                     newspeed = 8 * speedFactor; newscale = 7 * scaleFactor; colorLoop = 1; }
//    if( secondHand == 15)  { currentPalette = ForestColors_p;          speed =  3; scale = 8 * scaleFactor; colorLoop = 0; }
    if( secondHand == 25)  { targetPalette = CloudColors_p;           newspeed =  8 * speedFactor; newscale = 7 * scaleFactor; colorLoop = 0; }
    if( secondHand == 30)  { targetPalette = RainbowColors_p;         newspeed = 12 * speedFactor; newscale = 5 * scaleFactor; colorLoop = 1; }
    if( secondHand == 35)  { SetupRandomPalette();                     newspeed = 5 * speedFactor; newscale = 15 * scaleFactor; colorLoop = 1; }
    if( secondHand == 40)  { targetPalette = OceanColors_p;           newspeed = 15 * speedFactor; newscale = 25 * scaleFactor; colorLoop = 0; }
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
