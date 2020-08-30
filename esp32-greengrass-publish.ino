/*
  Portions copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
  Released under Apache 2.0 - see LICENSE for full license.

  Implements publish via the local GreenGrass device.
  Assumes you have a local Greengrass core on the local network.
*/

#include <WiFi.h>

#include <WiFiClientSecure.h>
#include <AWSGreenGrassIoT.h>

#include "DHT.h"

#include <WiFiUdp.h>      // Used for the NTP time sync client
#include <NTPClient.h>    // Time sync used to add timestamp to message

#include "config.h"       // Import config values (eg WiFi creds, AWS endpoints, etc)

DHT dht(DHTPIN, DHTTYPE); // Set the connection pins for DHT sensor

int publishes = 0;
bool first_publish = true;

extern const char aws_root_ca[];
extern const char thingCA[];
extern const char thingKey[];

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

String sensorID;
IPAddress ip;             // the IP address of your shield

const int AmberLedPin = 26;

float Celcius = 0;

AWSGreenGrassIoT * greengrass;

int status = WL_IDLE_STATUS;
int tick = 0;
char payload[512];
bool ConnectedToGGC = false;


void publishToGreengrass(String id, float tempC, float Humid) {
  if (first_publish) {
    timeClient.update();
    Serial.println("Skipping first publish as it tends to break.");
    first_publish = false;
  }
  else {
    unsigned long epoch = 0;

    do {
      timeClient.update();
      epoch = timeClient.getEpochTime();
      Serial.print("Timestamp: ");
      Serial.println(epoch);
    } while (epoch < 1598793801);

    if (publishes < 60) {
      // Keep minute-by-minute for 3 days
      sprintf(payload, JSONPAYLOAD, id.c_str(), epoch, tempC, Humid, (epoch + 259200));
    }
    else {
      // Keep hourly for longer (currently 10 years)
      sprintf(payload, JSONPAYLOAD, id.c_str(), epoch, tempC, Humid, (epoch + 315400000));
    }

    if (greengrass->publish(TOPIC_NAME, payload)) {
      Serial.print("Publish Message: ");
      Serial.println(payload);
      if (publishes < 60) {
        publishes += 1;
      }
      else {
        publishes = 0;
      }
    }
    else {
      Serial.println("Publish failed, retrying...");
      delay(3000);
      publishToGreengrass(id, tempC, Humid);
    }
  }
}


void setup() {
  pinMode(AmberLedPin, OUTPUT);

  digitalWrite(AmberLedPin, HIGH);    // Turn AmberLED on as device is not yet connected.

  Serial.begin(115200);               // Serial port used for status monitoring.
  Serial.println("\n\nDevice started\n");

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(WIFI_SSID);
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(3000);                      // Wait before retry Wifi connection
  }

  Serial.println("Connected to local wifi");

  ip = WiFi.localIP();
  Serial.print("local ip address of this ESP32 = ");
  Serial.println(ip);

  delay(10000);                       // Delay to allow wifi connection to establish
  ConnToGGC();                        // Try to connect to local GreenGrass Core device

  timeClient.begin();                 // Start time client
  dht.begin();                        // Start Sensor polling
}

void ConnToGGC(void) {

  ConnectedToGGC = false;

  while (ConnectedToGGC == false) {
    greengrass = new AWSGreenGrassIoT(AWSIOTURL, THING, aws_root_ca, thingCA, thingKey);

    if (greengrass->connectToGG() == true) {
      Serial.println("Connected to AWS GreenGrass");
      digitalWrite(AmberLedPin, LOW);                 // Turn Amber led off as connected
      ConnectedToGGC = true;                          // Set flag to true

      delay(1000);                                    // Put this in to stop subscribing twice by mistake.
    }
    else {
      Serial.println("Connection to Greengrass failed, check if Greengrass core is on and connected to the WiFi");
      digitalWrite(AmberLedPin, HIGH);   // turn the LED on as connection error
    }
    delay(1000);
  }
}

void loop() {
  if (greengrass->isConnected()) {
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius
    float t = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      delay(1000);
      return;
    }

    publishToGreengrass(THING, t, h);

  } else {
    Serial.println("Greengrass not connected");
    digitalWrite(AmberLedPin, HIGH);  // Lost connection
    // TODO: Put in some method of restart here.
    // ConnectedToGGC = false;
    ConnToGGC();
  }

  delay(publish_frequency);
}
