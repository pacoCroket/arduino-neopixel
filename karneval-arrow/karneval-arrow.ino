#include <FastLED.h>

// LED
#define LED_PIN 3
#define BRIGHTNESS 255
#define LED_TYPE WS2811
#define COLOR_ORDER BRG // GRB for WS2812, BRG for WS2811
#define NUM_LEDS 10

// The leds
CRGB leds[NUM_LEDS];
uint8_t currentLed = 0;
uint8_t outFrameSpace = 4; // virtual LEDs to add time between waves of color from the sides
uint8_t delayTime = 100;
long lastDelay = 0;

void setup()
{
    LEDS.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    LEDS.setBrightness(BRIGHTNESS);
}

void loop()
{
    if (millis() - lastDelay >= delayTime)
    {
        waveColor();
        lastDelay = millis();
    }
    fadeAll();
    LEDS.show();
}

void waveColor()
{
    leds[currentLed] = CRGB :: Coral;
    currentLed = (currentLed + 1) % NUM_LEDS;
}

void fadeAll()
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i].nscale8(map((triwave8((seconds16()) % 255) + 64) % 255, 0, 255, 251, 254));
        // leds[i].nscale8(254);
    }
}