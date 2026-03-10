#include <Arduino.h>
#include "S3XYButton.h"
#include <OneButton.h>

#define    INPIN                  D8    // INPUT from Switch

long delayLowMin;
long delayLowMax;

bool powerSwitch                = false;

unsigned long timeNow           = 0;
unsigned long delayBy           = 0;

OneButton btn;

static void randomize() {
  delayLowMin                      = random(9500, 10000);
  delayLowMax                      = random(12000, 12500);
  delayBy                          = random(delayLowMin, delayLowMax);
}


static void setLED() {
  if (powerSwitch) {
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  Serial.println("Power LED status set/reset!!");
}

static void blinkLED(int blink) {
  for (int i = 1; i <= blink; i++) {
    digitalWrite(LED_BUILTIN, HIGH);          //LED OFF
    delay(75);
    digitalWrite(LED_BUILTIN, LOW);           //LED ON
    delay(75);
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
  Serial.println("S3XY Button Pressed!!");
}

static void doubleClickS3XYButton() {
  s3xy_send_double();
  blinkLED(2);
  Serial.println("S3XY Button Double Clicked!!");
}

static void multiClickS3XYButton() {
  s3xy_send_long();
  blinkLED(3);
  Serial.println("S3XY Button Long Clicked!!");
}

static void toggleOnOff() {
  powerSwitch = !powerSwitch;
  blinkLED(2);
  timeNow = millis();
  Serial.println("3x Clicked! Power Toggled!!");
}

void onConnected() {
  blinkLED(2);
  Serial.println("[user] connected"); 
}

void onDisconnected() {
  blinkLED(2);
  Serial.println("[user] disconnected");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  s3xy_on_connect(onConnected);
  s3xy_on_disconnect(onDisconnected);
  s3xy_begin("ENH_BTN");

  Serial.println("Setup Begins");

  pinMode(INPIN, INPUT);    
  pinMode(LED_BUILTIN, OUTPUT);

  timeNow = millis();
  randomize();

  btn.setup(INPIN, INPUT_PULLUP, false);
  btn.attachClick(pressS3XYBtnNow);
  btn.attachDoubleClick(doubleClickS3XYButton);
  btn.attachLongPressStart(multiClickS3XYButton);
  btn.attachMultiClick(toggleOnOff);
  btn.reset();

  blinkLED(2);

  Serial.println("Setup Complete");

}

void loop() {
  //s3xy_loop();

  if (s3xy_ready() ) {
    //save this for referance
  }
    btn.tick();   //Capture control button status
  
  if (millis() - timeNow > delayBy && powerSwitch) {
    //Serial.println("Pressed: " + String((millis() - timeNow)*0.001) + "s");
    pressS3XYBtnNow();
    timeNow = millis();
    randomize();
  Serial.println("Loop: " + String((millis() - timeNow)*0.001) + "s. D8 = " + digitalRead(INPIN));
}