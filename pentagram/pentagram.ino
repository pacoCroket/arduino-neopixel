#include <FastLED.h>

// LED
#define LED_PIN1 3
#define LED_PIN2 5
#define BRIGHTNESS 180
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS1 75
#define NUM_LEDS2 5
#define BUTTON_PIN 2

// the box is 29 x 24 x 38 cm
// store leds1 in x, y, z, cordinates
const PROGMEM uint8_t ledsArray[NUM_LEDS1][2] =
    {{59, 26}, {72, 37}, {87, 50}, {100, 61}, {115, 72}, {142, 95}, {157, 106}, {173, 118}, {187, 128}, {202, 140}, {217, 150}, {244, 171}, {226, 171}, {208, 171}, {189, 170}, {171, 170}, {134, 169}, {114, 169}, {95, 168}, {78, 168}, {59, 167}, {41, 167}, {5, 166}, {20, 155}, {36, 145}, {50, 135}, {66, 125}, {96, 103}, {112, 93}, {127, 82}, {143, 72}, {156, 62}, {172, 50}, {202, 30}, {196, 48}, {189, 66}, {183, 86}, {177, 102}, {166, 137}, {160, 154}, {152, 171}, {147, 188}, {140, 205}, {134, 222}, {121, 255}, {116, 237}, {110, 220}, {106, 203}, {101, 186}, {90, 150}, {85, 133}, {81, 115}, {76, 97}, {71, 80}, {66, 61}, {78, 13}, {113, 0}, {148, 0}, {181, 17}, {219, 44}, {239, 75}, {247, 105}, {250, 143}, {240, 193}, {216, 221}, {184, 239}, {149, 251}, {94, 251}, {62, 236}, {37, 216}, {13, 187}, {0, 138}, {5, 101}, {16, 73}, {36, 42}};
// The leds1
CRGB leds1[NUM_LEDS1];
CRGB leds2[NUM_LEDS2];

static double x;
static double y;
static double z;

double speedFactor = 0.3;
double speed = 6 * speedFactor; // speed is set dynamically once we've started up
double newSpeed = speed;
double oldSpeed = speed;
double scaleFactor = 0.3;
double scale = 6 * scaleFactor; // scale is set dynamically once we've started up
double newScale = scale;
double oldScale = scale;
uint8_t colorLoop = 1;

// for blending in palettes smoothly
uint8_t maxChanges = 5;
double lerpAmount = 2;
uint8_t easeInVal = 255;
CRGBPalette16 targetPalette(LavaColors_p);
CRGBPalette16 currentPalette(LavaColors_p);

boolean buttonState = HIGH;
boolean prevButtonState = HIGH;
uint8_t pressCount = 0;
unsigned long lastStatusSwitch = 999999;
uint8_t briScale = 255;
boolean isSwitchingPalette = true;
boolean isPulsePattern = false;

double pulseRadius = 0;
double pulseRate = 1.7;

void setup()
{
    //  Serial.begin(9600);

    LEDS.addLeds<LED_TYPE, LED_PIN1, COLOR_ORDER>(leds1, NUM_LEDS1);
    LEDS.addLeds<LED_TYPE, LED_PIN2, COLOR_ORDER>(leds2, NUM_LEDS2);
    LEDS.setBrightness(BRIGHTNESS);

    // Initialize our coordinates to some random values
    x = random8();
    y = random8();
    z = random8();
}

