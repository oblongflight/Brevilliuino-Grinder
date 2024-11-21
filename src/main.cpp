#include <stdint.h>
#include <Arduino.h>
#include <ResponsiveAnalogRead.h>
#include <EasyButton.h>
#include <EasyButtonBase.h>
#include <Sequence.h>
#include <HX711.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Adafruit_TLC5947.h>
#include <Wifi.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>
#include "NotoSansBold15.h"
#include "NotoSansBold36.h"
#include "NotoSans96.h"
#include <credentials.h> //must create this file and #define SinricPro credentials: WIFI_SSID, WIFI_PASS, APP_KEY, APP_SECRET, and SWITCH_ID
#include <bean.h>
#include <gaggiuinoLogo.h>
#define AA_FONT_SMALL NotoSansBold15
#define AA_FONT_LARGE NotoSansBold36
#define AA_FONT_HUGE NotoSans96

Adafruit_TLC5947 tlc = Adafruit_TLC5947(1, 17, 18, 21); // # of TLC5947s (1), clock, data, latch pins

const int dout = 1; // load cell dout pin
const int sck = 2;  // load cell sck pin
HX711 LC;

const int grindRelay = 10; // grinder relay signal pin
const int powerRelay = 3;  // whole machine power relay signal pin

const int potPin = 16; // potentiometer ADC pin
float potValue = 0;

ResponsiveAnalogRead pot(potPin, true);

EasyButton grindtareButton(12);
EasyButton powerButton(11);

float mass = 0;
float massPrevious = 1;
float target = 18;
float targetPrevious;

bool menu = 1; // menu state (1 if in mass selection mode, 0 if in mass display mode)
unsigned long currentTime = 0;
unsigned long previousTime = 0;

bool powerOn = 0;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
TFT_eSprite spr_up = TFT_eSprite(&tft);
TFT_eSprite spr_down = TFT_eSprite(&tft);

const int backlight = 15;

unsigned long progressbarTimer = 0;
unsigned long progressbarInterval = 10;
unsigned long progressbarProgress = 0;

void resetLEDs()
{
  tlc.setPWM(6, 3600);
  tlc.setPWM(7, 3600);
  tlc.setPWM(8, 3600);
  tlc.setPWM(9, 1080);
  tlc.setPWM(10, 1080);
  tlc.setPWM(11, 3600);
  tlc.setPWM(12, 3600);
  tlc.setPWM(13, 3600);
  tlc.setPWM(14, 3600);
  tlc.setPWM(15, 1080);
  tlc.setPWM(16, 1080);
  tlc.write();
}

void grindtareButton_Hold()
{
  LC.tare();
  resetLEDs();
}

void grindtareButton_Press()
{
  if (digitalRead(grindRelay) == HIGH)
  {
    digitalWrite(grindRelay, LOW);
    return;
  }
  else if (digitalRead(grindRelay) == LOW)
  {
    digitalWrite(grindRelay, HIGH);
  }
  resetLEDs();
}

void powerButton_Press()
{
  if (powerOn == 1)
  {
    while (progressbarProgress < 360)
    {
      if (millis() - progressbarTimer > progressbarInterval)
      {
        progressbarTimer = millis();
        progressbarProgress += 4;
        tlc.setPWM(6, 4095 - progressbarProgress * 10);
        tlc.setPWM(7, 3600 - progressbarProgress * 10);
        tlc.setPWM(8, 3600 - progressbarProgress * 10);
        tlc.setPWM(9, 3600 - progressbarProgress * 10);
        tlc.setPWM(10, 3600 - progressbarProgress * 10);
        tlc.setPWM(11, 3600 - progressbarProgress * 10);
        tlc.setPWM(12, 3600 - progressbarProgress * 10);
        tlc.setPWM(13, 3600 - progressbarProgress * 10);
        tlc.setPWM(14, 3600 - progressbarProgress * 10);
        tlc.setPWM(15, 3600 - progressbarProgress * 10);
        tlc.setPWM(16, 3600 - progressbarProgress * 10);
        tlc.write();
      }
    }
    powerOn = 0;
    digitalWrite(powerRelay, LOW);
    digitalWrite(grindRelay, LOW);
    digitalWrite(backlight, LOW);
    tft.fillScreen(TFT_BLACK);
    SinricProSwitch &mySwitch = SinricPro[SWITCH_ID];
    mySwitch.sendPowerStateEvent(powerOn);
    return;
  }
  else if (powerOn == 0)
  {
    powerOn = 1;
    digitalWrite(backlight, HIGH);
    digitalWrite(powerRelay, HIGH);
    previousTime = millis();
    tft.pushImage(0, 0, 320, 170, gaggiuino);
    progressbarProgress = 0;
    while (progressbarProgress < 360)
    {
      if (millis() - progressbarTimer > progressbarInterval)
      {
        progressbarTimer = millis();
        tft.drawSmoothRoundRect(10, 130, 10, 0, map(progressbarProgress, 0, 360, 0, 300), 20, TFT_ORANGE, TFT_BLACK);
        progressbarProgress += 1;
        tlc.setPWM(6, progressbarProgress * 10);
        tlc.setPWM(7, progressbarProgress * 10);
        tlc.setPWM(8, progressbarProgress * 10);
        tlc.setPWM(9, progressbarProgress * 3);
        tlc.setPWM(10, progressbarProgress * 3);
        tlc.setPWM(11, progressbarProgress * 10);
        tlc.setPWM(12, progressbarProgress * 10);
        tlc.setPWM(13, progressbarProgress * 10);
        tlc.setPWM(14, progressbarProgress * 10);
        tlc.setPWM(15, progressbarProgress * 3);
        tlc.setPWM(16, progressbarProgress * 3);
        tlc.write();
      }
    }
    tft.fillScreen(TFT_BLACK);
    SinricProSwitch &mySwitch = SinricPro[SWITCH_ID];
    mySwitch.sendPowerStateEvent(powerOn);
    LC.tare();
  }
  progressbarProgress = 0;
}

