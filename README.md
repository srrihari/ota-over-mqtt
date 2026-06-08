# ESP32 OTA Firmware Updater

A generic ESP32 Over-The-Air (OTA) firmware update system using MQTT and HTTPS.

The device listens for OTA commands through MQTT, downloads the latest firmware from a remote server, and updates itself automatically without requiring a USB connection.

## Features

* WiFi connectivity
* MQTT-based OTA trigger
* HTTPS firmware download
* Automatic firmware installation
* Version-based update mechanism
* Remote device management
* Serial debugging and monitoring

---

## Architecture

```text
Developer
    │
    ▼
Build firmware.bin
    │
    ▼
Upload to Cloud Storage
(Azure Blob / AWS S3 / Any HTTPS Server)
    │
    ▼
Publish OTA JSON via MQTT
    │
    ▼
ESP32 receives update command
    │
    ▼
Downloads firmware.bin
    │
    ▼
Installs update
    │
    ▼
Reboots automatically
```

---

## OTA Message Format

Publish a JSON message to the device OTA topic.

```json
{
  "version": "1.0.1",
  "url": "https://your-storage-url.com/firmware.bin"
}
```

### Example

```json
{
  "version": "1.1.7",
  "url": "https://example.blob.core.windows.net/firmware/firmware.bin"
}
```

---

## MQTT Topic Structure

```text
esp32/<device-id>/ota
```

### Example

```text
esp32/device001/ota
```

---

## Required Libraries

Install the following libraries from Arduino IDE Library Manager:

* WiFi
* WiFiClientSecure
* PubSubClient
* HTTPClient
* HTTPUpdate
* ArduinoJson

---

## Firmware Upload Steps

### 1. Update Firmware Code

Modify your ESP32 application as required.

### 2. Build Firmware

In Arduino IDE:

```text
Sketch → Export Compiled Binary
```

This generates:

```text
firmware.bin
```

### 3. Upload Binary

Upload the generated firmware file to:

* Azure Blob Storage
* AWS S3
* GitHub Releases
* Any HTTPS-accessible server

### 4. Copy Firmware URL

Example:

```text
https://example.blob.core.windows.net/firmware/firmware.bin
```

### 5. Publish OTA Command

Publish the JSON OTA message to the device topic.

### 6. Device Updates Automatically

The ESP32:

* Receives the MQTT message
* Downloads firmware
* Installs update
* Reboots automatically

---

## Initial Setup

### Configure WiFi

```cpp
const char* WIFI_SSID = "YOUR_WIFI";
const char* WIFI_PASSWORD = "YOUR_PASSWORD";
```

### Configure Device ID

```cpp
const char* DEVICE_ID = "device001";
```

### Configure Firmware Version

```cpp
String firmwareVersion = "1.0.0";
```

---

## Running the Project

### First Upload

The initial firmware must be flashed using USB.

Steps:

1. Connect ESP32 via USB
2. Select board in Arduino IDE
3. Select COM Port
4. Upload firmware
5. Open Serial Monitor

After the first installation, future updates can be performed remotely using OTA.

---

## Serial Output Example

```text
WiFi Connected
MQTT Connected
Subscribed: esp32/device001/ota

MQTT MESSAGE RECEIVED

Current Version : 1.0.0
New Version     : 1.0.1

NEW VERSION FOUND

========== OTA START ==========
Downloading firmware...
OTA UPDATE SUCCESS
```

---

## Notes

* First firmware upload requires USB.
* All future updates can be performed OTA.
* Firmware URL must be publicly accessible.
* Device must have internet connectivity.
* MQTT broker must be reachable.
* HTTPS is recommended for production deployments.

---

## Future Improvements

* Secure MQTT authentication
* Device authentication and authorization
* OTA progress reporting
* Web dashboard
* Device fleet management
* Rollback support
* Digital signature verification
* Version management service
* Update scheduling

---

## License

MIT License

---

## Author

**Srri Hari T R**