void mapCoordToColor()
{
    static uint8_t ihue = random8() / 4;

    uint8_t dataSmoothing = 0;
    // TODO what speed?
    if (speed < 3)
    {
        dataSmoothing = 200 - (speed * 5);
    }

    for (uint8_t i = 0; i < NUM_LEDS1; i++)
    {
        // first value is the radius

        uint16_t xoffset = pgm_read_byte(&(ledsArray[i][0])) * scale;
        uint16_t yoffset = pgm_read_byte(&(ledsArray[i][1])) * scale;

        uint8_t index = inoise8(x + xoffset, y + yoffset, z);
        uint8_t bri = inoise8(x + yoffset, y + xoffset, z); // another random point for brightness

        if (dataSmoothing)
        {
            uint8_t oldIndex = inoise8(x + xoffset, y + yoffset - speed, z - speed);
            uint8_t oldBri = inoise8(x + yoffset, y + xoffset - speed, z - speed);
            index = scale8(oldIndex, dataSmoothing) + scale8(index, 256 - dataSmoothing);
            bri = scale8(oldBri, dataSmoothing) + scale8(bri, 256 - dataSmoothing);
        }

        // if this palette is a 'loop', add a slowly-changing base value
        if (colorLoop)
        {
            index += ihue;
        }

        bri = dim8_raw(scale8(bri, briScale));

        CRGB color = ColorFromPalette(currentPalette, index, bri);
        leds1[i] = color;
        if (i < NUM_LEDS2)
            leds2[i] = color;
    }

    z += speed;
    y += speed;

    ihue += 1;
}

void loop()
{
    // handleButton();

    // Periodically choose a new palette, speed, and scale
    blend();

    if (minutes16() % 7 < 6)
    {
        ChangePaletteAndSettingsPeriodically();
        mapCoordToColor();
        if (isPulsePattern)
            isPulsePattern = false;
    }
    else
    {
        if (!isPulsePattern)
        {
            SetupLavaPattern();
            isPulsePattern = true;
        }
        pulse();
    }

    LEDS.show();
    //   delay(20);
}

void blend()
{
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);

    if (easeInVal < 255)
    {
        uint8_t easeOutVal = ease8InOutQuad(easeInVal);
        if (easeOutVal == 0)
        {
            scale = oldScale;
            speed = oldSpeed;
        }
        else
        {
            speed = lerp8toDouble(easeOutVal, 0, oldSpeed, 255, newSpeed);
            scale = lerp8toDouble(easeOutVal, 0, oldScale, 255, newScale);
            // scale = oldScale * (1 - 255.0 / easeOutVal) + newScale * (255.0 / easeOutVal);
            // speed = oldSpeed * (1 - 255.0 / easeOutVal) + newSpeed * (255.0 / easeOutVal);
        }

        // speed = lerp8by8(oldSpeed, newSpeed, ease8InOutQuad(easeInVal));
        // scale = lerp8by8(oldScale, newScale, ease8InOutQuad(easeInVal));
        easeInVal++;
    }
}

void pulse()
{
    // radial lenth of LEDs to light up
    static uint16_t range = 22;

    for (uint8_t i = 0; i < NUM_LEDS1; i++)
    {

        // finding x and y coordinates to pick the color and to vector calculation
        uint8_t x2 = pgm_read_byte(&(ledsArray[i][0]));
        uint8_t y2 = pgm_read_byte(&(ledsArray[i][1]));

        uint8_t index = inoise8(x + x2 * scale, y + y2 * scale, z);
        uint8_t bri = 0;

        // find distance to center (127, 127)
        uint16_t dist = sqrt(pow(x2 - 127, 2) + pow(y2 - 127, 2));
        if (dist > pulseRadius && dist <= pulseRadius + range)
        {
            bri = map(dist, 0, 127, 256, 180);
            CRGB color = ColorFromPalette(currentPalette, index, bri);
            leds1[i] = color;
        }
        else
        {
            leds1[i].nscale8(243);
        }

        if (i < NUM_LEDS2)
            leds2[i] = leds1[i];
    }

    z += speed / 2;
    y += speed / 2;

    if (pulseRadius >= 165 - pulseRate)
        pulseRate = -1 * pulseRate;
    else if (pulseRadius <= -pulseRate)
        pulseRate = -1 * pulseRate;

    pulseRadius += pulseRate;
}

