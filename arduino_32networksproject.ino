#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define TDS_PIN 34  // ADC pin for TDS sensor

// WiFi credentials
const char* ssid = "Seddik1";
const char* password = "seddik00";

// MQTT Broker info
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_topic = "/water_quality/tds123";

// Set your LCD I2C address here (0x27 or 0x3F etc.)
LiquidCrystal_I2C lcd(0x27, 16, 2);

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println(" connected!");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32TDSClient")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();            // Initialize I2C
  lcd.begin(16, 2);        // Initialize LCD (16 cols, 2 rows)
  lcd.backlight();         // Turn on LCD backlight
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Water Quality");

  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int adcValue = analogRead(TDS_PIN);
  float voltage = adcValue * (3.3 / 4095.0);
  float tdsValue = (133.42 * voltage * voltage * voltage
                   - 255.86 * voltage * voltage
                   + 857.39 * voltage) * 0.5;

  Serial.print("ADC: "); Serial.print(adcValue);
  Serial.print(" | Voltage: "); Serial.print(voltage, 2);
  Serial.print(" V | TDS: "); Serial.print(tdsValue, 2); Serial.println(" ppm");

  lcd.setCursor(0, 1);
  if (tdsValue < 250) {
    lcd.print("Safe  TDS: ");
  } else {
    lcd.print("UnsafeTDS:");
  }
  lcd.setCursor(11, 1);
  lcd.print("     "); // Clear previous number
  lcd.setCursor(11, 1);
  lcd.print((int)tdsValue);

  // Publish TDS value to MQTT topic
  char tdsStr[10];
  dtostrf(tdsValue, 4, 1, tdsStr);
  client.publish(mqtt_topic, tdsStr);

  delay(2000);
}
