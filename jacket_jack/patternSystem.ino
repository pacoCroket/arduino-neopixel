class PatterSystem:
  def __init__(self):
//    int maxCount = 200;
    uint16_t self.zoff = random(1000);
    uint16_t self.zoff2 = random(1000);
//    float incSpread = 0.0002;
//    float[] colorSpread = {1.7, 0.5};  //[center, range/2]
  
//    int intensifySyncDuration = 15000; // will fade out
//    float[] syncRange1 = {0.8, 1.25};
//    float[] syncRange2 = {0.8, 1.25};

  def getLedColor(self, led):
    uint16_t hue = inoise16 (led.x, led.y, self.zoff);
    uint16_t bri = inoise16 (led.x, led.y, self.zoff2);

    