void displayMass()
{
  if (massPrevious > mass + 0.1 || massPrevious < mass - 0.1)
  {
    spr.setTextColor(TFT_GREEN, TFT_BLACK);
    spr.loadFont(AA_FONT_HUGE);
    if (mass < -0.1)
    {
      spr.fillSprite(TFT_BLACK);
      spr.setTextDatum(BR_DATUM);
      spr.drawString("-----", 300, 100);
    }
    else
    {
      spr.fillSprite(TFT_BLACK);
      spr.setTextDatum(BR_DATUM);
      spr.drawString("   " + String(fabs(mass), 1) + "g", 300, 100);
    }
    spr.pushSprite(0, 40);
    massPrevious = mass;
  }
}

void displayMenu()
{
  if (targetPrevious != target)
  {
    spr.setTextColor(TFT_ORANGE, TFT_BLACK);
    spr.loadFont(AA_FONT_HUGE);
    spr.fillSprite(TFT_BLACK);
    spr.setTextDatum(BR_DATUM);
    spr.drawString("   " + String(target, 1) + "g", 300, 100);
    spr.pushSprite(0, 40);
    targetPrevious = target;
  }
  massPrevious = 0;
}

void progressBar()
{
  int progress = round(map(mass, 0, target, 0, 16));
  tft.pushImage((progress * 21), 20, 15, 20, bean);
}

bool onPowerState(const String &deviceId, bool &state)
{
  powerButton_Press();
  return true; // request handled properly
}

void setup()
{

  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  SinricProSwitch &mySwitch = SinricPro[SWITCH_ID];
  mySwitch.onPowerState(onPowerState);
  SinricPro.begin(APP_KEY, APP_SECRET);

  tlc.begin();

  pinMode(backlight, OUTPUT);
  digitalWrite(backlight, HIGH);

  pinMode(grindRelay, OUTPUT);
  digitalWrite(grindRelay, LOW);

  pinMode(powerRelay, OUTPUT);
  digitalWrite(powerRelay, LOW);

  grindtareButton.begin();
  powerButton.begin();

  pot.setAnalogResolution(4096);

  LC.begin(dout, sck);
  LC.set_scale(2237.f); // calibration factor

  tft.begin();
  tft.setRotation(3); // rotate to landscape with buttons on the left
  tft.fillScreen(TFT_BLACK);

  spr.setColorDepth(16);
  spr.createSprite(300, 100);

  delay(250);

  tft.fillScreen(TFT_BLACK);

  LC.tare();

  grindtareButton.onPressed(grindtareButton_Press);
  grindtareButton.onPressedFor(1000, grindtareButton_Hold);
  powerButton.onPressed(powerButton_Press);

  powerOn = 0;
}

void loop()
{

  powerButton.read();
  SinricPro.handle();

  if (powerOn == 1)
  {

    pot.update();
    potValue = pot.getValue();
    target = potValue / 100 + 16;

    mass = LC.get_units(1);
    grindtareButton.read();

    // if target mass knob is moved AND grinding isn't ongoing, change the target and display target mass selector screen
    if (target > targetPrevious + 0.1 || target < targetPrevious - 0.1 && digitalRead(grindRelay) == LOW)
    {
      targetPrevious = target;
      previousTime = millis();
      menu = 1;
      resetLEDs();
    }

    if (menu == 1)
    {

      displayMenu();
      currentTime = millis();

      if (currentTime - previousTime >= 2000)
      {
        menu = 0;
        tft.fillScreen(TFT_BLACK);
        massPrevious = 100;
      }
    }
    else
    {

      displayMass();

      // display progress bar while grinding
      if (digitalRead(grindRelay) == HIGH)
      {
        progressBar();
      }

      // stop grinding .2 grams before target to account for delay in grounds dropping into hopper
      if (digitalRead(grindRelay) == HIGH && mass >= target - 0.2)
      {
        digitalWrite(grindRelay, LOW);
        tft.fillRect(0, 20, 320, 21, TFT_BLACK);
      }
    }
  }
}