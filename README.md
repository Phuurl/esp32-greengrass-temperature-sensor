# esp32-greengrass-temperature-sensor

ESP32 project to publish the current temperature and humidity (from a DHT11 sensor) to AWS IoT via Greengrass.

This can then be streamed to a DynamoDB table via IoT rules, and entries automatically aged off using the `TTL` field in the payload.

## Payload

The ESP32 will send the following payload to Greengrass:

```json
{
  "DeviceID": "ESP32-temperature",
  "timestamp": 1598798739,
  "tempC": 25.6,
  "Humid": 30,
  "TTL": 1599057939
}
```

- `DeviceID` - the device ID as configured in AWS and `config.h`
- `timestamp` - epoch timestamp of the message
- `tempC` - reported temperature (°C) from the DHT11
- `Humid` - reported % humidity from the DHT11
- `TTL` - epoch TTL timestamp for use with DynamoDB TTL

## Usage

1. Clone the repo.
2. Follow the initial setup instructions at https://github.com/aws-samples/arduino-aws-greengrass-iot. You will need to have setup the libraries, created a Greengrass group, deployed a Core that is accessible from the network that the ESP32 will connect to, and created a Device for the ESP32.
3. Copy and modify the example config files to the root of the project.
   ```bash
   cp config-examples/* ./
   mv aws_certificates-example.c aws_certificates.c
   nano aws_certificates.c
   mv config-example.h config.h
   nano config.h
   ```
4. Connect your DHT11 sensor to your ESP32. Note that if you are using pin #12 for data, you will need to disconnect it during program upload to avoid communication errors.
5. Compile and deploy to the ESP32 via the Arduino IDE.

## Acknowledgements

This project is heavily based in the sample ESP32 Greengrass publish example [by AWS](https://github.com/aws-samples/arduino-aws-greengrass-iot/tree/master/examples/gg_temp_humid_pub_sub), and uses the Arduino AWSGreengrassIoT Library. See [NOTICE](./NOTICE) for copyright information.
