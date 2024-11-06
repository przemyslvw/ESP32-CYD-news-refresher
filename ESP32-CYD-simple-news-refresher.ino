#include <WiFi.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h>   // Biblioteka wyświetlacza TFT
#include "WiFiConfig.h" // Plik konfiguracyjny Wi-Fi

// URL strony do wyświetlenia
const char* url = "http://example.com";

// Inicjalizacja wyświetlacza TFT
TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);

  tft.println("Laczenie z WiFi...");
  Serial.print("Laczenie z WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Sprawdzenie adresu MAC punktu dostępowego po połączeniu
  uint8_t bssid[6];
  WiFi.BSSID(bssid);  // Pobiera MAC adres punktu dostępowego

  bool authorizedAP = true;
  for (int i = 0; i < 6; i++) {
    if (bssid[i] != apMAC[i]) {
      authorizedAP = false;
      break;
    }
  }

  if (authorizedAP) {
    Serial.println("Polaczono z autoryzowanym AP!");
    tft.fillScreen(TFT_BLACK);
    tft.println("Polaczono z autoryzowanym AP");
  } else {
    Serial.println("Nieautoryzowany AP, przerywam polaczenie!");
    tft.fillScreen(TFT_RED);
    tft.println("Nieautoryzowany AP");
    WiFi.disconnect();  // Rozłącza się, jeśli punkt dostępowy jest nieautoryzowany
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(url);

    int httpCode = http.GET();
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0);
      for (int i = 0; i < payload.length(); i += 30) {
        String line = payload.substring(i, i + 30);
        tft.println(line);
        delay(100);
      }
    } else {
      Serial.print("Blad HTTP: ");
      Serial.println(httpCode);
      tft.println("Blad ladowania strony");
    }
    http.end();
  } else {
    Serial.println("Brak polaczenia z WiFi");
    tft.println("Brak polaczenia z WiFi");
  }
  delay(60000);
}