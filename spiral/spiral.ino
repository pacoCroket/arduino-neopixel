#include <Bounce2.h>
#include <FastLED.h>

// LED
#define LED_PIN 3
#define BRIGHTNESS 255
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB // GRB for WS2812, BRG for WS2811
#define NUM_LEDS 100
#define BUTTON_PIN 2

// The leds
CRGB leds[NUM_LEDS];

static int x;
static int z;

#define HOLD_PALETTES_X_TIMES_AS_LONG 15 // 15
uint16_t delayTime = 30;
double hueSpeed = 0.5;
double speedFactor = 0.25;
double scaleFactor = 2;
//
double speed = 4 * speedFactor; // speed is set dynamically once we've started up
double newspeed = speed;
double scale = 4 * scaleFactor; // scale is set dynamically once we've started up
double newscale = scale;
uint8_t colorLoop = 0;

// for blending in palettes smoothly
uint8_t maxChanges = 200;
float lerpAmount = 0.0001;
CRGBPalette16 targetPalette(LavaColors_p);
CRGBPalette16 currentPalette(LavaColors_p);

// Instantiate a Bounce object
Bounce debouncer = Bounce();

boolean buttonState = LOW;
boolean prevButtonState = LOW;
uint8_t pressCount = 0;
unsigned long lastStatusSwitch = 999999;
uint8_t briScale = 255;
boolean isSwitchingPalette = true;

void setup()
{
  //  Serial.begin(9600);

  LEDS.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  LEDS.setBrightness(BRIGHTNESS);

  // Initialize our coordinates to some random values
  x = random8();
  z = random8();

  // Setup the button with an internal pull-up :
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // After setting up the button, setup the Bounce instance :
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(10); // interval in ms
}

void handleButton()
{
  // Update the Bounce instance :
  debouncer.update();

  // Get the updated value :
  buttonState = debouncer.read();

  if (buttonState != prevButtonState)
  {
    pressCount++;

    // button released
    if (pressCount % 2 == 0)
    {
      // change pallete if pressed and released within 1 sec
      if (millis() - lastStatusSwitch < 1000)
      {
        isSwitchingPalette = !isSwitchingPalette;
      }

      // button pressed
    }
    // else {}
    // record last state change
    lastStatusSwitch = millis();
  }

  if (buttonState == LOW && millis() - lastStatusSwitch >= 1000)
  {
    // increase or decrease brightness
    if (pressCount % 4 == 1 && briScale < 255)
    {
      briScale++;
    }
    else if (pressCount % 4 == 3 && briScale > 0)
    {
      briScale--;
    }
  }

  prevButtonState = buttonState;
}

void mapCoordToColor()
{
  static double ihue = 0;

  uint8_t dataSmoothing = 0;
  if (speed < 50)
  {
    dataSmoothing = 200 - (speed * 4);
  }

  for (int i = 0; i < NUM_LEDS; i++)
  {
    // first value is the radius

    uint8_t xoffset = i * scale;

    uint8_t index = inoise8(x + xoffset, z);
    uint8_t bri = inoise8(z - xoffset, x); // another random point for brightness

    // The range of the inoise8 function is roughly 16-238.
    // These two operations expand those values out to roughly 0..255
    // You can comment them out if you want the raw noise data.
    // index = qsub8(index, 16);
    // index = qadd8(index, scale8(index, 39));
    // bri = qsub8(bri, 16);
    // bri = qadd8(bri, scale8(bri, 39));

    if (dataSmoothing)
    {
      uint8_t oldindex = inoise8(x + xoffset - speed / 2, z - speed);
      index = scale8(oldindex, dataSmoothing) + scale8(index, 256 - dataSmoothing);
      uint8_t oldbri = inoise8(z - speed - xoffset, x - speed / 2);
      bri = scale8(oldbri, dataSmoothing) + scale8(bri, 256 - dataSmoothing);
    }

    // if this palette is a 'loop', add a slowly-changing base value
    if (colorLoop)
    {
      index += ihue;
    }

    // brighten up, as the color palette itself often contains the
    // light/dark dynamic range desired
    if (bri > 64)
    {
      bri = 255;
    }
    else
    {
      bri = dim8_raw(bri * 4);
    }

    CRGB color = ColorFromPalette(currentPalette, index, bri);
    leds[i] = color;
  }

  z += speed;
  x += speed / 2;

  ihue += hueSpeed;
}

