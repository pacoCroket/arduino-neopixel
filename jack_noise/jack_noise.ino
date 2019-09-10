#include <FastLED.h>

#define LED_PIN     3
#define BRIGHTNESS  70
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS 294
#define NUM_SKIP_LEDS 5

const uint8_t ledsArray[NUM_LEDS][3] = {{168,240}, {162,240}, {157,240}, {151,240}, {145,240}, {139,240}, {134,240}, {127,239}, {121,239}, {115,238}, {109,238}, {102,237}, {96,236}, {90,236}, {83,235}, {77,235}, {69,233}, {64,232}, {58,231}, {51,230}, {45,229}, {38,238}, {37,232}, {35,227}, {34,221}, {33,215}, {32,207}, {31,201}, {30,195}, {29,189}, {27,182}, {25,176}, {24,169}, {22,164}, {20,158}, {13,148}, {18,148}, {24,148}, {30,148}, {35,148}, {41,148}, {54,149}, {61,149}, {67,149}, {73,149}, {81,150}, {86,150}, {93,149}, {99,149}, {107,149}, {113,149}, {119,149}, {133,151}, {139,151}, {145,152}, {151,152}, {157,152}, {163,153}, {169,153}, {168,164}, {165,169}, {161,173}, {158,178}, {154,182}, {151,188}, {147,193}, {145,199}, {137,209}, {134,215}, {131,221}, {128,226}, {125,231}, {90,228}, {92,223}, {95,215}, {98,211}, {101,203}, {103,197}, {106,191}, {109,186}, {112,178}, {115,173}, {117,168}, {120,163}, {123,155}, {128,145}, {131,138}, {133,133}, {135,127}, {137,121}, {139,116}, {141,110}, {142,105}, {145,99}, {148,94}, {151,87}, {154,82}, {157,74}, {160,69}, {164,64}, {171,58}, {172,64}, {172,71}, {172,77}, {171,84}, {171,90}, {170,97}, {171,103}, {172,109}, {173,115}, {173,122}, {174,128}, {175,134}, {176,141}, {177,147}, {177,153}, {176,165}, {176,171}, {176,175}, {176,181}, {177,187}, {177,193}, {177,198}, {177,204}, {177,212}, {178,218}, {178,223}, {171,212}, {167,216}, {163,220}, {158,225}, {154,230}, {151,234}, {147,228}, {145,223}, {144,217}, {143,211}, {141,205}, {140,199}, {138,192}, {136,186}, {135,181}, {132,175}, {131,169}, {130,163}, {128,158}, {126,151}, {125,145}, {123,137}, {120,132}, {118,127}, {116,121}, {114,115}, {112,109}, {111,104}, {108,96}, {107,90}, {105,83}, {103,78}, {101,71}, {100,65}, {97,59}, {60,59}, {59,64}, {59,71}, {59,77}, {57,87}, {57,93}, {57,98}, {55,104}, {54,110}, {53,115}, {52,121}, {51,127}, {49,135}, {48,141}, {47,148}, {47,154}, {45,161}, {45,167}, {45,174}, {44,179}, {43,186}, {43,192}, {42,199}, {41,205}, {40,211}, {40,217}, {44,220}, {49,217}, {53,211}, {58,208}, {63,202}, {68,199}, {73,194}, {77,190}, {83,185}, {88,182}, {92,178}, {98,173}, {103,169}, {108,164}, {112,160}, {116,157}, {170,143}, {168,137}, {164,131}, {161,126}, {157,120}, {154,115}, {150,109}, {147,104}, {142,99}, {141,93}, {138,86}, {136,80}, {134,74}, {129,69}, {126,64}, {124,60}, {122,52}, {120,47}, {98,45}, {95,50}, {90,57}, {86,61}, {82,68}, {78,73}, {75,77}, {70,84}, {67,89}, {64,93}, {61,98}, {51,106}, {48,111}, {43,117}, {39,121}, {33,127}, {29,131}, {24,136}, {21,140}, {46,101}, {44,95}, {43,90}, {40,84}, {38,79}, {36,71}, {34,65}, {31,58}, {47,53}, {52,52}, {59,50}, {65,49}, {71,47}, {77,46}, {84,44}, {90,43}, {97,41}, {102,39}, {118,37}, {123,40}, {129,42}, {135,44}, {142,47}, {147,49}, {153,51}, {158,54}, {153,64}, {149,68}, {143,73}, {138,77}, {132,80}, {128,84}, {124,88}, {119,92}, {113,96}, {107,101}, {102,104}, {97,107}, {91,111}, {87,115}, {83,119}, {78,122}, {70,126}, {66,130}, {59,135}, {57,156}, {61,161}, {66,167}, {69,171}, {73,176}, {77,180}, {80,186}, {84,190}};
const uint16_t skipLeds[NUM_SKIP_LEDS] = {52, 68, 87, 119, 237};