void handleButton()
{
    buttonState = digitalRead(BUTTON_PIN);

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

#define HOLD_PALETTES_X_TIMES_AS_LONG 8

void ChangePaletteAndSettingsPeriodically()
{
    uint8_t secondHand = (seconds16() / HOLD_PALETTES_X_TIMES_AS_LONG) % 60;
    static uint8_t lastSecond = 99;

    if (lastSecond != secondHand && isSwitchingPalette)
    {
        lastSecond = secondHand;
        if (secondHand == 0)
        {
            SetupLavaPattern();
        }
        if (secondHand == 6)
        {
            SetupBlackAndWhiteStripedPalette();
            newSpeed = 35 * speedFactor;
            newScale = 4 * scaleFactor;
            colorLoop = 1;
        }
        if (secondHand == 15)
        {
            SetupPurpleAndGreenPalette();
            newSpeed = 1 * speedFactor;
            newScale = 7 * scaleFactor;
            colorLoop = 1;
        }
        if (secondHand == 25)
        {
            SetupRandomPalette();
            newSpeed = 7 * speedFactor;
            newScale = 7 * scaleFactor;
            colorLoop = 1;
        }
        //    if( secondHand == 15)  { currentPalette = ForestColors_p;          speed =  3; scale = 8 * scaleFactor; colorLoop = 0; }
        // if( secondHand == 25)  { targetPalette = CloudColors_p;           newSpeed =  8 * speedFactor; newScale = 7 * scaleFactor; colorLoop = 0; }
        if (secondHand == 35)
        {
            targetPalette = RainbowColors_p;
            newSpeed = 12 * speedFactor;
            newScale = 5 * scaleFactor;
            colorLoop = 1;
            startBlend();
        }
        if (secondHand == 40)
        {
            SetupRandomPalette();
            newSpeed = 5 * speedFactor;
            newScale = 12 * scaleFactor;
            colorLoop = 1;
        }
        // if( secondHand == 40)  { targetPalette = OceanColors_p;           newSpeed = 15 * speedFactor; newScale = 25 * scaleFactor; colorLoop = 0; }
        if (secondHand == 45)
        {
            targetPalette = PartyColors_p;
            newSpeed = 11 * speedFactor;
            newScale = 4 * scaleFactor;
            colorLoop = 1;
            startBlend();
        }
        if (secondHand == 53)
        {
            SetupRandomPalette();
            newSpeed = 15 * speedFactor;
            newScale = 6 * scaleFactor;
            colorLoop = 1;
        }
        // if( secondHand == 55)  { targetPalette = RainbowStripeColors_p;   newSpeed = 9 * speedFactor; newScale = 4 * scaleFactor; colorLoop = 1; }
    }
}

void SetupRandomPalette()
{
    startBlend();
    targetPalette = CRGBPalette16(
        CHSV(random8(), 255, 32),
        CHSV(random8(), 255, 255),
        CHSV(random8(), 128, 255),
        CHSV(random8(), 255, 255));
}

void SetupBlackAndWhiteStripedPalette()
{
    startBlend();
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
    startBlend();
    CRGB purple = CHSV(HUE_PURPLE, 255, 255);
    CRGB green = CHSV(HUE_GREEN, 255, 255);
    CRGB black = CRGB::Black;

    targetPalette = CRGBPalette16(
        green, green, black, black,
        purple, purple, black, black,
        green, green, black, black,
        purple, purple, black, black);
}

void SetupFutureBluePalette()
{
    startBlend();
    newSpeed = 7 * speedFactor;
    newScale = 8 * scaleFactor;
    colorLoop = 0;

    CRGB blue0 = CRGB::DarkBlue;
    CRGB blue1 = CRGB::DodgerBlue;
    CRGB blue2 = CRGB::MidnightBlue;
    CRGB blue3 = CRGB::RoyalBlue;
    CRGB blue4 = CRGB::Navy;
    CRGB black = CRGB::Black;

    targetPalette = CRGBPalette16(
        blue0, black, blue0, blue0,
        blue1, blue1, blue2, blue2,
        blue3, blue3, blue4, blue4,
        blue2, blue0, blue3, blue1);
}

boolean isNumberInArray(uint8_t number, uint8_t array[])
{
    for (uint8_t i = 0; i < 5; i++)
    {
        if (array[i] == number)
            return true;
    }
    return false;
}

void SetupLavaPattern()
{
    targetPalette = LavaColors_p;
    newSpeed = 7 * speedFactor;
    newScale = 8 * scaleFactor;
    colorLoop = 0;
    startBlend();
}

void startBlend()
{
    easeInVal = 0;
    oldSpeed = speed;
    oldScale = scale;
}

double lerp8toDouble(uint8_t x, double x0, double y0, double x1, double y1)
{
    return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
}