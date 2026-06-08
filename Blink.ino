#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <ArduinoJson.h>

#define LED_BUILTIN 2

// ======================
// WIFI
// ======================
const char* ssid = "ACT_Sustainabyte";
const char* password = "Sustain@7788";

// ======================
// HIVEMQ
// ======================
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;

const char* otaTopic = "esp32/device001/ota";

// Current firmware version
String firmwareVersion = "1.1.6";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// ======================
// OTA FUNCTION
// ======================
void performOTA(String firmwareURL)
{
  Serial.println();
  Serial.println("========== OTA START ==========");
  Serial.print("URL: ");
  Serial.println(firmwareURL);

  Serial.print("Free Heap: ");
  Serial.println(ESP.getFreeHeap());

  Serial.print("Free Sketch Space: ");
  Serial.println(ESP.getFreeSketchSpace());

  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  WiFiClientSecure otaClient;
  otaClient.setInsecure();   // Accept Azure HTTPS certificate

  httpUpdate.rebootOnUpdate(true);

  t_httpUpdate_return result =
      httpUpdate.update(otaClient, firmwareURL);

  switch(result)
  {
    case HTTP_UPDATE_FAILED:
      Serial.printf(
        "OTA FAILED (%d): %s\n",
        httpUpdate.getLastError(),
        httpUpdate.getLastErrorString().c_str()
      );
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("NO UPDATE AVAILABLE");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("UPDATE SUCCESS");
      break;
  }

  Serial.println("========== OTA END ==========");
}

// ======================
// MQTT CALLBACK
// ======================
void mqttCallback(char* topic,
                  byte* payload,
                  unsigned int length)
{
  String message = "";

  for(unsigned int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }

  Serial.println();
  Serial.println("MQTT MESSAGE RECEIVED");
  Serial.println(message);

  DynamicJsonDocument doc(512);

  if(deserializeJson(doc, message))
  {
    Serial.println("JSON PARSE FAILED");
    return;
  }

  String newVersion =
      doc["version"].as<String>();

  String firmwareURL =
      doc["url"].as<String>();

  Serial.print("Current Version : ");
  Serial.println(firmwareVersion);

  Serial.print("New Version     : ");
  Serial.println(newVersion);

  if(newVersion != firmwareVersion)
  {
    Serial.println("NEW VERSION FOUND");
    performOTA(firmwareURL);
  }
  else
  {
    Serial.println("ALREADY LATEST VERSION");
  }
}

// ======================
// WIFI CONNECT
// ======================
void connectWiFi()
{
  Serial.print("Connecting WiFi");

  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
}

// ======================
// MQTT CONNECT
// ======================
void connectMQTT()
{
  while(!mqttClient.connected())
  {
    Serial.println("Connecting MQTT...");

    String clientId =
      "ESP32-" + String(random(0xffff), HEX);

    if(mqttClient.connect(clientId.c_str()))
    {
      Serial.println("MQTT Connected");

      mqttClient.subscribe(otaTopic);

      Serial.print("Subscribed: ");
      Serial.println(otaTopic);
    }
    else
    {
      Serial.print("MQTT Failed. State=");
      Serial.println(mqttClient.state());

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

  pinMode(LED_BUILTIN, OUTPUT);

  connectWiFi();

  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(mqttCallback);
}

// ======================
// LOOP
// ======================
void loop()
{
  if(!mqttClient.connected())
  {
    connectMQTT();
  }

  mqttClient.loop();

  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);
}