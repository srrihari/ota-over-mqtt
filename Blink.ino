#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <HTTPUpdate.h>
#include <ArduinoJson.h>

#define STATUS_LED 2

// ======================
// DEVICE CONFIG
// ======================
const char* DEVICE_ID = "device001";
String firmwareVersion = "1.0.0";

// ======================
// WIFI CONFIG
// ======================
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ======================
// MQTT CONFIG
// ======================
const char* MQTT_SERVER = "broker.hivemq.com";
const int MQTT_PORT = 1883;

// Topic format: esp32/<device_id>/ota
String otaTopic = "esp32/" + String(DEVICE_ID) + "/ota";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// ======================
// OTA UPDATE
// ======================
void performOTA(String firmwareURL)
{
  if (firmwareURL.length() == 0)
  {
    Serial.println("OTA URL is empty");
    return;
  }

  Serial.println("\n========== OTA START ==========");
  Serial.println("Device ID: " + String(DEVICE_ID));
  Serial.println("Current Version: " + firmwareVersion);
  Serial.println("Firmware URL: " + firmwareURL);
  Serial.println("Free Heap: " + String(ESP.getFreeHeap()));
  Serial.println("Free Sketch Space: " + String(ESP.getFreeSketchSpace()));
  Serial.println("WiFi RSSI: " + String(WiFi.RSSI()));

  WiFiClientSecure otaClient;
  otaClient.setInsecure();

  httpUpdate.rebootOnUpdate(true);

  t_httpUpdate_return result = httpUpdate.update(otaClient, firmwareURL);

  if (result == HTTP_UPDATE_FAILED)
  {
    Serial.printf(
      "OTA FAILED (%d): %s\n",
      httpUpdate.getLastError(),
      httpUpdate.getLastErrorString().c_str()
    );
  }
  else if (result == HTTP_UPDATE_NO_UPDATES)
  {
    Serial.println("NO UPDATE AVAILABLE");
  }
  else if (result == HTTP_UPDATE_OK)
  {
    Serial.println("OTA UPDATE SUCCESS");
  }

  Serial.println("========== OTA END ==========");
}

// ======================
// MQTT CALLBACK
// ======================
void mqttCallback(char* topic, byte* payload, unsigned int length)
{
  String message;

  for (unsigned int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }

  Serial.println("\nMQTT MESSAGE RECEIVED");
  Serial.println("Topic: " + String(topic));
  Serial.println("Payload: " + message);

  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, message);

  if (error)
  {
    Serial.println("JSON PARSE FAILED");
    Serial.println(error.c_str());
    return;
  }

  String newVersion = doc["version"] | "";
  String firmwareURL = doc["url"] | "";

  if (newVersion.length() == 0 || firmwareURL.length() == 0)
  {
    Serial.println("Invalid OTA message. Required: version and url");
    return;
  }

  Serial.println("Current Version: " + firmwareVersion);
  Serial.println("New Version: " + newVersion);

  if (newVersion != firmwareVersion)
  {
    Serial.println("NEW VERSION FOUND");
    performOTA(firmwareURL);
  }
  else
  {
    Serial.println("ALREADY RUNNING LATEST VERSION");
  }
}

// ======================
// WIFI CONNECT
// ======================
void connectWiFi()
{
  Serial.print("Connecting to WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.println("IP Address: " + WiFi.localIP().toString());
  Serial.println("RSSI: " + String(WiFi.RSSI()));
}

// ======================
// MQTT CONNECT
// ======================
void connectMQTT()
{
  while (!mqttClient.connected())
  {
    Serial.println("Connecting to MQTT...");

    String clientId = "ESP32-" + String(DEVICE_ID) + "-" + String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str()))
    {
      Serial.println("MQTT Connected");

      mqttClient.subscribe(otaTopic.c_str());

      Serial.println("Subscribed Topic: " + otaTopic);
    }
    else
    {
      Serial.println("MQTT Failed. State: " + String(mqttClient.state()));
      delay(5000);
    }
  }
}

// ======================
// SETUP
// ======================
void setup()
{
  Serial.begin(115200);
  delay(1000);

  pinMode(STATUS_LED, OUTPUT);

  Serial.println("\nESP32 OTA Firmware Updater");
  Serial.println("Device ID: " + String(DEVICE_ID));
  Serial.println("Firmware Version: " + firmwareVersion);

  connectWiFi();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
}

// ======================
// LOOP
// ======================
void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    connectWiFi();
  }

  if (!mqttClient.connected())
  {
    connectMQTT();
  }

  mqttClient.loop();

  digitalWrite(STATUS_LED, HIGH);
  delay(200);
  digitalWrite(STATUS_LED, LOW);
  delay(200);
}
