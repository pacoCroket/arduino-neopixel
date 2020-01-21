#include <FastLED.h>

// LED
#define LED_PIN1     3
#define LED_PIN2     5
#define BRIGHTNESS  255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB // GRB for WS2812, BRG for WS2811
#define NUM_LEDS1 50 // chritmas lights
#define NUM_LEDS2 60 // separate parts of normal strip
#define BUTTON_PIN 2

CRGB flow1Leds[NUM_LEDS1]; // chritmas light looking leds
CRGB flow2Leds[NUM_LEDS2]; // spaced normal LED strips

// store leds in [x, y] cordinates
const PROGMEM uint8_t flow1LedsArray[NUM_LEDS1][2] = {{245, 115}, {237, 114}, {225, 109}, {217, 104}, {208, 99}, {198, 95}, {189, 92}, {179, 86}, {169, 79}, {161, 70}, {153, 59}, {149, 53}, {142, 43}, {137, 35}, {132, 29}, {126, 19}, {120, 11}, {115, 0}, {105, 7}, {102, 16}, {94, 21}, {88, 29}, {81, 38}, {75, 47}, {68, 53}, {61, 53}, {52, 53}, {41, 54}, {40, 60}, {49, 66}, {55, 69}, {60, 72}, {66, 74}, {73, 76}, {79, 80}, {85, 86}, {82, 93}, {74, 94}, {66, 94}, {57, 95}, {49, 94}, {42, 94}, {32, 94}, {28, 89}, {23, 81}, {18, 76}, {13, 71}, {9, 66}, {4, 60}, {0, 55}};
const PROGMEM uint8_t flow2LedsArray[NUM_LEDS2][2] = {{255, 115}, {254, 111}, {253, 107}, {251, 104}, {250, 100}, {249, 97}, {244, 75}, {244, 71}, {244, 67}, {243, 64}, {243, 60}, {243, 56}, {239, 46}, {237, 43}, {235, 40}, {233, 36}, {231, 33}, {229, 30}, {208, 19}, {205, 17}, {201, 15}, {198, 13}, {195, 12}, {191, 10}, {173, 1}, {169, 1}, {165, 0}, {162, 0}, {158, 0}, {154, 0}, {141, 0}, {137, 0}, {133, 0}, {129, 0}, {126, 0}, {122, 0}, {103, 0}, {99, 0}, {96, 0}, {92, 0}, {88, 0}, {84, 0}, {71, 1}, {67, 1}, {63, 1}, {60, 1}, {56, 1}, {52, 1}, {40, 14}, {36, 14}, {32, 14}, {28, 14}, {24, 14}, {20, 14}, {1, 23}, {1, 26}, {1, 30}, {0, 34}, {0, 38}, {0, 42}};

static double x;
static double y;
static double z;

double speedFactor = 0.2;
double speed = 6 * speedFactor; // speed is set dynamically once we've started up
double scaleFactor = 0.5; 
double scale = 6 * scaleFactor; // scale is set dynamically once we've started up
// double rate = 1;
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
uint8_t briScale = 150;
boolean isSwitchingPalette = true;

void setup() {

  LEDS.addLeds<LED_TYPE,LED_PIN1,COLOR_ORDER>(flow1Leds,NUM_LEDS1);
  LEDS.addLeds<LED_TYPE,LED_PIN2,COLOR_ORDER>(flow2Leds,NUM_LEDS2);
  LEDS.setBrightness(BRIGHTNESS);

  // Initialize our coordinates to some random values
  x = random8();
  y = random8();
  z = random8();

  // initialize the pushbutton pin as an input:
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void flowMapping(int numLeds, uint8_t ledArray[][2], CRGB leds[]) {
  static uint8_t ihue=0;

  uint8_t dataSmoothing = 0;
  // TODO what speed?
  if( speed < 2) {
    dataSmoothing = 200 - (speed * 8);
  }

  for (int i = 0; i < numLeds; i++) {
    // first value is the radius
    
    uint16_t xoffset = pgm_read_byte(&(ledArray[i][0])) * scale;
    uint16_t yoffset = pgm_read_byte(&(ledArray[i][1])) * scale;
    
    uint8_t index = inoise8(x + xoffset, y + yoffset, z);
    uint8_t bri = inoise8(x + yoffset, y + xoffset, z); // another random point for brightness

    if( dataSmoothing ) {
        uint8_t oldIndex = inoise8(x + xoffset + speed, y + yoffset, z-speed/8);
        uint8_t oldBri = inoise8(x + yoffset + speed, y + xoffset, z-speed/8);
        index = scale8( oldIndex, dataSmoothing) + scale8( oldIndex, 256 - dataSmoothing);
        bri = scale8( oldBri, dataSmoothing) + scale8( oldBri, 256 - dataSmoothing);
    }

    // if this palette is a 'loop', add a slowly-changing base value
    if( colorLoop) { 
      index += ihue;
    }

    bri = dim8_raw( scale8(bri, briScale) );

    CRGB color = ColorFromPalette(currentPalette, index, bri);
    leds[i] = color;
  }
  ihue++;
}

void loop() {
  handleButton();

  // Periodically choose a new palette, speed, and scale
  ChangePaletteAndSettingsPeriodically();

  // run the blend function only every Nth frames
  if (countBlend == 3) {
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);
    countBlend = 0;
  }
  countBlend++;

  flowMapping(NUM_LEDS1, flow1LedsArray, flow1Leds);
  flowMapping(NUM_LEDS2, flow2LedsArray, flow2Leds);
    
  z += speed/8;
  x -= speed;

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
