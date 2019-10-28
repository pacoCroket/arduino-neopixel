#include <FastLED.h>

#define LED_PIN     3
#define BRIGHTNESS  70
#define LED_TYPE    WS2812B
#define COLOR_ORDER RGB
#define NUM_LEDS 294

const uint8_t ledsArray[NUM_LEDS] = {{0,59,84}, {1,57,84}, {2,55,84}, {3,53,84}, {4,51,84}, {5,49,84}, {6,47,84}, {7,44,84}, {8,42,84}, {9,40,83}, {10,38,83}, {11,36,83}, {12,34,83}, {13,32,83}, {14,29,82}, {15,27,82}, {16,24,82}, {17,22,81}, {18,20,81}, {19,18,80}, {20,16,80}, {21,13,83}, {22,13,81}, {23,12,79}, {24,12,77}, {25,12,75}, {26,11,72}, {27,11,70}, {28,10,68}, {29,10,66}, {30,9,64}, {31,9,62}, {32,8,59}, {33,8,57}, {34,7,55}, {35,4,52}, {36,6,52}, {37,8,52}, {38,10,52}, {39,12,52}, {40,14,52}, {41,19,52}, {42,21,52}, {43,23,52}, {44,26,52}, {45,28,52}, {46,30,52}, {47,33,52}, {48,35,52}, {49,37,52}, {50,39,52}, {51,42,52}, {53,47,53}, {54,49,53}, {55,51,53}, {56,53,53}, {57,55,53}, {58,57,53}, {59,59,54}, {60,59,57}, {61,58,59}, {62,56,61}, {63,55,62}, {64,54,64}, {65,53,66}, {66,52,68}, {67,51,70}, {69,48,73}, {70,47,75}, {71,46,78}, {72,45,79}, {73,44,81}, {74,32,80}, {75,32,78}, {76,33,75}, {77,34,74}, {78,35,71}, {79,36,69}, {80,37,67}, {81,38,65}, {82,39,62}, {83,40,61}, {84,41,59}, {85,42,57}, {86,43,54}, {88,45,51}, {89,46,48}, {90,47,46}, {91,47,44}, {92,48,42}, {93,49,41}, {94,49,39}, {95,50,37}, {96,51,35}, {97,52,33}, {98,53,31}, {99,54,29}, {100,55,26}, {101,56,24}, {102,57,22}, {103,60,20}, {104,60,22}, {105,60,25}, {106,60,27}, {107,60,30}, {108,60,32}, {109,60,34}, {110,60,36}, {111,60,38}, {112,60,40}, {113,61,43}, {114,61,45}, {115,61,47}, {116,62,49}, {117,62,51}, {118,62,53}, {120,62,58}, {121,62,60}, {122,62,61}, {123,62,63}, {124,62,65}, {125,62,67}, {126,62,69}, {127,62,71}, {128,62,74}, {129,62,76}, {130,62,78}, {131,60,74}, {132,58,76}, {133,57,77}, {134,55,79}, {135,54,80}, {136,53,82}, {137,51,80}, {138,51,78}, {139,51,76}, {140,50,74}, {141,49,72}, {142,49,70}, {143,48,67}, {144,48,65}, {145,47,63}, {146,46,61}, {147,46,59}, {148,45,57}, {149,45,55}, {150,44,53}, {151,44,51}, {152,43,48}, {153,42,46}, {154,41,44}, {155,41,42}, {156,40,40}, {157,39,38}, {158,39,36}, {159,38,34}, {160,37,32}, {161,37,29}, {162,36,27}, {163,35,25}, {164,35,23}, {165,34,20}, {166,21,21}, {167,21,23}, {168,21,25}, {169,21,27}, {170,20,31}, {171,20,32}, {172,20,34}, {173,19,36}, {174,19,38}, {175,19,40}, {176,18,42}, {177,18,45}, {178,17,47}, {179,17,50}, {180,17,52}, {181,16,54}, {182,16,56}, {183,16,58}, {184,16,61}, {185,15,63}, {186,15,65}, {187,15,67}, {188,15,70}, {189,14,72}, {190,14,74}, {191,14,76}, {192,15,77}, {193,17,76}, {194,19,74}, {195,20,73}, {196,22,71}, {197,24,69}, {198,26,68}, {199,27,67}, {200,29,65}, {201,31,64}, {202,32,62}, {203,34,60}, {204,36,59}, {205,38,57}, {206,39,56}, {207,41,55}, {208,60,50}, {209,59,48}, {210,57,46}, {211,56,44}, {212,55,42}, {213,54,40}, {214,52,38}, {215,52,36}, {216,50,35}, {217,49,33}, {218,48,30}, {219,48,28}, {220,47,26}, {221,45,24}, {222,44,22}, {223,43,21}, {224,43,18}, {225,42,16}, {226,34,16}, {227,33,17}, {228,32,20}, {229,30,22}, {230,29,24}, {231,27,25}, {232,26,27}, {233,25,29}, {234,24,31}, {235,22,33}, {236,21,34}, {238,18,37}, {239,17,39}, {240,15,41}, {241,14,42}, {242,11,44}, {243,10,46}, {244,8,48}, {245,7,49}, {246,16,35}, {247,15,33}, {248,15,31}, {249,14,29}, {250,13,28}, {251,12,25}, {252,12,23}, {253,11,20}, {254,16,18}, {255,18,18}, {256,21,17}, {257,23,17}, {258,25,17}, {259,27,16}, {260,29,15}, {261,31,15}, {262,34,14}, {263,36,14}, {264,41,13}, {265,43,14}, {266,45,15}, {267,47,15}, {268,50,16}, {269,52,17}, {270,53,18}, {271,55,19}, {272,54,22}, {273,52,24}, {274,50,26}, {275,48,27}, {276,46,28}, {277,45,29}, {278,43,31}, {279,42,32}, {280,39,34}, {281,37,35}, {282,36,36}, {283,34,38}, {284,32,39}, {285,30,40}, {286,29,42}, {287,27,43}, {288,24,44}, {289,23,45}, {290,21,47}, {291,20,55}, {292,21,56}, {293,23,58}, {294,24,60}, {295,26,62}, {296,27,63}, {297,28,65}, {298,29,67}};

