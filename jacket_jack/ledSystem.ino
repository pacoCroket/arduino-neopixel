class LedSystem:
  def __init__(self):
    self.ledDict = {};

  def getLed(self, id):
    if (self.ledDict.has_key(id)):
      return self.ledDict.get(id);

    return null;

  def addLed(self, id, pos):
    self.ledDict[id] = [pos[0], pos[1]];
    
  def addLed(self, pos):
    self.ledDict[self.ledDict.length] = [pos[0], pos[1]];

  def show(self):
    for key, led in self.ledDict.items():
      leds[key] = led.getColor();

    FastLED.show();
