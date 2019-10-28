#include <FastLED.h>
import re

#define LED_PIN     7
#define NUM_LEDS    299
#define BRIGHTNESS  64
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define UPDATES_PER_SECOND 100

LedSystem ledSystem;
PatternSystem patternSystem;

void setup() {
  // put your setup code here, to run once:
  ledSystem = new LedSystem();
  patternSystem = new PatternSystem();
  LayoutData data = new LayoutData();

  for line in data.leds.split('\n'):
    sys.stdout.write(line);
    
    String[] m = re.match("ID:(\\d+),((\\d+\\.\\d+,?)*)", line, re.M|re.I);
    if (m != null):
      int id = m[1];
      String pos[] = m[2].split(","); // [x,y]
      ledSystem.addLed(id, pos);
      continue;
      
  for line in data.letters.split('\n'):
    sys.stdout.write(line);

    String[] m2 = match("Letter:(\\s|\\S),\\[((\\d+,?)*)]", line); // char,[int, int, int...]
    if (m2 != null):
      Character letter = m2[1].charAt(0);
      IntList idList = new IntList();
      for (String idString : m2[2].split(",")) {
        int ledId = int(idString);
        idList.append(ledId);
      }
//      letterSystem.addLetter(idList, letter);
      continue;
   

}

void loop() {
  // put your main code here, to run repeatedly:
  for key, led in self.ledDict.items():
    // set the led color from the led coordinates
    led.setColor(patternSystem.getLedColor(led));
  
}
