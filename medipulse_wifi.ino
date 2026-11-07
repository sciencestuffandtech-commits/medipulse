// MediPulse ESP32-WROOM-32 Wi-Fi firmware skeleton.
// Serves the HTTP API that the MediPulse app expects. It never invents readings:
// until readSensors() is connected to your real heart-rate / SpO2 sensor code,
// a started checkup finishes with status "failed" and reason "sensor_not_implemented".

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
const char* FIRMWARE = "0.1.0";

WebServer server(80);
String deviceId;

enum State { IDLE, MEASURING, COMPLETED, FAILED };
State state = IDLE;
unsigned long startedAt = 0;
const unsigned long MEASURE_TIMEOUT_MS = 60000;

int lastBpm = 0;
float lastSpo2 = 0;
String lastQuality = "good";
String lastReason = "";

// TODO: connect your sensor here (for example MAX30102 with the SparkFun MAX3010x library).
// Return true once a complete, valid measurement is ready and fill the three outputs.
// quality must be "good", "fair" or "poor".
bool readSensors(int &bpm, float &spo2, String &quality) {
    return false;
}

void cors() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.sendHeader("Access-Control-Allow-Private-Network", "true");
}

const char* stateName() {
    switch (state) {
        case MEASURING: return "measuring";
        case COMPLETED: return "completed";
        case FAILED: return "failed";
        default: return "idle";
    }
}

void handleStatus() {
    cors();
    String j = "{\"device\":\"medipulse\",\"device_id\":\"" + deviceId + "\",\"firmware\":\"" + FIRMWARE + "\",\"state\":\"" + stateName() + "\"}";
    server.send(200, "application/json", j);
}

void handleStart() {
    cors();
    if (state == MEASURING) {
        server.send(409, "application/json", "{\"error\":\"busy\"}");
        return;
    }
    state = MEASURING;
    startedAt = millis();
    lastReason = "";
    server.send(200, "application/json", "{\"ok\":true}");
}

void handleCancel() {
    cors();
    state = IDLE;
    server.send(200, "application/json", "{\"ok\":true}");
}

void handleCheckup() {
    cors();
    String j = "{\"status\":\"" + String(stateName()) + "\",\"device_id\":\"" + deviceId + "\"";
    if (state == COMPLETED) {
        j += ",\"bpm\":" + String(lastBpm) + ",\"spo2\":" + String(lastSpo2, 1) + ",\"quality\":\"" + lastQuality + "\"";
    }
    if (state == FAILED) {
        j += ",\"reason\":\"" + lastReason + "\"";
    }
    j += "}";
    server.send(200, "application/json", j);
}

void handleOptions() {
    cors();
    server.send(204);
}

void setup() {
    Serial.begin(115200);
    uint64_t mac = ESP.getEfuseMac();
    char buf[16];
    snprintf(buf, sizeof(buf), "MP-%04X", (uint16_t)(mac >> 32));
    deviceId = buf;

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(400);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("MediPulse IP: ");
    Serial.println(WiFi.localIP());
}