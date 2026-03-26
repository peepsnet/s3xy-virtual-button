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

Logger logger(Debug, "[{time}] {level}: [{file}.{function}.{lineno}] {message}");

AsyncWebServer ASYNC_SERVER(80);

#define    INPIN                  D8    // INPUT from Switch

#define SCREEN_WIDTH              128 // OLED display width,  in pixels
#define SCREEN_HEIGHT             64 // OLED display height, in pixels
// declare an SSD1306 display object connected to I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const char* AP_SSID               = "S3XY_Buttton";
const char* AP_PASSWD             = "9548951411!";
String WiFI_SSID                  = "SheltAirGuestWiFi";
String WiFi_PASSWD                = "1234567890!";
//String WiFI_SSID                = "3844-Main-2.4G";
//String WiFi_PASSWD              = "";
const char* hostName              = "s3xy";

long delayLowMin;
long delayLowMax;

bool powerSwitch                  = false;
bool useSwitch                    = true;

unsigned long timeNow             = 0;
unsigned long delayBy             = 0;

OneButton btn;

static void randomize() {
  delayLowMin                      = random(9500, 10000);
  delayLowMax                      = random(12000, 12500);
  delayBy                          = random(delayLowMin, delayLowMax);
}

void oled_init() {
  int attempts = 0;
  const int maxAttempts = 5;
  bool initialized = false;

  while (attempts < maxAttempts && !initialized) {
      initialized = oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
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
      oled.clearDisplay();
      oled.display();
  }
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
  //LOG(Debug, "%s", var);
  LOG(Debug, "Replacing: %s", var);
  if (powerSwitch) {
    return "checked";
  }
  return " ";
}

void WiFiEvent(WiFiEvent_t event) {
  switch(event) {
      case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        LOG(Debug, "Connected to AP: %s", "test:" );
        break;

      case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        LOG(Debug, "DHCP WiFi IP: %s", WiFi.localIP().toString().c_str() );
        LOG(Debug, "WiFi Mac Address: %s", WiFi.macAddress().c_str() );
        break;

      case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        LOG(Debug, "Not Connected to a WiFI Router/AP. Trying to Reconnecting...");
        WiFi.begin(WiFI_SSID, WiFi_PASSWD);
        break;

      case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
        LOG(Debug, "Someone connected to the AP...");
        break;
      
      case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
        LOG(Debug, "Someone disconnected to the AP...");
        break;

      case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
        LOG(Debug, "AP IP Assigned: %s", WiFi.softAPIP().toString());

        break;

      default:
        break;
  }
}

void setup() {  
  Serial.begin(115200);
  blinkLED(3);

  oled_init();  

  delay(1000);
  oled.clearDisplay();

  LOG(Debug, "Power on. Setup Begins");
  LOG(Debug, "Setting up AP");
  WiFi.setHostname(hostName);
  WiFi.onEvent(WiFiEvent);
  WiFi.mode(WIFI_AP_STA);
  LOG(Debug, "AP IP address: %s", WiFi.softAPIP().toString() );
  LOG(Debug, "AP MAC Address: %s", WiFi.softAPmacAddress().c_str() );

  delay(1000);

  WiFi.begin(WiFI_SSID, WiFi_PASSWD);
  LOG(Debug, "Connectiong to WiFi Router if available...");
  int x = 0;
  while ( (WiFi.status() != WL_CONNECTED) && (x < 10) ) {
    delay(1000);
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