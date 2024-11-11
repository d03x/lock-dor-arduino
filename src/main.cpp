#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
int wifiStatus;
const char *ssid = "Ruzman";
const char *password = "12345678";

#define RST_PIN D1
#define SDA_PIN D2
#define RELAY_LOCKDOOR D3
#define BUZER_JOKOWI D8
MFRC522 mfrc522(SDA_PIN, RST_PIN);

void soundCardValid()
{
  digitalWrite(BUZER_JOKOWI, HIGH);
  delay(100);
  digitalWrite(BUZER_JOKOWI, LOW);
  delay(100);
  digitalWrite(BUZER_JOKOWI, HIGH);
  delay(100);
  digitalWrite(BUZER_JOKOWI, LOW);
}

void setup()
{
  Serial.begin(9600);
  // set pin mode
  pinMode(BUZER_JOKOWI, OUTPUT);
  pinMode(RELAY_LOCKDOOR, OUTPUT);
  digitalWrite(RELAY_LOCKDOOR, HIGH); // Relay off by default
  digitalWrite(BUZER_JOKOWI,LOW);
  // start spi
  SPI.begin();
  mfrc522.PCD_Init();
  // connect to wifi
  WiFi.begin(ssid, password);
  //koneksi kan ke jaringan
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  //cek wifi koneksi nya apakah sudah konek apa belum
  if ( WiFi.status() == WL_CONNECTED ) {
    Serial.println("Koneksi Dengan Wifi: ");
    Serial.print(WiFi.localIP());
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

    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
    {
      String cardUID = readData();
      Serial.print("Detected UID: ");
      Serial.println(cardUID);

      if (cardUID == " E3 6A 46 FE" || cardUID == " 05 8F 3B 69 B3 A2 00")
      {
        soundCardValid();
        Serial.println("STMIKKK");
        digitalWrite(RELAY_LOCKDOOR, LOW);  // Aktifkan relay (coba HIGH jika ini tidak aktif)
        delay(2000);                        // Relay aktif selama 2 detik
        digitalWrite(RELAY_LOCKDOOR, HIGH); // Matikan relay
        Serial.println("Relay Deactivated");
      }
      else
      {
        digitalWrite(BUZER_JOKOWI, HIGH);
        delay(1000);
        digitalWrite(BUZER_JOKOWI, LOW);
        Serial.println("Access Denied");
        digitalWrite(RELAY_LOCKDOOR, HIGH); // Pastikan relay mati jika kartu tidak cocok
      }
    }
  } else {
    Serial.print("Wifi tidak konek bos");
  }
}
