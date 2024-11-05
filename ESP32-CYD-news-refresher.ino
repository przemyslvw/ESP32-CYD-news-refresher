#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <TFT_eSPI.h>
#include <HTTPClient.h>
#include "WiFiConfig.h"

// Konfiguracja wyświetlacza
TFT_eSPI tft = TFT_eSPI();


// Lista URL stron do wyświetlenia (wprowadzana przez użytkownika)
String urlList[5] = {"http://example1.com", "http://example2.com", "http://example3.com"};
int urlCount = 3; // Liczba URL-i w urlList

// Ustawienia dla wyświetlania stron
unsigned long displayInterval = 60000; // 1 minuta na stronę
unsigned long lastDisplayTime = 0;
int currentUrlIndex = 0;

// Serwer do konfiguracji przez przeglądarkę (np. http://192.168.x.x)
AsyncWebServer server(80);

void setup() {
  // Uruchomienie wyświetlacza TFT
  tft.init();
  tft.setRotation(1); // Ustawienie orientacji ekranu
  tft.fillScreen(TFT_BLACK);

  // Inicjalizacja Wi-Fi
  WiFi.begin(ssid.c_str(), password.c_str());
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Laczenie z Wi-Fi...", tft.width() / 2, tft.height() / 2);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  tft.fillScreen(TFT_BLACK);
  tft.drawString("Polaczono z Wi-Fi", tft.width() / 2, tft.height() / 2);
  delay(1000);

  // Konfiguracja interfejsu konfiguracyjnego (adres IP modułu)
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><body><h2>Konfiguracja Wi-Fi i URL-i</h2>";
    html += "<form action=\"/save\" method=\"POST\">";
    html += "SSID Wi-Fi: <input type=\"text\" name=\"ssid\"><br>";
    html += "Haslo Wi-Fi: <input type=\"password\" name=\"password\"><br><br>";
    html += "Adresy URL:<br>";
    for (int i = 0; i < 5; i++) {
      html += "URL " + String(i+1) + ": <input type=\"text\" name=\"url" + String(i) + "\"><br>";
    }
    html += "<br><input type=\"submit\" value=\"Zapisz\">";
    html += "</form></body></html>";
    request->send(200, "text/html", html);
  });
  
  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request){
    // Odczytanie parametrów konfiguracyjnych
    if (request->hasParam("ssid", true)) ssid = request->getParam("ssid", true)->value();
    if (request->hasParam("password", true)) password = request->getParam("password", true)->value();

    for (int i = 0; i < 5; i++) {
      String paramName = "url" + String(i);
      if (request->hasParam(paramName, true)) {
        urlList[i] = request->getParam(paramName, true)->value();
      }
    }
    urlCount = 5;

    // Restart urządzenia, aby zapisać nowe ustawienia
    request->send(200, "text/html", "<html><body><h2>Ustawienia zapisane. Restart...</h2></body></html>");
    delay(2000);
    ESP.restart();
  });
  
  server.begin();
}

void loop() {
  // Sprawdzanie czasu do zmiany strony
  if (millis() - lastDisplayTime > displayInterval) {
    lastDisplayTime = millis();
    displayWebPage(urlList[currentUrlIndex]);
    
    // Przejście do następnej strony w kolejce
    currentUrlIndex = (currentUrlIndex + 1) % urlCount;
  }
}

void displayWebPage(String url) {
  HTTPClient http;
  http.begin(url);
  
  int httpCode = http.GET();
  if (httpCode > 0) {
    String payload = http.getString();
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(0, 0);
    tft.println("Strona: " + url);
    tft.println("Kod odpowiedzi: " + String(httpCode));
    
    tft.println("Zawartosc strony:");
    tft.println(payload.substring(0, 500)); // Wyświetl część strony (500 znaków)
  } else {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(TFT_RED);
    tft.println("Blad polaczenia!");
  }
  
  http.end();
}
