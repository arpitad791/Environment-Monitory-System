#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WebServer.h>

// WiFi Credentials
const char* ssid = "nahi dungi jaa";
const char* password = "realme02";

// DHT11 Sensor
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// MQ-6 Gas Sensor
#define MQ6_PIN 34 // GPIO for MQ-6 Analog Output
#define GAS_THRESHOLD 500

// Flame Sensor
#define FLAME_SENSOR_PIN 35 // GPIO for Flame Sensor Analog Output
#define FLAME_THRESHOLD 300 // Define a threshold for flame detection

// Buzzer & LED
#define BUZZER_PIN 32
#define LED_PIN 26

// Web Server
WebServer server(80);

// Function to calculate gas percentage
float calculateGasPercentage(int sensorValue) {
    return (sensorValue / 1023.0) * 100.0;
}

// Function to read sensor data
void handleRoot() {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    int gasValue = analogRead(MQ6_PIN);
    float gasPercentage = calculateGasPercentage(gasValue);
    int flameValue = analogRead(FLAME_SENSOR_PIN);
    bool flameDetected = flameValue < FLAME_THRESHOLD;

    if (isnan(temperature) || isnan(humidity)) {
        server.send(500, "text/plain", "❗ Failed to read from DHT sensor!");
        return;
    }

    // Activate buzzer and LED if gas level is high or flame is detected
    if (gasValue > GAS_THRESHOLD || flameDetected) {
        digitalWrite(BUZZER_PIN, HIGH);
        digitalWrite(LED_PIN, HIGH);
    } else {
        digitalWrite(BUZZER_PIN, LOW);
        digitalWrite(LED_PIN, LOW);
    }

    // Create HTML Response
    String html = "<html><head><meta charset='UTF-8'><title>ESP32 Sensor Dashboard</title>";
    html += "<style>body { font-family: Arial, sans-serif; text-align: center; background-color: #eaf6ff; color: #333; }";
    html += "h2 { color: #007bff; } .warning { color: red; font-size: 1.4em; }";
    html += ".normal { color: green; font-size: 1.2em; }";
    html += ".card { background-color: #fff; border-radius: 8px; box-shadow: 0px 4px 8px rgba(0,0,0,0.1); margin: 20px auto; padding: 20px; width: 90%; max-width: 400px; }";
    html += "</style></head><body>";

    html += "<div class='card'>";
    html += "<h2> SenseNet </h2>";
    html += "<p>🌞 <strong>Temperature:</strong> " + String(temperature) + " °C</p>";
    html += "<p>💧 <strong>Humidity:</strong> " + String(humidity) + " %</p>";
    html += "<p>🛢 <strong>Gas Sensor Value:</strong> " + String(gasValue) + "</p>";
    html += "<p>📈 <strong>Gas Percentage:</strong> " + String(gasPercentage, 2) + " %</p>";
    html += "<p>🔥 <strong>Flame Sensor Value:</strong> " + String(flameValue) + "</p>";
    
    if (gasValue > GAS_THRESHOLD) {  
        html += "<h3 class='warning'>⚠ High Gas Level Detected! 🚨</h3>";
    }
    
    if (flameDetected) {
        html += "<h3 class='warning'>🔥 Flame Detected! 🚨</h3>";
    }

    if (gasValue <= GAS_THRESHOLD && !flameDetected) {
        html += "<h3 class='normal'>✅ All Readings Normal</h3>";
    }

    html += "</div></body></html>";
    server.send(200, "text/html", html);
}

void setup() {
    Serial.begin(115200);
    dht.begin();
    
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(FLAME_SENSOR_PIN, INPUT);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);

    // WiFi Connection
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    
    Serial.println("\nWiFi Connected!");
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());

    // Start Web Server
    server.on("/", handleRoot);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
}