typedef struct Led {
  uint16_t id;
  uint8_t x;
  uint8_t y;
} Led;


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
static uint8_t x;
static uint8_t y;
static uint8_t z;

// We're using the x/y dimensions to map to the x/y pixels on the matrix.  We'll
// use the z-axis for "time".  speed determines how fast time moves forward.  Try
// 1 for a very slow moving effect, or 60 for something that ends up looking like
// water.
uint8_t speed = 20; // speed is set dynamically once we've started up

// Scale determines how far apart the pixels in our noise matrix are.  Try
// changing these values around to see how it affects the motion of the display.  The
// higher the value of scale, the more "zoomed out" the noise iwll be.  A value
// of 1 will be so zoomed in, you'll mostly see solid colors.

uint8_t scale = 30; // scale is set dynamically once we've started up

// This is the array that we keep our computed noise values in
//uint8_t noise[MAX_DIMENSION][MAX_DIMENSION];

CRGBPalette16 currentPalette( PartyColors_p );
uint8_t       colorLoop = 1;

void setup() {
  delay(1000);
  LEDS.addLeds<LED_TYPE,LED_PIN,COLOR_ORDER>(leds,NUM_LEDS);
  LEDS.setBrightness(BRIGHTNESS);

  // Initialize our coordinates to some random values
  x = random8();
  y = random8();
  z = random8();
}

