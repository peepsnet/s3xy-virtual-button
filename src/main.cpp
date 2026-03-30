#include <Arduino.h>
#include <Logger.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneButton.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "S3XYButton.h"
#include "html.h"
#include "images.h"

Logger logger(Debug, "[{time}] {level}: [{file}.{function}.{lineno}] {message}");

AsyncWebServer ASYNC_SERVER(80);

//#define    INPIN                  D8    // INPUT from Switch

#define SCREEN_WIDTH              128 // OLED display width,  in pixels
#define SCREEN_HEIGHT             64 // OLED display height, in pixels
// declare an SSD1306 display object connected to I2C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
#define LOGO_W_S3XY   128
#define LOGO_H_S3XY   50

const char* AP_SSID                 = "S3XY_Buttton";
const char* AP_PASSWD               = "9548951411!";
String WiFI_SSID                    = "SheltAirGuestWiFi";
String WiFi_PASSWD                  = "1234567890!";
//String WiFI_SSID                  = "3844-Main-2.4G";
//String WiFi_PASSWD                = "";
const char* hostName                = "s3xy";

long delayLowMin;
long delayLowMax;

bool powerSwitch                    = false;
bool useSwitch                      = true;

unsigned long timeNow               = 0;
unsigned long delayBy               = 0;

OneButton btn;

static void randomize() {
  delayLowMin                       = random(8500, 9000);
  delayLowMax                       = random(11000, 11500);
  delayBy                           = random(delayLowMin, delayLowMax);
}

static void setLED() {
  if (powerSwitch) {
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  LOG(Debug, "Power LED status set/reset!!");
}

static void blinkLED(int blink, int dly = 75) {
  for (int i = 1; i <= blink; i++) {
    digitalWrite(LED_BUILTIN, HIGH);          //LED OFF
    delay(dly);
    digitalWrite(LED_BUILTIN, LOW);           //LED ON
    delay(dly);
    if (!powerSwitch) {
      digitalWrite(LED_BUILTIN,HIGH);           //LED OFF
      delay(75);
    }
  }
  setLED();
}

// Handler function for a single click:
static void pressS3XYBtnNow() {
  s3xy_send_single();
  blinkLED(1);
  LOG(Debug, "S3XY Button Pressed!!");
}

static void doubleClickS3XYButton() {
  s3xy_send_double();
  blinkLED(2);
  LOG(Debug, "S3XY Button Double Clicked!!");
}

static void longClickS3XYButton() {
  s3xy_send_long();
  blinkLED(1, 200);
  LOG(Debug, "S3XY Button Long Clicked!!");
}

static void toggleOnOff() {
  powerSwitch = !powerSwitch;
  blinkLED(3);
  timeNow = millis();
  LOG(Debug, "Power Toggled!!");
}

static void toggleSwitch() {
  useSwitch = !useSwitch;
  blinkLED(3);
  LOG(Debug, "Switch function toggled!!");
}

void onS3XYConnected() {
  blinkLED(2);
  LOG(Debug, "Virtural S3XY Button Connected"); 
}

void onS3XYDisconnected() {
  blinkLED(2);
  LOG(Debug, "Virtural S3XY Button Connected"); 
}

String htmlProcessor(const String& var) {
  LOG(Debug, "Replacing: %s", var);
  if (powerSwitch) {
    return "checked";
  }
  return " ";
}

void drawS3XYTitle(String myStr) {
  display.setFont();
  display.setTextSize(2);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.print(myStr);
}

void drawS3XYAPIP(String myIP) {
  int y = 16;
  display.setFont();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, y);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.write("AP:");
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setCursor(0, (y+12));     
  display.print(myIP);
}

void drawS3XYDHCPIP(const String& myIP) {
  int y = 44;
  display.setFont();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, y);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.write("DHCP:");
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setCursor(0, (y+12));     
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.print(myIP);
}

void drawS3XYbitmap(void) {
  display.clearDisplay();
  display.drawBitmap(
    (display.width()  - LOGO_W_S3XY ) / 2,
    (display.height() - LOGO_H_S3XY) / 2,
    image_data_Image, LOGO_W_S3XY, LOGO_H_S3XY, 1);
  display.display();
  delay(1500);
}

void d(char myChar) {
  display.setFont();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, ((SCREEN_HEIGHT/2)+6) );   
  display.write(myChar);
}

void oled_init() {
  int attempts = 0;
  const int maxAttempts = 5;
  bool initialized = false;

  while (attempts < maxAttempts && !initialized) {
      initialized = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
      if (!initialized) {
          LOG(Debug, "SSD1306 allocation failed, retrying...");
          attempts++;
          delay(1000); // Wait before retrying
      }
  }

  if (!initialized) {
      LOG(Debug, "Failed to initialize SSD1306 after several attempts.");
  } else {
      LOG(Debug, "SSD1306 initialized successfully.");
      display.clearDisplay();
      display.display();
  }
}

String myAPIP = "";
String myAPMAC = "";
String myWIFIIP= "";
String myWIFIMAC = "";

