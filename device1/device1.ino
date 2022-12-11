#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoMqttClient.h>
#include "M5Atom.h"
#include "M5_ENV.h"
#include <Arduino_JSON.h>
#include "Adafruit_SGP30.h"
#include <Wire.h>
#include <algorithm>

// Board Info
String ID = "ESP32_Device1";

// WiFi Info
const char WIFI_SSID[] = "WIFI_SSID"; //TODO: Change
const char WIFI_PASSWORD[] = "WIFI_PASSWORD"; //TODO: Change

// AWS IoT Core Connection Information
const char AWS_IOT_ENDPOINT[] = "AWS_IOT_ENDPOINT"; //TODO: Change
const char* SUBSCRIBED_TOPICS[3]
        = { "info", "new_request", "finished" };

const char initialization[]  = "initialization";
const char sensor_readings[] = "sensor_readings";
const char product_pickup[]  = "product_pickup";

// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
TODO: Change
-----END CERTIFICATE-----
)EOF";

// Device Certificate
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
TODO: Change
-----END CERTIFICATE-----
)KEY";

// Device Private Key
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
TODO: Change
-----END RSA PRIVATE KEY-----
)KEY";

WiFiClientSecure wifiClient = WiFiClientSecure();
MqttClient mqttClient(wifiClient);

//set interval for sending messages = 30 sec
const long interval = 30000;
unsigned long previousMillis = 0;
unsigned long previousButton = 0;

JSONVar sensorRead;
JSONVar initial;
JSONVar productPickup;

Adafruit_SGP30 sgp;

int button = 0;  // Store the number of button presses.

String product = "";
int quantity = 0;

void setup() {
  // IoT device initialization
  M5.begin(true, false, true);
  // Atom Matrix I2C GPIO Pin is 26 and 32
  Wire.begin(26, 32);         
  delay(50); 
  M5.dis.fillpix(0xff0000);  // Set initial color to red

  // initialize the serial port
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  if (! sgp.begin()){
    Serial.println("Sensor not found :(");
    while (1);
  }

  // Connect to Wi-Fi network with SSID and password
  Serial.println("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Print local IP address 
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Connect to AWS IoT
  wifiClient.setCACert(AWS_CERT_CA);
  wifiClient.setCertificate(AWS_CERT_CRT);
  wifiClient.setPrivateKey(AWS_CERT_PRIVATE);
  if (mqttClient.connect(AWS_IOT_ENDPOINT, 8883)) {
    Serial.println("You're connected to the MQTT broker!");
    Serial.println();
  } else {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
  }

  // Subscribe to MQTT and register a callback
  mqttClient.onMessage(messageHandler);
  for (int i = 0; i < 3; i++)
    mqttClient.subscribe(SUBSCRIBED_TOPICS[i]);

  initial["id"] = ID;

  String jsonString = JSON.stringify(initial);
  Serial.println(jsonString);

  mqttClient.beginMessage(initialization);
  mqttClient.print(initial);
  mqttClient.endMessage();
}

int requested_quantity = 0;

// Handle a received message
void messageHandler(int messageSize) {
  Serial.print("Received a message with topic ");
  String topic = mqttClient.messageTopic();
  Serial.println(topic);

  String message;
  String aux_product;
  String curr_id;

  while (mqttClient.available()) {
    message += (char)mqttClient.read();
  }

  JSONVar command = JSON.parse(message);
  Serial.println(command);

  if (topic == "info") {
    if (command.hasOwnProperty("id")) {
      curr_id = (const char*) command["id"];
      if(curr_id == ID) {
        if (command.hasOwnProperty("product")) {
          product = (const char*) command["product"];
          Serial.println((const char*) command["product"]);
        }
        if (command.hasOwnProperty("quantity")) {
          quantity = (int) command["quantity"];
          Serial.println((int) command["quantity"]);
        }
      }
    }   
  } else if (topic == "new_request") {
    if (command.hasOwnProperty("products") && command.hasOwnProperty("quantities")) {
      int num_products = command["products"].length();
      Serial.print("Number of products: ");
      Serial.println(num_products);
      for (int i = 0; i < num_products; i++) {
        aux_product = (const char*) command["products"][i];
        if (product == aux_product) {
          requested_quantity = (int) command["quantities"][i];
          if (((int) command["quantities"][i]) <= quantity) {
            M5.dis.fillpix(0x00ff00); // Set color to green
          } else {
            M5.dis.fillpix(0x0000ff); // Set color to blue
          }
        }
      }
    }
  } else if (topic == "finished") {
    M5.dis.fillpix(0xff0000); // Set color back to red
  }   
}

int pressed_loop = 0;
void loop() {
  mqttClient.poll();

  unsigned long currentMillis = millis();

  if (currentMillis - previousButton > 5000 && pressed_loop) {
    if (button != 0) {
      productPickup["id"] = ID;
      productPickup["product"] = product;
      productPickup["quantity"] = button;

      String jsonString = JSON.stringify(productPickup);
      Serial.println(jsonString);

      mqttClient.beginMessage(product_pickup);
      mqttClient.print(productPickup);
      mqttClient.endMessage();

      button = 0;
      requested_quantity = 0;
      M5.dis.fillpix(0xff0000); // Set color back to red
    }

    pressed_loop = 0;
    previousButton = currentMillis;
  }

  M5.update();
  if (M5.Btn.wasPressed() && button < quantity && requested_quantity != 0 && button < requested_quantity) {
    if (pressed_loop) {
      if (currentMillis - previousButton <= 5000) {
        button++;
      }
    } else {
      pressed_loop = 1;
      button++;
    }
    previousButton = currentMillis;  
  }

  delay(500);

  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    previousMillis = currentMillis;

    if (!sgp.IAQmeasure()) {  // Commands the sensor to take a single eCO2/VOC measurement.
        Serial.println("Measurement failed");
        return;
    }

    if (! sgp.IAQmeasureRaw()) {
      Serial.println("Raw Measurement failed");
      return;
    }

    sensorRead["id"] = ID;
    sensorRead["tmp"] = 0;
    sensorRead["hum"] = 0;
    sensorRead["pressure"] = 0;
    sensorRead["tvoc"] = sgp.TVOC;
    sensorRead["eco2"] = sgp.eCO2;
    sensorRead["rawh2"] = sgp.rawH2;
    sensorRead["rawethanol"] = sgp.rawEthanol;

    String jsonString = JSON.stringify(sensorRead);
    Serial.println(jsonString);

    mqttClient.beginMessage(sensor_readings);
    mqttClient.print(sensorRead);
    mqttClient.endMessage();
  }
}