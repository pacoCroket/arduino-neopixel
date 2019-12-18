#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 300 
#define BRIGHTNESS 255
#define DATA_PIN 3

// Define the array of leds
CRGB leds[NUM_LEDS];
int delayTime = 22;
int currentLed = 0;
bool dir = true; // true for forward, false for backwards
static uint8_t hue = 0;
bool ledOn = true;
int bri_scale = 200;

ESP8266WebServer server(80);
const char* ssid = "MOVISTAR_81D2";
const char* password = "rAcRzyJ3S23UPmji3z8m";

// Set web server port number to 80
//WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output5State = "off";
String output4State = "off";

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() { 
  // set up leds
  LEDS.addLeds<WS2812B,DATA_PIN,RGB>(leds,NUM_LEDS);
  LEDS.setBrightness(BRIGHTNESS);

  WiFi.begin(ssid, password);
  Serial.begin(115200);
  Serial.print("Connecting to:");
  Serial.println(ssid);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {              // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.on("/toggle", toggleLedOn);
  server.on("/ledsoff", [](){
    server.send(200, "text/plain", "ledsoff");
    ledOn = false;
  });
  server.on("/setBri", handleSetBri);
  server.begin();

//  MDNS.addService("http", "tcp", 80);
}

void loop() {

  server.handleClient();

  if (!ledOn) {
    fadeall();
  }
  else {
    makeStep();
    fadeall();  
    delay(delayTime);
  }
  FastLED.show(); 
}

void toggleLedOn() {
  ledOn = !ledOn;
  server.send(204,"");
}

void makeStep() {

  leds[currentLed] = CHSV(hue++, 255, dim8_lin(bri_scale));
  hue++;  

  if (dir) {
    if (currentLed > NUM_LEDS) {
      dir = false;
    } else {
      currentLed++;
    }      
  } else {
    if (currentLed <= 0) {      
      dir = true;
    } else {      
      currentLed--;
    }
  }
}

void fadeall() {  
//  handleButton();
  for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(251); }
}

void handleSetBri() {
  Serial.println("Handling new brightness setting...");
  
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
  } else {
    String message = "POST form was:\n";
    for (uint8_t i = 0; i < server.args(); i++) {
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
      if (server.argName(i) == "briValue")
        bri_scale = server.arg(i).toInt();
    }
    server.send(200, "text/plain", message);
    Serial.println(message);
  }
}

void handleRoot() {
  String message = "<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script></head>";
  message += "<body style='font-family: Helvetica; font-size: 16px; display: inline-block; margin: 2px auto; padding: 16px'>Following functions are available:<br><br>";
  message += "<a href='/rainbow?fade=3000'>/rainbow</a> a rainbow animation on LEDs<br>";
  message += "<a href='/wave?r=255&g=32&b=10&fade=5000'>/wave</a> a slow wave animation on LED on base color specified by arguments: r=<0..255> g=<0..255> b=<0..255><br>";
  message += "<a href='/setleds?r=32&g=64&b=32&fade=1000'>/setleds</a> sets LEDs to the color from arguments: r=<0..255> g=<0..255> b=<0..255><br>";
  message += "<a href='/ledsoff?fade=500'>/ledsoff</a> turns off LEDs<br>";
  message += "<a href='/setpins?D1=128&D2=256&D3=512'>/setpins</a> sets to any of the in arguments specified pins (D0..D8) to their PWM values (0..1023). To use them digital: 0=off, 1023=on<br>";
  message += "<a href='/toggle'>/toggle</a> inverts all pin values form pins used before.<br>";
  message += "<a href='/rf?D=6&t=200&id=28013&channel=0&on=1'>/rf</a> sends a rf code from arguments: D=<0..8> t=<0..1000> id=<0..1048575> channel=<0..2> on=<0..1>. Dx is the pin, t is the optional signal clock(default is 200, works for me)<br><br>";
  message += "All functions except togglepins and rf support the argument 'fade' which specifies the milliseconds it takes to fade to the new specified state. ...nice blending ;-)<br>";
  message += "<br>Syntax is as follows: http://&ltip>/&ltcommand>?&ltargument1>=&ltvalue1>&&ltargument2>=&ltvalue2>&...<br>";
  message += "You can click on each link to see an example.<br><br>";
  
  message += "<p>Brightness: <span id=\"ledBrightnessElement\"></span></p>";
  message += "<div class=slidecontainer><input type=range min=0 max=255 value=" + String(bri_scale) + " class=slider id=ledBrightnessSlider></div>";
  message += "<script>var slider = document.getElementById(\"ledBrightnessSlider\");\n";
  message += "var ledBrightness = document.getElementById(\"ledBrightnessElement\"); ledBrightness.innerHTML = (slider.value * 100 / 255).toFixed(0) + ' %';\n";
  message += "slider.onchange = function() { slider.value = this.value; led(this.value); ledBrightness.innerHTML = (slider.value * 100 / 255).toFixed(0) + ' %';}\n";
  message += "$.ajaxSetup({timeout:1000});\n function led(bri) {\n";
  message += "$.post(\"/setBri?briValue=\" + bri + \"&\"); console.log(\"New britghness setting: \" + bri); {Connection: close};}</script>";
  
  message += "\nhave fun -<a href='http://youtube.com/bitlunislab'>bitluni</a></body></html>";
  server.send(200, "text/html", message);
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
