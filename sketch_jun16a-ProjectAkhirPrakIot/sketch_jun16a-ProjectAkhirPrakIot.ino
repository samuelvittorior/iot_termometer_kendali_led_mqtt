#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Definisikan konstanta untuk konfigurasi WiFi dan MQTT
const char* ssid = "MyWi-Fi"; // Ganti dengan SSID WiFi Anda
const char* password = "mywfpw99"; // Ganti dengan password WiFi Anda
const char* mqtt_server = "usm.revolusi-it.com"; // Ganti dengan nama server MQTT
const int mqtt_port = 1883; // Port default untuk MQTT
const char* mqtt_user = "usm"; // Ganti dengan username MQTT
const char* mqtt_password = "usmjaya24"; // Ganti dengan password MQTT
const char* topic = "test/test"; // Topik MQTT untuk pesan

// Inisialisasi objek WiFiClient dan PubSubClient
WiFiClient espClient;
PubSubClient client(espClient);

// Definisi pin dan tipe sensor DHT
#define DHTPIN D5
#define DHTTYPE DHT11

// Inisialisasi objek DHT dan LCD
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin untuk LED
#define LED1 D6
#define LED2 D7
#define LED3 D8

void setup() {
  Wire.begin(D1, D2); // Inisialisasi I2C
  lcd.begin(16, 2); // Inisialisasi LCD dengan ukuran 16x2
  lcd.backlight(); // Mengaktifkan backlight LCD
  dht.begin(); // Menginisialisasi sensor DHT
  Serial.begin(115200);

  // Inisialisasi pin LED sebagai output
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  // Mencoba koneksi ke WiFi dengan SSID dan password yang ditentukan
  WiFi.begin(ssid, password);
  // Menunggu koneksi WiFi berhasil
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Menampilkan informasi tentang koneksi WiFi yang berhasil
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); // Menampilkan alamat IP Lokal NodeMCU

  // Mengatur server MQTT dengan nama server dan port yang ditentukan
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    konek_wifi();
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float h = dht.readHumidity(); // Membaca kelembaban
  float t = dht.readTemperature(); // Membaca suhu
  //Serial.println("Suhu: " + String(t) + "C" + " Kelembaban: " + String(h));

  // Periksa apakah pembacaan berhasil
  if (isnan(h) || isnan(t)) {
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error   ");
    lcd.setCursor(0, 1);
    lcd.print("                "); // Membersihkan baris kedua
    delay(1000);
    return;
  }

  // Menampilkan suhu pada baris pertama
  lcd.setCursor(0, 0);
  lcd.print("Suhu : " + String(t) + "C   ");

  // Menampilkan kelembaban pada baris kedua
  lcd.setCursor(0, 1);
  lcd.print("Kelembaban: " + String(h) + "%   ");

  delay(1500); // Delay untuk menghindari pembaruan terlalu cepat

  // Membuat pesan MQTT yang berisi nama, suhu, dan kelembaban
  String message = "{\"nim\":\"G.231.21.0187\",\"suhu\":" + String(t) + ",\"kelembaban\":" + String(h) + "}";
  client.publish(topic, message.c_str());
  Serial.println("MQTT message published: " + message); // Menampilkan pesan "MQTT message published" dan isi pesan

  // Menentukan interval pengiriman pesan (2 detik)
  delay(2000);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Pesan dari MQTT [");
  Serial.print(topic);
  Serial.print("] ");

  StaticJsonDocument<256> doc; // Membuat dokumen JSON dengan kapasitas yang cukup
  DeserializationError error = deserializeJson(doc, payload, length); // Mengurai JSON

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  const char* nim = doc["nim"]; // Mengambil nilai NIM
  const char* messages = doc["messages"]; // Mengambil nilai messages

  if (String(nim) == "G.231.21.0187" || String(nim) == "all" || String(nim) == "ALL") { // Sesuaikan dengan nim Anda
    Serial.println(messages);

    if (String(messages) == "LED1=1" || String(messages)=="ALL=1" || String(messages) == "all=1") { digitalWrite(LED1, HIGH); }
    if (String(messages) == "LED2=1" || String(messages)=="ALL=1" || String(messages) == "all=1") { digitalWrite(LED2, HIGH); }
    if (String(messages) == "LED3=1" || String(messages)=="ALL=1" || String(messages) == "all=1") { digitalWrite(LED3, HIGH); }
    if (String(messages) == "LED1=0" || String(messages)=="ALL=0" || String(messages) == "all=0") { digitalWrite(LED1, LOW); }
    if (String(messages) == "LED2=0" || String(messages)=="ALL=0" || String(messages) == "all=0") { digitalWrite(LED2, LOW); }
    if (String(messages) == "LED3=0" || String(messages)=="ALL=0" || String(messages) == "all=0") { digitalWrite(LED3, LOW); }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Menghubungkan diri ke MQTT Server: ");
    Serial.println(mqtt_server);
    // Attempt to connect
    if (client.connect("G.231.21.0187", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // ... and subscribe to topics
      client.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi dalam 5 detik... ");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void konek_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}
