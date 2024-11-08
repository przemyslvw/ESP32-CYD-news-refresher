#include <WiFi.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h>     // Biblioteka dla wyświetlacza TFT
#include "WiFiConfig.h"   // Konfiguracja WiFi

// Adres URL kanału RSS
const char* rssUrl = "http://rss.cnn.com/rss/edition.rss";

// Inicjalizacja wyświetlacza TFT
TFT_eSPI tft = TFT_eSPI();
int newsIndex = 0;
unsigned long lastRefreshTime = 0;
const unsigned long refreshInterval = 60000;  // Odświeżenie co minutę

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.println("Laczenie z WiFi...");

  // Połącz z WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nPolaczono z WiFi!");
  tft.println("Polaczono z WiFi");
}

void loop() {
  // Sprawdź, czy upłynął czas odświeżenia (1 minuta)
  if (WiFi.status() == WL_CONNECTED && millis() - lastRefreshTime >= refreshInterval) {
    lastRefreshTime = millis();  // Zapisz czas odświeżenia
    fetchRSS();
  }
}

// Funkcja do pobrania i wyświetlenia kanału RSS
void fetchRSS() {
  HTTPClient http;
  http.begin(rssUrl);  // Podaj URL kanału RSS

  int httpCode = http.GET();
  if (httpCode > 0) {  // Sprawdzenie odpowiedzi HTTP
    String payload = http.getString();

    // Wyświetl zawartość na serial monitorze
    Serial.println(payload);

    // Przetwarzanie i wyświetlanie tytułów na ekranie
    displayRSS(payload);
  } else {
    Serial.print("Blad HTTP: ");
    Serial.println(httpCode);
    tft.println("Blad ladowania");
  }
  http.end();
}

// Funkcja przetwarzająca i wyświetlająca tytuły z kanału RSS
void displayRSS(const String &rssData) {
  // Szukamy i wyświetlamy tytuły z XML (prosty parser)
  String tagStart = "<title>";
  String tagEnd = "</title>";

  int startPos = 0;
  newsIndex = 0;
  tft.fillScreen(TFT_BLACK);

  while ((startPos = rssData.indexOf(tagStart, startPos)) != -1) {
    startPos += tagStart.length();
    int endPos = rssData.indexOf(tagEnd, startPos);

    if (endPos != -1) {
      String title = rssData.substring(startPos, endPos);
      Serial.println(title);  // Wydruk na Serial Monitor

      // Wyświetlenie tytułu na ekranie TFT
      tft.setCursor(0, newsIndex * 20);  // Pozycja w pionie
      tft.println(title);
      newsIndex++;
      delay(500);  // Krótkie opóźnienie dla lepszej czytelności

      if (newsIndex >= 10) break;  // Przerwij po 10 tytułach
    }
  }
}