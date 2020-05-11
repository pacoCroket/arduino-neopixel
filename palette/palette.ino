#include <FastLED.h>

// LED
#define LED_PIN 3
#define LED_PIN2 5
#define BRIGHTNESS 115
#define LED_TYPE WS2812
#define COLOR_ORDER GRB // GRB for WS2812, BRG for WS2811
#define NUM_LEDS 88
#define NUM_LEDS2 13

// The leds
CRGB leds[NUM_LEDS];
CRGB leds2[NUM_LEDS2];

static double x;
static double y;
static double z;

/////// SETTINGS
#define HOLD_PALETTES_X_TIMES_AS_LONG 8
#define UPDATES_PER_SECOND 20
double speedFactor = 0.5;
double hueSpeedFactor = 0.25;
double scaleFactor = 0.5;
double scaleFactor2 = 4;
uint8_t maxChanges = 3;
double lerpSpeed = 0.0001;
////////
double lerpAmount = 0;
double speed = 4 * speedFactor; // speed is set dynamically once we've started up
double newspeed = speed;
double scale = 4 * scaleFactor; // scale is set dynamically once we've started up
double newscale = scale;
uint8_t colorLoop = 1;
static double ihue = 0;

// for blending in palettes smoothly
CRGBPalette16 targetPalette(PartyColors_p);
CRGBPalette16 currentPalette(PartyColors_p);

void setup()
{
    //  Serial.begin(9600);
    delay(300);
    LEDS.setMaxPowerInVoltsAndMilliamps(5, 1000);
    LEDS.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    LEDS.addLeds<LED_TYPE, LED_PIN2, COLOR_ORDER>(leds2, NUM_LEDS2);
    LEDS.setBrightness(BRIGHTNESS);

    // Initialize our coordinates to some random values
    x = random8();
    z = random8();
    y = random8();
}

void mapCoordToColor()
{
    uint8_t dataSmoothing = 0;
    if (speed < 50)
    {
        dataSmoothing = 220 - (speed * 4);
    }

    for (int i = 0; i < NUM_LEDS; i++)
    {
        // first value is the radius

        int xoffset = i * scale;
        double z_frac = (1.0 / 30.0) * (i % 30);
        // double z_frac = 0;
        uint8_t index = inoise8(x + xoffset + z_frac, z + z_frac);
        uint8_t bri = inoise8(x - z_frac, z + xoffset - z_frac); // another random point for brightness

        if (dataSmoothing)
        {
            uint8_t oldindex = inoise8(x + xoffset - speed / 2 + z_frac, z - speed + z_frac);
            uint8_t oldbri = inoise8(x - speed / 2 - z_frac, z - speed + xoffset - z_frac);
            index = scale8(oldindex, dataSmoothing) + scale8(index, 256 - dataSmoothing);
            bri = scale8(oldbri, dataSmoothing) + scale8(index, 256 - dataSmoothing);
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
            bri = dim8_video(bri * 2);
        }

        CRGB color = ColorFromPalette(currentPalette, index, bri);
        leds[i] = color;
    }
}

void mapCoordToColor2()
{
    uint8_t dataSmoothing = 0;
    if (speed < 50)
    {
        dataSmoothing = 220 - (speed * 4);
    }

    for (int i = 0; i < NUM_LEDS2; i++)
    {
        // first value is the radius

        int xoffset = i * scale * scaleFactor2;
        double z_frac = (1.0 / 10.0) * (i % 10);
        uint8_t index = inoise8(y + xoffset + z_frac, z + z_frac);
        // uint8_t bri = inoise8(y, z + xoffset); // another random point for brightness

        if (dataSmoothing)
        {
            uint8_t oldindex = inoise8(y + xoffset - speed / 2 + z_frac, z - speed + z_frac);
            // uint8_t oldbri = inoise8(y - speed / 2, z - speed + xoffset);
            index = scale8(oldindex, dataSmoothing) + scale8(index, 256 - dataSmoothing);
            // bri = scale8(oldbri, dataSmoothing) + scale8(index, 256 - dataSmoothing);
        }

        // if this palette is a 'loop', add a slowly-changing base value
        if (colorLoop)
        {
            index += ihue;
        }

        // brighten up, as the color palette itself often contains the
        // light/dark dynamic range desired
        // if (bri > 127)
        // {
        //     bri = 255;
        // }
        // else
        // {
        //     bri = dim8_raw(bri * 2);
        // }

        CRGB color = ColorFromPalette(currentPalette, index, 255);
        leds2[i] = color;
    }
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

    if (lerpAmount <= 1)
    {
        speed = (1 - lerpAmount) * speed + newspeed * lerpAmount;
        scale = (1 - lerpAmount) * scale + newscale * lerpAmount;
        lerpAmount += lerpSpeed;
    }

    mapCoordToColor();
    mapCoordToColor2();

    z += speed;
    x += speed / 2.0;
    y += speed / 2.0;
    ihue += hueSpeedFactor;

    LEDS.show();
    LEDS.delay(1000 / UPDATES_PER_SECOND);
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