void mapCoordToColor() {
  // If we're runing at a low "speed", some 8-bit artifacts become visible
  // from frame-to-frame.  In order to reduce this, we can do some fast data-smoothing.
  // The amount of data smoothing we're doing depends on "speed".
  
  uint8_t dataSmoothing = 0;
  static uint8_t ihue=0;
  
  if( speed < 50) {
    dataSmoothing = 200 - (speed * 4);
  }

  for (int i = 0; i < NUM_LEDS; i++) {
//    uint16_t id = ledsArray[i].id;
//    uint8_t xoffset = scale * ledsArray[i].x;    
//    uint8_t yoffset = scale * ledsArray[i].y;

    uint8_t id = ledsArray[i][0];
    uint8_t xoffset = scale * ledsArray[i][1];    
    uint8_t yoffset = scale * ledsArray[i][2];
      
    uint8_t data = inoise8(x + xoffset,y + yoffset,z);

    // The range of the inoise8 function is roughly 16-238.
    // These two operations expand those values out to roughly 0..255
    // You can comment them out if you want the raw noise data.
    data = qsub8(data,16);
    data = qadd8(data,scale8(data,39));

    if( dataSmoothing ) {
      uint8_t olddata = inoise8(x + xoffset,y + yoffset,z-speed);
      uint8_t newdata = scale8( olddata, dataSmoothing) + scale8( data, 256 - dataSmoothing);
      data = newdata;
    }

    // directly set LED color here
    uint8_t index = data;
    uint8_t bri = inoise8(y + yoffset,x + xoffset,z); // TODO make a secong noise for the brightness
    
//    noise[coords[0]][coords[1]] = data; // don't need noise here any more

    // if this palette is a 'loop', add a slowly-changing base value
      if( colorLoop) { 
        index += ihue;
      }

      // brighten up, as the color palette itself often contains the 
      // light/dark dynamic range desired
      if( bri > 127 ) {
        bri = 255;
      } else {
        bri = dim8_raw( bri * 2);
      }

      CRGB color = ColorFromPalette( currentPalette, index, bri);
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

  // generate noise data
//  fillnoise8();
//  
//  // convert the noise data to colors in the LED array
//  // using the current palette
//  mapNoiseToLEDsUsingPalette();

  mapCoordToColor();

  LEDS.show();
  // delay(10);
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
#define HOLD_PALETTES_X_TIMES_AS_LONG 10

void ChangePaletteAndSettingsPeriodically()
{
  uint8_t secondHand = ((millis() / 1000) / HOLD_PALETTES_X_TIMES_AS_LONG) % 60;
  static uint8_t lastSecond = 99;
  
  if( lastSecond != secondHand) {
    lastSecond = secondHand;
    if( secondHand ==  0)  { currentPalette = RainbowColors_p;         speed = 20; scale = 30; colorLoop = 1; }
    if( secondHand ==  5)  { SetupPurpleAndGreenPalette();             speed = 10; scale = 50; colorLoop = 1; }
    if( secondHand == 10)  { SetupBlackAndWhiteStripedPalette();       speed = 20; scale = 30; colorLoop = 1; }
    if( secondHand == 15)  { currentPalette = ForestColors_p;          speed =  8; scale =120; colorLoop = 0; }
    if( secondHand == 20)  { currentPalette = CloudColors_p;           speed =  4; scale = 30; colorLoop = 0; }
    if( secondHand == 25)  { currentPalette = LavaColors_p;            speed =  8; scale = 50; colorLoop = 0; }
    if( secondHand == 30)  { currentPalette = OceanColors_p;           speed = 20; scale = 90; colorLoop = 0; }
    if( secondHand == 35)  { currentPalette = PartyColors_p;           speed = 20; scale = 30; colorLoop = 1; }
    if( secondHand == 40)  { SetupRandomPalette();                     speed = 20; scale = 20; colorLoop = 1; }
    if( secondHand == 45)  { SetupRandomPalette();                     speed = 50; scale = 50; colorLoop = 1; }
    if( secondHand == 50)  { SetupRandomPalette();                     speed = 90; scale = 90; colorLoop = 1; }
    if( secondHand == 55)  { currentPalette = RainbowStripeColors_p;   speed = 30; scale = 20; colorLoop = 1; }
    speed = speed/3;
    scale = scale/5;
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


//
// Mark's xy coordinate mapping code.  See the XYMatrix for more information on it.
//
//uint16_t XY( uint8_t x, uint8_t y)
//{
//  uint16_t i;
//  if( kMatrixSerpentineLayout == false) {
//    i = (y * kMatrixWidth) + x;
//  }
//  if( kMatrixSerpentineLayout == true) {
//    if( y & 0x01) {
//      // Odd rows run backwards
//      uint8_t reverseX = (kMatrixWidth - 1) - x;
//      i = (y * kMatrixWidth) + reverseX;
//    } else {
//      // Even rows run forwards
//      i = (y * kMatrixWidth) + x;
//    }
//  }
//  return i;
//}
