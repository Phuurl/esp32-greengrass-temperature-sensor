#define DHTPIN 12  // what pin we're connected to
#define DHTTYPE DHT11  // DHT 11  (AM2302)

int publish_frequency = 60000;  // Time in ms to publish sensor data

char WIFI_SSID[] = "wifi";  // Your WiFi SSID
char WIFI_PASSWORD[] = "password";  // Your WiFi password
char AWSIOTURL[] = "xxxx.iot.eu-west-1.amazonaws.com";  // Your AWS IoT core endpoint (this can be found in under settings in the AWS IoT console)
char THING[] = "ESP32-temperature";  // This name must match the name of your device in AWS IoT
char TOPIC_NAME[] = "topic/subtopic";  // Topic you wish the device to publish to.

const char JSONPAYLOAD[] = "{ \"DeviceID\" : \"%s\", \"timestamp\": %lu, \"tempC\": %f, \"Humid\": %f, \"TTL\": %lu}";
