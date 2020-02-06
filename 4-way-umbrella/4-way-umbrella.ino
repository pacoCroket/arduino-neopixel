#include <Bounce2.h>
#include <FastLED.h>

// LED
#define LED_PIN     3
#define BRIGHTNESS  255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB // GRB for WS2812, BRG for WS2811
#define NUM_LEDS 40
#define BUTTON_PIN 2

// The leds
CRGB leds[NUM_LEDS];

static double x;
static double z;

double speedFactor = 0.15;
double speed = 6 * speedFactor; // speed is set dynamically once we've started up
double newspeed = speed;
double scaleFactor = 3; 
double scale = 6 * scaleFactor; // scale is set dynamically once we've started up
double newscale = scale;
uint8_t       colorLoop = 1;

 // for blending in palettes smoothly
uint8_t maxChanges = 48;
uint8_t countBlend = 0;
float lerpAmount = 0.1;
CRGBPalette16 targetPalette( LavaColors_p );
CRGBPalette16 currentPalette( LavaColors_p );

// Instantiate a Bounce object
Bounce debouncer = Bounce(); 

boolean buttonState = LOW;
boolean prevButtonState = LOW;
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
  z = random8();

  // Setup the button with an internal pull-up :
  pinMode(BUTTON_PIN,INPUT_PULLUP);

  // After setting up the button, setup the Bounce instance :
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(10); // interval in ms
}

void handleButton() {
    // Update the Bounce instance :
  debouncer.update();

  // Get the updated value :
  buttonState = debouncer.read();

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

void mapCoordToColor() {
  static uint8_t ihue=0;
    
  uint8_t dataSmoothing = 0;
  if( speed < 4) {
    dataSmoothing = 200 - (speed * 4);
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    // first value is the radius
    
    uint16_t xoffset = i * scale;
    
    uint8_t index = inoise8(x + xoffset, z);
    uint8_t bri = inoise8(x, z + xoffset); // another random point for brightness

    if( dataSmoothing ) {
      uint8_t oldindex = inoise8(x + xoffset - speed / 2,z-speed/2);
      uint8_t oldbri = inoise8(x - speed / 2,z-speed/2 + xoffset);
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
  
  z += speed/2;
  // apply slow drift to X and Y, just for visual variation.
  x -= speed / 2;

  ihue++;
}

void loop() {
  handleButton();

  // Periodically choose a new palette, speed, and scale
  ChangePaletteAndSettingsPeriodically();

  // run the blend function only every Nth frames
  if (countBlend == 5) {
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
    if( secondHand == 35)  { SetupRandomPalette();                     newspeed = 7 * speedFactor; newscale = 15 * scaleFactor; colorLoop = 1; }
    if( secondHand == 40)  { targetPalette = OceanColors_p;           newspeed = 20 * speedFactor; newscale = 25 * scaleFactor; colorLoop = 0; }
    if( secondHand == 45)  { targetPalette = PartyColors_p;           newspeed = 12 * speedFactor; newscale = 4 * scaleFactor; colorLoop = 1; }
    if( secondHand == 50)  { SetupRandomPalette();                     newspeed = 22 * speedFactor; newscale = 6 * scaleFactor; colorLoop = 1; }
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