struct Led {
  uint16_t id;
  uint8_t x;
  uint8_t y;
};


// This example combines two features of FastLED to produce a remarkable range of
// effects from a relatively small amount of code.  This example combines FastLED's 
// color palette lookup functions with FastLED's Perlin/simplex noise generator, and
// the combination is extremely powerful.
//
// You might want to look at the "ColorPalette" and "Noise" examples separately
// if this example code seems daunting.
//
// 
// The basic setup here is that for each frame, we generate a new array of 
// 'noise' data, and then map it onto the LED matrix through a color palette.
//
// Periodically, the color palette is changed, and new noise-generation parameters
// are chosen at the same time.  In this example, specific noise-generation
// values have been selected to match the given color palettes; some are faster, 
// or slower, or larger, or smaller than others, but there's no reason these 
// parameters can't be freely mixed-and-matched.
//
// In addition, this example includes some fast automatic 'data smoothing' at 
// lower noise speeds to help produce smoother animations in those cases.
//
// The FastLED built-in color palettes (Forest, Clouds, Lava, Ocean, Party) are
// used, as well as some 'hand-defined' ones, and some proceedurally generated
// palettes.


// The leds
CRGB leds[NUM_LEDS];

// The 8 bit version of our coordinates
static uint16_t x;
static uint16_t y;
static uint16_t z;

// We're using the x/y dimensions to map to the x/y pixels on the matrix.  We'll
// use the z-axis for "time".  speed determines how fast time moves forward.  Try
// 1 for a very slow moving effect, or 60 for something that ends up looking like
// water.
uint8_t speed = 20; // speed is set dynamically once we've started up

// Scale determines how far apart the pixels in our noise matrix are.  Try
// changing these values around to see how it affects the motion of the display.  The
// higher the value of scale, the more "zoomed out" the noise iwll be.  A value
// of 1 will be so zoomed in, you'll mostly see solid colors.

uint8_t scale = 15; // scale is set dynamically once we've started up

CRGBPalette16 currentPalette( PartyColors_p );
uint8_t       colorLoop = 1;

void setup() {
  delay(1000);
  LEDS.addLeds<LED_TYPE,LED_PIN,COLOR_ORDER>(leds,NUM_LEDS);
  LEDS.setBrightness(BRIGHTNESS);

  // Initialize our coordinates to some random values
  x = random16();
  y = random16();
  z = random16();
}

void mapCoordToColor() {
  // If we're runing at a low "speed", some 8-bit artifacts become visible
  // from frame-to-frame.  In order to reduce this, we can do some fast data-smoothing.
  // The amount of data smoothing we're doing depends on "speed".

  static uint8_t ihue=0;
    
  uint8_t dataSmoothing = 0;
  if( speed < 50) {
    dataSmoothing = 200 - (speed * 4);
  }

  for (uint16_t i = 0; i < NUM_LEDS; i++) {
//    uint16_t id = ledsArray[i].id;
//    uint8_t xoffset = scale * ledsArray[i].x;    
//    uint8_t yoffset = scale * ledsArray[i].y;

    uint16_t id = i;
    if(isvalueinarray(id, skipLeds, NUM_SKIP_LEDS)) {
      id++;
    }
    
    uint8_t xoffset = scale * ledsArray[i][0];    
    uint8_t yoffset = scale * ledsArray[i][1];
      
    uint8_t hue = inoise8(x + xoffset, y + yoffset,z);
    uint8_t bri = inoise8(y + yoffset, x + xoffset,z);

    // The range of the inoise8 function is roughly 16-238.
    // These two operations expand those values out to roughly 0..255
    // You can comment them out if you want the raw noise data.
//    hue = qsub8(hue,16);
//    hue = qadd8(hue,scale8(hue,39));

    if( dataSmoothing ) {
      uint8_t olddata = inoise8(x + xoffset - speed / 8,y + yoffset + speed / 16,z-speed);
      uint8_t newdata = scale8( olddata, dataSmoothing) + scale8( hue, 256 - dataSmoothing);
      hue = newdata;
    }

    // if this palette is a 'loop', add a slowly-changing base value
      if( colorLoop) { 
        hue += ihue;
      }

      // brighten up, as the color palette itself often contains the 
      // light/dark dynamic range desired
      if( bri > 127 ) {
        bri = 255;
      } else {
        bri = dim8_raw( bri * 2);
      }

      CRGB color = ColorFromPalette( currentPalette, hue, bri);
      leds[id] = color;
  }
  
  z += speed;
  
  // apply slow drift to X and Y, just for visual variation.
  x += speed / 8;
  y -= speed / 16;

  ihue+=1;
}