void WiFiEvent(WiFiEvent_t event) {
  switch(event) {
      case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        LOG(Debug, "Connected to Router");
        drawS3XYDHCPIP("Obtaining IP...");
        display.display();
        break;

      case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        myWIFIIP= WiFi.localIP().toString().c_str();
        myWIFIMAC = WiFi.macAddress().c_str();
        LOG(Debug, "DHCP WiFi IP: %s",  myWIFIIP);
        LOG(Debug, "WiFi Mac Address: %s",  myWIFIMAC);
        drawS3XYDHCPIP(myWIFIIP);
        display.display();
        break;

      case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        LOG(Debug, "Not Connected to a WiFI Router/AP. Trying to Reconnecting...");
        WiFi.begin(WiFI_SSID, WiFi_PASSWD);
        drawS3XYDHCPIP("Connecting...");
        display.display();
        break;

      case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
        LOG(Debug, "Someone connected to the AP...");
        break;
      
      case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
        LOG(Debug, "Someone disconnected from the AP...");
        break;

      case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
        myAPIP = WiFi.softAPIP().toString();
        myAPMAC = WiFi.softAPmacAddress().c_str();
        LOG(Debug, "AP IP address: %s", myAPIP );
        LOG(Debug, "AP MAC Address: %s", myAPMAC );
        drawS3XYAPIP(myAPIP);
        display.display();
        break;

      default:
        break;
  }
}

void setup() {  
  Serial.begin(115200);
  LOG(Debug, "Begining...");
  blinkLED(3);
  
  oled_init();
  delay(500);

  LOG(Debug, "Display Logo!");
  display.clearDisplay();
  //drawS3XYbitmap();
  display.display();
  
  display.clearDisplay();
  drawS3XYTitle("Loading...");
  display.display();

  LOG(Debug, "Power on. Setup Begins");
  LOG(Debug, "Setting up AP");
  drawS3XYAPIP("Starting...");
  display.display();
  delay(1500);

  WiFi.setHostname(hostName);
  WiFi.onEvent(WiFiEvent);
  WiFi.mode(WIFI_AP_STA);
  delay(1000);

  WiFi.begin(WiFI_SSID, WiFi_PASSWD);
  LOG(Debug, "Connectiong to WiFi Router if available...");
  drawS3XYDHCPIP("Connection...");
  display.display();

  int x = 0;
  while ( (WiFi.status() != WL_CONNECTED) && (x < 40) ) {
    delay(250);
    LOG(Debug, ".%d", x);
    x++;
  }

  ASYNC_SERVER.on("/", HTTP_GET, [](AsyncWebServerRequest *my_request) {
    my_request->send(200, "text/html", index_html, htmlProcessor);
  });
  
  ASYNC_SERVER.on("/toggle_power", HTTP_GET, [] (AsyncWebServerRequest *request) {
    toggleOnOff();
    request->send(200, "text/plain", "Power Toggled");
  });

  ASYNC_SERVER.on("/block_switch", HTTP_GET, [] (AsyncWebServerRequest *request) {
    toggleSwitch();
    request->send(200, "text/plain", "Switch Function Toggled");
  });

  ASYNC_SERVER.on("/click", HTTP_GET, [] (AsyncWebServerRequest *request) {
    pressS3XYBtnNow();
    request->send(200, "text/plain", "Clicked Once");
  });

  ASYNC_SERVER.on("/dbl_click", HTTP_GET, [] (AsyncWebServerRequest *request) {
    doubleClickS3XYButton();
    request->send(200, "text/plain", "Double Clicked");
  });

  ASYNC_SERVER.on("/long_click", HTTP_GET, [] (AsyncWebServerRequest *request) {
    longClickS3XYButton();
    request->send(200, "text/plain", "Long Clicked");
  });

  ASYNC_SERVER.begin();

  s3xy_on_connect(onS3XYConnected);
  s3xy_on_disconnect(onS3XYDisconnected);
  s3xy_begin("ENH_BTN");

  pinMode(INPIN, INPUT);    
  pinMode(LED_BUILTIN, OUTPUT);

  timeNow = millis();
  randomize();

  btn.setup(INPIN, INPUT_PULLUP, false);
  btn.attachClick(pressS3XYBtnNow);
  btn.attachDoubleClick(doubleClickS3XYButton);
  btn.attachLongPressStart(longClickS3XYButton);
  btn.attachMultiClick(toggleOnOff);
  btn.reset();

  blinkLED(2);

  LOG(Debug, "Setup Complete");

}

void loop() {
  //s3xy_loop();

  //if (s3xy_ready() ) {
    //save this for referance
  //}
  
  if ( useSwitch ) {
      btn.tick();   //Capture control button status
  }

  if (millis() - timeNow > delayBy && powerSwitch) {
    LOG(Debug, "Pressed: %s s", String((millis() - timeNow)*0.001));
    pressS3XYBtnNow();
    timeNow = millis();
    randomize();
  }
}