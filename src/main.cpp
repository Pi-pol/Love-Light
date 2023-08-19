#include <Arduino.h>
#include <WiFi.h>
#include <FastLED.h>
#include <Firebase_ESP_Client.h>
#include "secrets.h" // Has api key and database url
// Upload: C:\Users\micha\.platformio\penv\Scripts\platformio.exe run --target upload
// Monitor: C:\Users\micha\.platformio\penv\Scripts\platformio.exe device monitor
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

#define POTENTIOMETER_PIN 35
#define POTENTIOMETER_PIN2 34
// leds
#define LED_PIN 21
#define NUM_LEDS 20
uint8_t Brightness = 0;
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
#define UPDATES_PER_SECOND 100
CRGBPalette16 currentPalette;
TBlendType currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;
// leds
int intValue = 0;
const int THRESHOLD = 10; // Adjust this threshold as needed

int previousValue = 30;
int currentValue = 0;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
void setup()
{
  pinMode(5, INPUT);
  pinMode(15, INPUT);
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("Firebase ok!");
    signupOK = true;
  }
  else
  {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  // leds
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // leds
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void updateFirebase(int value)
{
  sendDataPrevMillis = millis();
  // Write an Int number on the database path test/int
  if (Firebase.RTDB.setInt(&fbdo, "lamps/hue", value))
  {
    Serial.println("PASSED");
  }
  else
  {
    Serial.println("FAILED");
  }
}

void updateLampColor(int value)
{
  // sth sth fastledrandombulshitgo ->number in the database is already in hue
}

int updateLampColorFromDatabase()
{
  if (Firebase.RTDB.getInt(&fbdo, "/lamps/hue"))
  {
    Serial.print("Hue Value: ");
    Serial.println(fbdo.dataPath());
    Serial.print("Hue type: ");
    Serial.println(fbdo.dataType());
    Serial.print("Hue value: ");
    Serial.println(fbdo.intData());
    if (fbdo.dataType() == "int")
    {
      intValue = fbdo.intData();
      Serial.println(intValue);
      for (size_t i = 0; i < 20; i++)
      {
        leds[i] = CHSV(intValue, 255, 128);
      }
      return intValue;
    }
  }
  else
  {
    Serial.println(fbdo.errorReason());
  }
  return 0;
}
uint8_t h;
bool shouldUpdateDB = false;
void loop()
{
  for (int i = 0; i < 20; i++)
  {
    leds[i] = CHSV(h, 255, 255);
  }
  FastLED.show();
  currentValue = analogRead(POTENTIOMETER_PIN);
  // Serial.println(currentValue);
  uint8_t value = 0;
  if (abs(currentValue - previousValue) >= THRESHOLD)
  {
    Serial.print("Potentiometer value changed: ");
    value = map(currentValue, 0, 4096, 0, 255);
    Serial.println(value);
    previousValue = currentValue;
    h = value;
    shouldUpdateDB = true;
  }
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();

    if (shouldUpdateDB)
    {
      Serial.println(value);

      updateFirebase(value);
      updateLampColor(value);

      previousValue = currentValue;
      shouldUpdateDB = false;
    }
    else
    {
      h = updateLampColorFromDatabase();
    }
  }
  delay(20); // Adjust the delay as needed
}