void loop() {
  // Periodically choose a new palette, speed, and scale
  ChangePaletteAndSettingsPeriodically();

  mapCoordToColor();

  LEDS.show();
//   delay(20);
}



// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.

// 1 = 5 sec per palette
// 2 = 10 sec per palette
// etc
#define HOLD_PALETTES_X_TIMES_AS_LONG 5

void ChangePaletteAndSettingsPeriodically()
{
  uint8_t secondHand = ((millis() / 1000) / HOLD_PALETTES_X_TIMES_AS_LONG) % 60;
  static uint8_t lastSecond = 99;
  
  if( lastSecond != secondHand) {
    lastSecond = secondHand;
    if( secondHand ==  0)  { currentPalette = RainbowColors_p;         speed = 10; scale = 5; colorLoop = 1; }
    if( secondHand ==  5)  { SetupPurpleAndGreenPalette();             speed = 5; scale = 5; colorLoop = 1; }
    if( secondHand == 10)  { SetupBlackAndWhiteStripedPalette();       speed = 8; scale = 3; colorLoop = 1; }
    if( secondHand == 15)  { currentPalette = ForestColors_p;          speed =  3; scale = 10; colorLoop = 0; }
    if( secondHand == 20)  { currentPalette = CloudColors_p;           speed =  4; scale = 5; colorLoop = 0; }
    if( secondHand == 25)  { currentPalette = LavaColors_p;            speed =  8; scale = 5; colorLoop = 0; }
    if( secondHand == 30)  { currentPalette = OceanColors_p;           speed = 10; scale = 12; colorLoop = 0; }
    if( secondHand == 35)  { currentPalette = PartyColors_p;           speed = 10; scale = 4; colorLoop = 1; }
    if( secondHand == 40)  { SetupRandomPalette();                     speed = 10; scale = 4; colorLoop = 1; }
    if( secondHand == 45)  { SetupRandomPalette();                     speed = 25; scale = 6; colorLoop = 1; }
    if( secondHand == 50)  { SetupRandomPalette();                     speed = 50; scale = 16; colorLoop = 1; }
    if( secondHand == 55)  { currentPalette = RainbowStripeColors_p;   speed = 15; scale = 4; colorLoop = 1; }
  }
}

// This function generates a random palette that's a gradient
// between four different colors.  The first is a dim hue, the second is 
// a bright hue, the third is a bright pastel, and the last is 
// another bright hue.  This gives some visual bright/dark variation
// which is more interesting than just a gradient of different hues.
void SetupRandomPalette()
{
  currentPalette = CRGBPalette16( 
                      CHSV( random8(), 255, 32), 
                      CHSV( random8(), 255, 255), 
                      CHSV( random8(), 128, 255), 
                      CHSV( random8(), 255, 255)); 
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
  // 'black out' all 16 palette entries...
  fill_solid( currentPalette, 16, CRGB::Black);
  // and set every fourth one to white.
  currentPalette[0] = CRGB::White;
  currentPalette[4] = CRGB::White;
  currentPalette[8] = CRGB::White;
  currentPalette[12] = CRGB::White;

}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
  CRGB purple = CHSV( HUE_PURPLE, 255, 255);
  CRGB green  = CHSV( HUE_GREEN, 255, 255);
  CRGB black  = CRGB::Black;
  
  currentPalette = CRGBPalette16( 
    green,  green,  black,  black,
    purple, purple, black,  black,
    green,  green,  black,  black,
    purple, purple, black,  black );
}

bool isvalueinarray(int val, int *arr, int size){
    int i;
    for (i=0; i < size; i++) {
        if (arr[i] == val)
            return true;
    }
    return false;
}
