#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <FirebaseJson.h>
#include <FastLED.h>

#define FIREBASE_HOST "your-firebase-url.firebaseio.com"
#define FIREBASE_AUTH "your-firebase-auth-token"
#define WIFI_SSID "your-wifi-ssid"
#define WIFI_PASSWORD "your-wifi-password"

#define POTENTIOMETER_PIN A0
#define NUM_LEDS 1 // Adjust this based on your LED setup

const int THRESHOLD = 2; // Adjust this threshold as needed

int previousValue = 0;
int currentValue = 0;

CRGB leds[NUM_LEDS];

FirebaseData firebaseData;

void setup()
{
  Serial.begin(115200);

  pinMode(POTENTIOMETER_PIN, INPUT);
  currentValue = analogRead(POTENTIOMETER_PIN);
  previousValue = currentValue;

  FastLED.addLeds<WS2812, D3, GRB>(leds, NUM_LEDS); // Define your LED setup

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  Serial.println("Setup complete.");
}

void loop()
{
  currentValue = analogRead(POTENTIOMETER_PIN);

  if (abs(currentValue - previousValue) >= THRESHOLD)
  {
    Serial.print("Potentiometer value changed: ");
    Serial.println(currentValue);

    updateFirebase(currentValue);
    updateLampColor(currentValue);

    previousValue = currentValue;
  }
  else
  {
    updateLampColorFromDatabase();
  }

  delay(100); // Adjust the delay as needed
}

void updateFirebase(int value)
{
  if (Firebase.setString(firebaseData, "/lamp/colorHSL", "120,255,128"))
  {
    Serial.println("Firebase update successful!");
  }
  else
  {
    Serial.println("Firebase update failed.");
    Serial.println("Reason: " + firebaseData.errorReason());
  }
}

void updateLampColor(int value)
{
  // Calculate hue from value
  int hue = map(value, 0, 1023, 0, 255);

  // Set LED color based on calculated hue
  fill_solid(leds, NUM_LEDS, CHSV(hue, 255, 255));
  FastLED.show();
}

void updateLampColorFromDatabase()
{
  if (Firebase.getString(firebaseData, "/lamp/colorHSL"))
  {
    String hslString = firebaseData.stringData();
    Serial.print("HSL color from Firebase: ");
    Serial.println(hslString);

    int hue, saturation, lightness;
    sscanf(hslString.c_str(), "%d,%d,%d", &hue, &saturation, &lightness);

    // Convert HSL to RGB using FastLED
    CRGB rgbColor = CHSV(hue, saturation, lightness);

    // Set the LED color
    leds[0] = rgbColor;
    FastLED.show();
  }
  else
  {
    Serial.println("Failed to get HSL color from Firebase.");
    Serial.println("Reason: " + firebaseData.errorReason());
  }
}