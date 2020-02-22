#include <FastLED.h>
#include <Bounce2.h>

// LED
#define LED_PIN1 3
// #define LED_PIN2 5
#define BRIGHTNESS 180
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS1 135
#define NUM_LEDS2 5
#define BUTTON_PIN A0

// the box is 29 x 24 x 38 cm
// store leds1 in x, y, z, cordinates
const PROGMEM uint8_t ledsArray[NUM_LEDS1][2] = {{220, 68}, {228, 73}, {235, 77}, {243, 81}, {251, 85}, {253, 93}, {247, 98}, {239, 105}, {234, 110}, {227, 116}, {220, 122}, {213, 128}, {207, 134}, {199, 133}, {194, 127}, {190, 119}, {184, 111}, {180, 104}, {175, 96}, {170, 89}, {165, 81}, {160, 74}, {155, 67}, {150, 59}, {146, 52}, {141, 44}, {136, 36}, {131, 29}, {126, 22}, {122, 14}, {126, 12}, {134, 16}, {141, 20}, {148, 25}, {157, 29}, {163, 33}, {172, 38}, {179, 43}, {187, 48}, {194, 52}, {202, 57}, {218, 62}, {226, 57}, {233, 52}, {241, 49}, {249, 43}, {255, 38}, {247, 34}, {241, 27}, {234, 22}, {226, 17}, {220, 11}, {213, 5}, {207, 0}, {199, 2}, {194, 9}, {188, 16}, {183, 23}, {178, 30}, {173, 37}, {167, 45}, {162, 52}, {156, 59}, {151, 66}, {145, 73}, {140, 79}, {134, 87}, {128, 94}, {123, 100}, {117, 107}, {112, 114}, {124, 109}, {131, 105}, {139, 101}, {147, 96}, {155, 93}, {162, 88}, {170, 83}, {178, 80}, {185, 75}, {193, 70}, {201, 66}, {209, 63}, {141, 63}, {133, 64}, {125, 68}, {116, 71}, {108, 74}, {98, 77}, {90, 80}, {82, 83}, {73, 86}, {65, 89}, {56, 92}, {47, 90}, {39, 85}, {31, 80}, {24, 74}, {16, 70}, {8, 64}, {0, 60}, {8, 54}, {15, 50}, {24, 45}, {32, 41}, {41, 36}, {48, 31}, {58, 30}, {67, 33}, {75, 37}, {84, 40}, {92, 44}, {101, 48}, {109, 50}, {117, 53}, {125, 57}, {134, 60}, {105, 108}, {100, 100}, {95, 93}, {90, 86}, {85, 78}, {81, 71}, {80, 53}, {85, 46}, {91, 38}, {97, 32}, {102, 25}, {108, 18}, {83, 62}, {74, 62}, {65, 62}, {39, 61}, {30, 61}, {20, 61}};

// The leds1
CRGB leds1[NUM_LEDS1];
// CRGB leds2[NUM_LEDS2];

static double x;
static double y;
static double z;

double speedFactor = 0.25;
double speed = 6 * speedFactor; // speed is set dynamically once we've started up
double newSpeed = speed;
double oldSpeed = speed;
double scaleFactor = 0.5;
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

boolean buttonState = LOW;
boolean prevButtonState = LOW;
uint8_t pressCount = 0;
unsigned long lastStatusSwitch = 999999;
uint8_t briScale = 255;
boolean isSwitchingPalette = true;

double pulseRadius = 0;
double pulseRate = 2.5;
uint8_t fadeAmt = 250;
uint8_t spikeLeds[5] = {0, 11, 22, 33, 44};
boolean isPulsePattern = false;
// Instantiate a Bounce object
Bounce debouncer = Bounce();

void setup()
{
    // Serial.begin(9600);
    // Serial.println("Start!");

    LEDS.addLeds<LED_TYPE, LED_PIN1, COLOR_ORDER>(leds1, NUM_LEDS1);
    // LEDS.addLeds<LED_TYPE, LED_PIN2, COLOR_ORDER>(leds2, NUM_LEDS2);
    LEDS.setBrightness(BRIGHTNESS);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);

    // Initialize our coordinates to some random values
    x = random8();
    y = random8();
    z = random8();

    // Setup the button with an internal pull-up :
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // After setting up the button, setup the Bounce instance :
    debouncer.attach(BUTTON_PIN);
    debouncer.interval(10); // interval in ms
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
            uint8_t oldIndex = inoise8(x + xoffset + speed, y + yoffset, z - speed);
            uint8_t oldBri = inoise8(x + yoffset + speed, y + xoffset, z - speed);
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
        // if (i < NUM_LEDS2)
        //     leds2[i] = color;
    }

    z += speed;
    x -= speed;

    ihue += 1;
}

void loop()
{
    // handleButton();

    // blend function to smoothly transition between patterns
    blend();

    // switch between perlin noise patterns and pulse patterns every few mins
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
            SetupFutureBluePalette();
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
    static uint16_t range = 30;

    for (uint8_t i = 0; i < NUM_LEDS1; i++)
    {

        // finding x and y coordinates to pick the color and to vector calculation
        uint8_t x2 = pgm_read_byte(&(ledsArray[i][0]));
        uint8_t y2 = pgm_read_byte(&(ledsArray[i][1]));

        uint8_t index = inoise8(x + x2 * scale, y + y2 * scale, z);
        uint8_t bri = inoise8(x + y2 * scale, y + x2 * scale, z);

        // find distance to center (127, 127)
        uint16_t dist = 255 - x2;
        // LEDs within the star
        if (dist > pulseRadius && dist <= pulseRadius + range)
        {
            // reduce effect of brightness so it's not very dark
            bri = map(bri, 0, 255, 110, 255);

            if (pulseRate > 0)
                bri = map(dist - pulseRadius, 0, range, bri, 0);
            else
                bri = map(dist - pulseRadius, 0, range, 0, bri);

            CRGB color = ColorFromPalette(currentPalette, index, bri);
            leds1[i] = color;
        }
        else
        {
            leds1[i].nscale8(map((triwave8((seconds16()) % 255) + 64) % 255, 0, 255, 235, 254));
            // leds1[i].nscale8(fadeAmt);
        }

        // replicate first back LEDs into front LEDs
        // if (i < NUM_LEDS2)
        //     leds2[i] = leds1[i];
    }

    // add some movement for the noise
    z += speed / 2;

    if (pulseRadius > 300)
        pulseRate = -1 * pulseRate;
    else if (pulseRadius < -45)
        pulseRate = -1 * pulseRate;

    pulseRadius += pulseRate;
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
        Serial.println("Button continuosly pressed");
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