void loop()
{
  // handleButton();

  // Periodically choose a new palette, speed, and scale
  ChangePaletteAndSettingsPeriodically();
  nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);
  // run the blend function only every Nth frames
  if (speed != newspeed || scale != newscale)
  {
    speed = (1 - lerpAmount) * speed + newspeed * lerpAmount;
    scale = (1 - lerpAmount) * scale + newscale * lerpAmount;
  }

  mapCoordToColor();

  LEDS.show();
  FastLED.delay(delayTime);
}

void ChangePaletteAndSettingsPeriodically()
{
  uint8_t secondHand = ((millis() / 1000) / HOLD_PALETTES_X_TIMES_AS_LONG) % 60;
  static uint8_t lastSecond = 99;

  if (lastSecond != secondHand && isSwitchingPalette)
  {
    lastSecond = secondHand;
    if (secondHand == 0)
    {
      targetPalette = LavaColors_p;
      newspeed = 4 * speedFactor;
      newscale = 4 * scaleFactor;
    }
    // if (secondHand == 5)
    // {
    //   SetupBlackAndWhiteStripedPalette();
    //   newspeed = 35 * speedFactor;
    //   newscale = 5 * scaleFactor;
    //   colorLoop = 0;
    // }
    if (secondHand == 8)
    {
      SetupPurpleAndGreenPalette();
      newspeed = 4 * speedFactor;
      newscale = 6 * scaleFactor;
      // colorLoop = 1;
    }
    if (secondHand == 15)
    {
      SetupRandomPalette();
      newspeed = 8 * speedFactor;
      newscale = 4 * scaleFactor;
      colorLoop = 1;
    }
    //    if( secondHand == 15)  { currentPalette = ForestColors_p;          speed =  3; scale = 8 * scaleFactor; colorLoop = 0; }
    // if (secondHand == 25)
    // {
    //   targetPalette = CloudColors_p;
    //   newspeed = 8 * speedFactor;
    //   newscale = 7 * scaleFactor;
    //   colorLoop = 1;
    // }
    if (secondHand == 25)
    {
      targetPalette = RainbowColors_p;
      newspeed = 8 * speedFactor;
      newscale = 8 * scaleFactor;
      colorLoop = 1;
    }
    if (secondHand == 35)
    {
      SetupRandomPalette();
      newspeed = 4 * speedFactor;
      newscale = 16 * scaleFactor;
      colorLoop = 1;
    }
    if (secondHand == 40)
    {
      targetPalette = OceanColors_p;
      newspeed = 16 * speedFactor;
      newscale = 24 * scaleFactor;
      // colorLoop = 1;
    }
    if (secondHand == 45)
    {
      targetPalette = PartyColors_p;
      newspeed = 8 * speedFactor;
      newscale = 4 * scaleFactor;
      colorLoop = 1;
    }
    if (secondHand == 50)
    {
      SetupRandomPalette();
      newspeed = 4 * speedFactor;
      newscale = 6 * scaleFactor;
      colorLoop = 1;
    }
    if (secondHand == 55)
    {
      targetPalette = RainbowStripeColors_p;
      newspeed = 12 * speedFactor;
      newscale = 4 * scaleFactor;
      colorLoop = 1;
    }
  }
}

void SetupRandomPalette()
{
  targetPalette = CRGBPalette16(
      CHSV(random8(), 255, 32),
      CHSV(random8(), 255, 255),
      CHSV(random8(), 128, 255),
      CHSV(random8(), 255, 255));
}

void SetupBlackAndWhiteStripedPalette()
{
  // 'black out' all 16 palette entries...
  fill_solid(targetPalette, 16, CRGB::Black);
  // and set every fourth one to white.
  targetPalette[0] = CRGB::White;
  //  targetPalette[9] = CRGB::White;
  targetPalette[11] = CRGB::White;
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
  CRGB purple = CHSV(HUE_PURPLE, 255, 255);
  CRGB green = CHSV(HUE_GREEN, 255, 255);
  CRGB black = CRGB::Black;

  targetPalette = CRGBPalette16(
      green, green, black, black,
      purple, purple, black, black,
      green, green, black, black,
      purple, purple, black, black);
}
