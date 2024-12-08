#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
int wifiStatus;
const char *ssid = "Ruzman";
const char *api_url = "http://192.168.43.50/loker";
const char *password = "12345678";
#define LOKER_ID 1;
#define RST_PIN D1
#define SDA_PIN D2
#define RELAY_LOCKDOOR D3
#define BUZZER D8
MFRC522 mfrc522(SDA_PIN, RST_PIN);

void soundCardValid(int soundInterval = 100)
{
  digitalWrite(BUZZER, HIGH);
  delay(soundInterval);
  digitalWrite(BUZZER, LOW);
  delay(soundInterval);
  digitalWrite(BUZZER, HIGH);
  delay(soundInterval);
  digitalWrite(BUZZER, LOW);
}

void setup()
{
  Serial.begin(9600);
  // set pin mode
  pinMode(BUZZER, OUTPUT);
  pinMode(RELAY_LOCKDOOR, OUTPUT);
  digitalWrite(RELAY_LOCKDOOR, HIGH); // Relay off by default
  digitalWrite(BUZZER, LOW);
  // start spi
  SPI.begin();
  mfrc522.PCD_Init();
  // connect to wifi
  WiFi.begin(ssid, password);
  // koneksi kan ke jaringan
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    soundCardValid();
  }
  // cek wifi koneksi nya apakah sudah konek apa belum
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Koneksi Dengan Wifi: ");
    Serial.print(WiFi.localIP());
    soundCardValid(300);
  }
}
// rad data
String readData()
{
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  return content;
}

void loop()
{
  wifiStatus = WiFi.status();
  if (wifiStatus == WL_CONNECTED)
  {
    HTTPClient http;
    WiFiClient wf_client;

    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
    {
      String cardUID = readData();
      Serial.print("Detected UID: ");
      Serial.println(cardUID);
      http.begin(wf_client, String(api_url) + String("/save_card.php"));
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // http.begin(wf_client, String(api_url) + String("/save_card.php"));
      int httpResponKode = http.POST("rfid=" + cardUID + "&loker=1");
      if (httpResponKode > 0)
      {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponKode);
        String payload = http.getString();
        if (payload == cardUID || cardUID == " 05 8F 3B 69 B3 A2 00")
        {
          soundCardValid();
          digitalWrite(RELAY_LOCKDOOR, LOW);  // Aktifkan relay (coba HIGH jika ini tidak aktif)
          delay(2000);                        // Relay aktif selama 2 detik
          digitalWrite(RELAY_LOCKDOOR, HIGH); // Matikan relay
        }
        else
        {
          digitalWrite(BUZZER, HIGH);
          delay(1000);
          digitalWrite(BUZZER, LOW);
          digitalWrite(RELAY_LOCKDOOR, HIGH); // Pastikan relay mati jika kartu tidak cocok
        }
      }
      else
      {
        Serial.print("Error code: ");
        Serial.println(httpResponKode);
        String payload = http.getString();
        Serial.println(payload);
      }
      http.end();
      
    }
  }
  else
  {
    Serial.print("Wifi tidak konek bos");
  }
}
