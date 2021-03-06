#include <FastLED.h>

// LED
#define LED_PIN 3
#define LED_PIN2 5
#define BRIGHTNESS 255
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB // GRB for WS2812, BRG for WS2811
#define NUM_LEDS 138
#define NUM_LEDS2 13

// The leds
CRGB leds[NUM_LEDS];
CRGB leds2[NUM_LEDS2];

static double x;
static double z;

/////// SETTINGS
#define HOLD_PALETTES_X_TIMES_AS_LONG 8
#define UPDATES_PER_SECOND 30
double speedFactor = 0.25;
double hueSpeedFactor = 1.0 / 6;
double scaleFactor = 1;
double scaleFactor2 = 2;
uint8_t lowResStripeStart = 87;
////////
double speed = 1 * speedFactor; // speed is set dynamically once we've started up
double newspeed = speed;
double scale = 16 * scaleFactor; // scale is set dynamically once we've started up
double newscale = scale;
uint8_t colorLoop = 1;
static double ihue = 0;
// lerp variables
uint8_t maxChanges = 6;
uint8_t lerpDuration = 20;
double lerpSpeed = 1 / (UPDATES_PER_SECOND * lerpDuration);
double scaleDiff = 0;
double speedDiff = 0;
double speedStepSize = 0;
double scaleStepSize = 0;
double lerpStepCurrent = 0;

// for blending in palettes smoothly
CRGBPalette16 targetPalette(RainbowColors_p);
CRGBPalette16 currentPalette(RainbowColors_p);
boolean isSwitchingPalette = true;

void setup()
{
    //  Serial.begin(9600);
    delay(150);
    LEDS.setMaxPowerInVoltsAndMilliamps(5, 1000);
    LEDS.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    LEDS.addLeds<LED_TYPE, LED_PIN2, COLOR_ORDER>(leds2, NUM_LEDS2);
    LEDS.setBrightness(BRIGHTNESS);

    // Initialize our coordinates to some random values
    x = random16();
    z = random16();
}

void mapCoordToColor()
{
    uint8_t dataSmoothing = 0;
    if (speed < 4)
    {
        dataSmoothing = 220 - (speed * 4);
    }

    for (int i = 0; i < NUM_LEDS; i++)
    {
        // first value is the radius
        double fractionOffset = 0.125 * (i % 8);
        double xoffset;
        if (NUM_LEDS < lowResStripeStart)
        {
            xoffset = i * scale + fractionOffset;
        }
        else
        {
            xoffset = lowResStripeStart * scale + (i - lowResStripeStart) * scale * scaleFactor2 + fractionOffset;
        }

        uint8_t index = inoise8(x + xoffset, z);
        uint8_t bri = inoise8(x, z + xoffset); // another random point for brightness

        if (dataSmoothing)
        {
            uint8_t oldindex = inoise8(x + xoffset - speed, z - speed);
            uint8_t oldbri = inoise8(x - speed, z - speed + xoffset);
            index = scale8(oldindex, dataSmoothing) + scale8(index, 256 - dataSmoothing);
            bri = scale8(oldbri, dataSmoothing) + scale8(index, 256 - dataSmoothing);
        }

        // if this palette is a 'loop', add a slowly-changing base value
        // if (colorLoop)
        // {
        //     index += ihue;
        // }
        // index += ihue;

        // brighten up, as the color palette itself often contains the
        // light/dark dynamic range desired
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
}

void mapCoordToColor2()
{
    uint8_t dataSmoothing = 0;
    if (speed < 4)
    {
        dataSmoothing = 220 - (speed * 4);
    }

    for (int i = 0; i < NUM_LEDS2; i++)
    {
        // first value is the radius

        double fractionOffset = 0.125 * (i % 8);
        int xoffset = (i + NUM_LEDS) * scale * scaleFactor2 + fractionOffset;

        uint8_t index = inoise8(x + xoffset, z);
        uint8_t bri = inoise8(x, z + xoffset); // another random point for brightness

        if (dataSmoothing)
        {
            uint8_t oldindex = inoise8(x + xoffset - speed, z - speed);
            uint8_t oldbri = inoise8(x - speed, z - speed + xoffset);
            index = scale8(oldindex, dataSmoothing) + scale8(index, 256 - dataSmoothing);
            bri = scale8(oldbri, dataSmoothing) + scale8(index, 256 - dataSmoothing);
        }

        // if this palette is a 'loop', add a slowly-changing base value
        // if (colorLoop)
        // {
        //     index += ihue;
        // }
        // index += ihue;

        // brighten up, as the color palette itself often contains the
        // light/dark dynamic range desired
        if (bri > 127)
        {
            bri = 255;
        }
        else
        {
            bri = dim8_raw(bri * 2);
        }

        CRGB color = ColorFromPalette(currentPalette, index, bri);
        leds2[i] = color;
    }
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
    mapCoordToColor2();

    z += speed;
    x += speed;
    ihue += hueSpeedFactor;

    LEDS.show();
    LEDS.delay(1000 / UPDATES_PER_SECOND);
}

void setPalette(CRGBPalette16 _targetPallete, double _newspeed, double _newscale, uint8_t _colorLoop)
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
            setPalette(LavaColors_p, 4, 4, 0);
        }
        if (secondHand == 8)
        {
            setPalette(getPurpleAndGreenPalette(), 4, 20, 1);
        }
        if (secondHand == 15)
        {
            setPalette(getRandomPalette(), 6, 2, 1);
        }
        if (secondHand == 25)
        {
            setPalette(RainbowColors_p, 6, 4, 1);
        }
        if (secondHand == 35)
        {
            setPalette(getRandomPalette(), 4, 8, 1);
        }
        if (secondHand == 40)
        {
            setPalette(OceanColors_p, 8, 16, 1);
        }
        if (secondHand == 45)
        {
            setPalette(PartyColors_p, 6, 4, 1);
        }
        if (secondHand == 50)
        {

            setPalette(getRandomPalette(), 4, 2, 1);
        }
        if (secondHand == 55)
        {
            setPalette(RainbowStripeColors_p, 4, 4, 1);
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
