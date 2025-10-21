#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <HTTPClient.h>

// --- DHT22 ---
#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// --- DS18B20 ---
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

// --- LCD I2C ---
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- WiFi & NTP ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";
#define NTP_SERVER     "pool.ntp.org"
#define UTC_OFFSET     7 * 3600  // WIB (UTC+7)
#define UTC_OFFSET_DST 0

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    lcd.print(".");
  }

  lcd.clear();
  lcd.print("WiFi Connected");
  Serial.println("WiFi Connected");

  delay(2000);
  lcd.clear();
  lcd.print("Halo Fakhrul");
  delay(2000);
  lcd.clear();
  lcd.print("Halo Haidar");

  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
  dht.begin();
  ds18b20.begin();
  delay(2000);
  lcd.clear();
}

void loop() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    lcd.setCursor(0, 0);
    lcd.print("Waktu Error     ");
    lcd.setCursor(0, 1);
    lcd.print("Cek NTP Server  ");
    delay(2000);
    return;
  }

  // Baca DHT22
  float suhuLuar = dht.readTemperature();
  float kelembaban = dht.readHumidity();

  // Baca DS18B20
  ds18b20.requestTemperatures();
  float suhuDalam = ds18b20.getTempCByIndex(0);

  // Validasi
  if (isnan(suhuLuar) || isnan(kelembaban) || suhuDalam == DEVICE_DISCONNECTED_C) {
    Serial.println("Sensor gagal baca");
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error     ");
    lcd.setCursor(0, 1);
    lcd.print("Retrying...");
    delay(2000);
    return;
  }

  // Format waktu
  char waktuStr[20];
  strftime(waktuStr, sizeof(waktuStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
  String waktuEncoded = String(waktuStr);
  waktuEncoded.replace(" ", "%20");

  // Serial log
  Serial.printf("%s | S.luar: %.1f°C | Hum: %.1f%% | S.dalam: %.1f°C\n",
                waktuStr, suhuLuar, kelembaban, suhuDalam);

  // LCD
  lcd.setCursor(0, 0);
  lcd.printf("T:%.1fC H:%.1f%", suhuLuar, kelembaban);
  delay(10000);
  lcd.setCursor(0, 0);
  lcd.printf("A:%.1fC", suhuDalam);
  lcd.setCursor(0, 1);
  lcd.print(&timeinfo, "%H:%M                  ");
  delay(5000);
  lcd.setCursor(0, 1);
  lcd.print(&timeinfo, "%d/%m/%Y");
  delay(5000);

}