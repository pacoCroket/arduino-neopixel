#include <FastLED.h>

// LED
#define LED_PIN 3
#define BRIGHTNESS 200
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB // GRB for WS2812, BRG for WS2811
#define NUM_LEDS 88

// The leds
CRGB leds[NUM_LEDS];

static float x;
static float z;

#define HOLD_PALETTES_X_TIMES_AS_LONG 5 // 15
#define UPDATES_PER_SECOND 30
float hueSpeed = 0.5;
float speedFactor = 0.25;
float scaleFactor = 0.5;
//
float speed = 4 * speedFactor; // speed is set dynamically once we've started up
float newspeed = speed;
float scale = 4 * scaleFactor; // scale is set dynamically once we've started up
float newscale = scale;
uint8_t colorLoop = 0;

// for blending in palettes smoothly
uint8_t maxChanges = 4;
float lerpAmount = 0;
float lerpSpeed = 0.0001;
CRGBPalette16 targetPalette(LavaColors_p);
CRGBPalette16 currentPalette(LavaColors_p);

void setup()
{
    delay(500);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 1000);
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    // FastLED.setDither(0);
    // Initialize our coordinates to some random values
    x = random16();
    z = random16();
}

void mapCoordToColor()
{
    static float ihue = 0.5;

    uint8_t dataSmoothing = 0;
    if (speed < 5)
    {
        dataSmoothing = 200 - (speed * 10);
    }

    for (int i = 0; i < NUM_LEDS; i++)
    {
        // first value is the radius

        uint8_t xoffset = i * scale;

        uint8_t index = inoise16(x + xoffset, z);
        uint8_t bri = inoise16(z - xoffset, x); // another random point for brightness

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
            uint8_t oldbri = inoise8(z - speed - xoffset, x - speed / 2);
            //
            index = scale8(oldindex, dataSmoothing) + scale8(index, 256 - dataSmoothing);
            bri = scale8(oldbri, dataSmoothing) + scale8(bri, 256 - dataSmoothing);
        }

        // if this palette is a 'loop', add a slowly-changing base value
        if (colorLoop)
        {
            index += ihue;
        }

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

    z += speed;
    x += speed / 2;

    ihue += hueSpeed;
}

void resetLerp()
{
    lerpAmount = 0;
}

void loop()
{

    // Periodically choose a new palette, speed, and scale
    ChangePaletteAndSettingsPeriodically();
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);
    // run the blend function only every Nth frames
    mapCoordToColor();
    if (lerpAmount <= 1)
    {
        speed = (1 - lerpAmount) * speed + newspeed * lerpAmount;
        scale = (1 - lerpAmount) * scale + newscale * lerpAmount;
        lerpAmount += lerpSpeed;
    }

    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
}

void ChangePaletteAndSettingsPeriodically()
{
    uint8_t secondHand = ((millis() / 1000) / HOLD_PALETTES_X_TIMES_AS_LONG) % 60;
    static uint8_t lastSecond = 99;

    if (lastSecond != secondHand)
    {
        lastSecond = secondHand;
        if (secondHand == 0)
        {
            targetPalette = LavaColors_p;
            newspeed = 4 * speedFactor;
            newscale = 4 * scaleFactor;
            resetLerp();
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
            resetLerp();
            // colorLoop = 1;
        }
        if (secondHand == 15)
        {
            SetupRandomPalette();
            newspeed = 6 * speedFactor;
            newscale = 2 * scaleFactor;
            colorLoop = 1;
            resetLerp();
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
            newspeed = 6 * speedFactor;
            newscale = 4 * scaleFactor;
            colorLoop = 1;
            resetLerp();
        }
        if (secondHand == 35)
        {
            SetupRandomPalette();
            newspeed = 4 * speedFactor;
            newscale = 8 * scaleFactor;
            colorLoop = 1;
            resetLerp();
        }
        if (secondHand == 40)
        {
            targetPalette = OceanColors_p;
            newspeed = 8 * speedFactor;
            newscale = 16 * scaleFactor;
            resetLerp();
            // colorLoop = 1;
        }
        if (secondHand == 45)
        {
            targetPalette = PartyColors_p;
            newspeed = 6 * speedFactor;
            newscale = 4 * scaleFactor;
            colorLoop = 1;
            resetLerp();
        }
        if (secondHand == 50)
        {
            SetupRandomPalette();
            newspeed = 4 * speedFactor;
            newscale = 2 * scaleFactor;
            colorLoop = 1;
            resetLerp();
        }
        if (secondHand == 55)
        {
            targetPalette = RainbowStripeColors_p;
            newspeed = 4 * speedFactor;
            newscale = 4 * scaleFactor;
            colorLoop = 1;
            resetLerp();
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
