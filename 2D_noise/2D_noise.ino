#include <FastLED.h>

// LED
#define LED_PIN 3
#define BRIGHTNESS 255
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB // GRB for WS2812, BRG for WS2811

// LED position array from https://noise2led.firebaseapp.com/
// COPY HERE !
#define NUM_LEDS 3 // !! the same number of LEDs as entries in the array
const PROGMEM int ledsArray[NUM_LEDS][2] = {{5, 193}, {6, 167}, {8, 146}};

/////// SETTINGS

#define HOLD_PALETTES_X_TIMES_AS_LONG 8 // duration factor of a pattern
#define UPDATES_PER_SECOND 30           // refresh rate
double speedFactor = 1.0 / 8;          // global speed
double scaleFactor = 1.0 / 2 ;                 // global scale
uint8_t maxChanges = 5;                 // palette blend (smaller slower)
uint8_t lerpDuration = 20;              // seconds of transition for speed and scale

/////// Initial values
CRGBPalette16 currentPalette(RainbowColors_p); // initial palette
boolean isSwitchingPalette = true; // true for transitioning patterns, false for only the initial
double speed = 6 * speedFactor; // speed is set dynamically with each new pattern
double scale = 4 * scaleFactor; // scale is set dynamically with each new pattern

///////
CRGB leds[NUM_LEDS];

static double x;
static double y;

double newscale = scale;
double newspeed = speed;
uint8_t colorLoop = 1;
// lerp variables
double lerpSpeed = 1.0 / (UPDATES_PER_SECOND * lerpDuration);
double scaleDiff = 0;
double speedDiff = 0;
double speedStepSize = 0;
double scaleStepSize = 0;
double lerpStepCurrent = 0;

// for blending in palettes smoothly
CRGBPalette16 targetPalette(currentPalette);

void setup()
{
    //  Serial.begin(9600);

    LEDS.setMaxPowerInVoltsAndMilliamps(5, 1000);
    LEDS.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    LEDS.setBrightness(BRIGHTNESS);

    // Initialize our coordinates to some random values
    x = random8();
    y = random8();
}

void mapCoordToColor()
{
    static uint8_t ihue = 0;

    uint8_t dataSmoothing = 0;
    if (speed < 8)
    {
        dataSmoothing = 220 - (speed * 4);
    }

    for (int i = 0; i < NUM_LEDS; i++)
    {
        double fractionOffset = 0.125 * (i % 8);
        double xoffset = pgm_read_byte(&(ledsArray[i][0])) * scale + fractionOffset;
        double yoffset = pgm_read_byte(&(ledsArray[i][1])) * scale + fractionOffset;

        uint8_t index = inoise8(x + xoffset, y + yoffset);
        uint8_t bri = inoise8(x + yoffset, y + xoffset); // another random point for brightness

        if (dataSmoothing)
        {
            uint8_t oldindex = inoise8(x + xoffset - speed, y + yoffset - speed);
            index = scale8(oldindex, dataSmoothing) + scale8(index, 256 - dataSmoothing);
            uint8_t oldbri = inoise8(x + yoffset - speed, y + xoffset - speed);
            bri = scale8(oldbri, dataSmoothing) + scale8(index, 256 - dataSmoothing);
        }

        // if this palette is a 'loop', add a slowly-changing base value
        // if (colorLoop)
        // {
        //     index += ihue;
        // }

        if (bri > 127)
        {
            bri = 255;
        }
        else
        {
            bri = dim8_raw(bri * 2);
        }

        CRGB color = ColorFromPalette(currentPalette, index, bri);
        leds[i] = color;
    }

    x += speed;
    y += speed;

    ihue++;
}

void loop()
{
    // Periodically choose a new palette, speed, and scale
    ChangePaletteAndSettingsPeriodically();

    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);

    if (lerpStepCurrent < 1)
    {
        if (lerpStepCurrent < 0.5)
        {

            speed += 4 * speedStepSize * lerpStepCurrent;
            scale += 4 * scaleStepSize * lerpStepCurrent;
        }
        else
        {
            speed += 4 * speedStepSize * (1 - lerpStepCurrent);
            scale += 4 * scaleStepSize * (1 - lerpStepCurrent);
        }
        lerpStepCurrent += lerpSpeed;
    }
    else
    {
        speed = newspeed;
        scale = newscale;
    }

    mapCoordToColor();
    LEDS.show();
    LEDS.delay(1000 / UPDATES_PER_SECOND);
}

void setPattern(CRGBPalette16 _targetPallete, double _newspeed, double _newscale, uint8_t _colorLoop)
{
    targetPalette = _targetPallete;
    newspeed = _newspeed * speedFactor;
    newscale = _newscale * scaleFactor;
    colorLoop = _colorLoop;
    // lerp variables
    lerpStepCurrent = 0;
    speedDiff = newspeed - speed;
    scaleDiff = newscale - scale;
    speedStepSize = speedDiff * lerpSpeed;
    scaleStepSize = scaleDiff * lerpSpeed;
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
            setPattern(LavaColors_p, 4, 4, 0);
        }
        if (secondHand == 8)
        {
            setPattern(getPurpleAndGreenPalette(), 4, 6, 1);
        }
        if (secondHand == 15)
        {
            setPattern(getRandomPalette(), 6, 2, 1);
        }
        if (secondHand == 25)
        {
            setPattern(RainbowColors_p, 6, 4, 1);
        }
        if (secondHand == 35)
        {
            setPattern(getRandomPalette(), 4, 8, 1);
        }
        if (secondHand == 40)
        {
            setPattern(OceanColors_p, 8, 16, 1);
        }
        if (secondHand == 45)
        {
            setPattern(PartyColors_p, 6, 4, 1);
        }
        if (secondHand == 50)
        {
            setPattern(getRandomPalette(), 4, 2, 1);
        }
        if (secondHand == 55)
        {
            setPattern(RainbowStripeColors_p, 4, 4, 1);
        }
    }
}

CRGBPalette16 getRandomPalette()
{
    return CRGBPalette16(
        CHSV(random8(), 255, 32),
        CHSV(random8(), 255, 255),
        CHSV(random8(), 128, 255),
        CHSV(random8(), 255, 255));
}

CRGBPalette16 getBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid(targetPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    targetPalette[0] = CRGB::White;
    //  targetPalette[9] = CRGB::White;
    targetPalette[11] = CRGB::White;
    return targetPalette;
}

// This function sets up a palette of purple and green stripes.
CRGBPalette16 getPurpleAndGreenPalette()
{
    CRGB purple = CHSV(HUE_PURPLE, 255, 255);
    CRGB green = CHSV(HUE_GREEN, 255, 255);
    CRGB black = CRGB::Black;

    return CRGBPalette16(
        green, green, black, black,
        purple, purple, black, black,
        green, green, black, black,
        purple, purple, black, black);
}

CRGBPalette16 getPurpleAndLilaPalette()
{
    CRGB a = CRGB(189, 0, 255);
    CRGB b = CRGB(214, 0, 255);
    CRGB c = CRGB(176, 0, 255);
    CRGB d = CRGB(150, 0, 255);
    CRGB e = CRGB(163, 0, 255);

    return CRGBPalette16(
        a, a, b, b,
        c, b, c, d,
        d, e, d, c,
        c, d, d, e);
}
