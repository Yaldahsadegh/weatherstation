#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <mongoose.h>

const char* ssid = "MB210-G";
const char* password = "studentMAMK";
const char* mqtt_server = "172.20.49.25";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

#define DHTPIN 12
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

struct WeatherData {
    double temperature;
    double humidity;
    time_t createdAt;
};

const char *mongoUri = "mongodb://localhost:27017";
static const char *s_http_port = "8000";
static struct mg_mgr mgr;
static struct mg_serve_http_opts s_http_server_opts;

void setup() {
  Serial.begin(115200);
  dht.begin();  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    
    // Temperature in Celsius
    float temperature = dht.readTemperature();   
    char tempString[8];
    dtostrf(temperature, 1, 2, tempString);
    client.publish("esp32/temperature", tempString);

    // Humidity
    float humidity = dht.readHumidity();
    char humString[8];
    dtostrf(humidity, 1, 2, humString);
    client.publish("esp32/humidity", humString);

    // Save data to MongoDB
    time_t now;
    time(&now);
    WeatherData data;
    data.temperature = temperature;
    data.humidity = humidity;
    data.createdAt = now;
    mgos_mongoose_save("weatherData", &data);
  }
}

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe("esp32/output");
    } else {
      delay(5000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  // handle MQTT messages
}
