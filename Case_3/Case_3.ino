#include <Wire.h> 
#include <LiquidCrystal.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ThingSpeak.h>
 // For ESP8266
//#include <WiFi.h> // For ESP32, uncomment this line and comment out the line above
#include <Adafruit_MQTT.h>
#include "Adafruit_MQTT_Client.h"
#include "DHT.h"

// WiFi credentials
#define WLAN_SSID "Jessie 🦋"
#define WLAN_PASS "jessicah2021"

// Adafruit IO credentials
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // Use 8883 for SSL
#define AIO_USERNAME    "JessBonnie2024"  // adafruit  username
#define AIO_KEY         "aio_QPtw26iEwhkaoya2nPoDmocQ9sQB" //aio key

WiFiClient client; 
unsigned long myChannelNumber =2497269;  //thinkspeak channel number 
const char * myWriteAPIKey ="JGMWHBH1CXUO6BRQ";  // thinkspeak api key
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish lightIntensityFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/light-intensity-feed");
Adafruit_MQTT_Subscribe ledControl = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/switch");

// Initialize the I2C LCD.
// The constructor arguments are the I2C address (usually 0x27 or 0x3F), columns, and rows of the LCD

#define RS D0
#define E  D8
#define DB4 D5
#define DB5 D4
#define DB6 D3
#define DB7 D2

LiquidCrystal lcd(RS, E, DB4, DB5, DB6, DB7);  // RS, E, DB4, DB5, DB6, DB7

const int lightSensorPin = A0; // The pin where the light sensor is connected
const int ledPin = D1; // LED pin (GPIO 2 for ESP8266, GPIO 5 for ESP32)

void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  ThingSpeak.begin(client); // Initialize ThingSpeak

}


void MQTT_connect() {
  while (mqtt.connected() == false) {
    Serial.print("Attempting MQTT connection...");
    if (mqtt.connect()) {
      Serial.println("connected");
      mqtt.subscribe(&ledControl);
    } else {
      Serial.print("failed, rc=");
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}


void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200); // Start serial communication at 115200 baud rate
  lcd.begin(16, 2); 
  connectToWiFi();
  MQTT_connect();
}

void loop() {
   if (!mqtt.connected()) {
    MQTT_connect();
  }
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &ledControl) {
      long value = atol((char *)ledControl.lastread);
      digitalWrite(ledPin, value);
    }
  }

  int lightIntensity = analogRead(lightSensorPin); // Read the light intensity
  Serial.print("Light Intensity: "); Serial.println(lightIntensity);
  
  if (!lightIntensityFeed.publish(lightIntensity)) {
    Serial.println(F("Failed to publish light intensity"));
  } else {
    Serial.println(F("Light intensity published"));
  }
  ThingSpeak.writeField( 2497269, 1, lightIntensity,"JGMWHBH1CXUO6BRQ");  // channel number and writeapikey

  // Check if the light intensity is lower than 250 lux
  if (lightIntensity < 250) {
    digitalWrite(ledPin, LOW); // Turn on the LED
    lcd.clear(); // Clear the LCD
    lcd.setCursor(0, 0); // Set cursor to the beginning of the first line
    lcd.print("Light is HIGH "); // Print a message
    lcd.setCursor(0, 1); // Move to the second line
    lcd.print("LED ON"); // Indicate that the LED is on

  } else {
    digitalWrite(ledPin, HIGH); // Turn off the LED
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Light is LOW ");
    lcd.setCursor(0, 1);
    lcd.print("LED OFF");
  }
  
  // Regardless of light condition, display the current light intensity
  lcd.setCursor(0, 1); // Move to the second line
  lcd.print("Lux: ");
  lcd.print(lightIntensity); // Show the light intensity value
  delay(1000); // Wait for a second before the next loop iterationa

}

