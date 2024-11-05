#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <TFT_eSPI.h>
#include <HTTPClient.h>
#include <SD.h> // Biblioteka do obsługi karty SD

// Konfiguracja wyświetlacza
TFT_eSPI tft = TFT_eSPI();
#include "WiFiConfig.h"


// Lista URL stron
String urlList[5];
int urlCount = 0;

// Ustawienia wyświetlania stron
unsigned long displayInterval = 60000; // 1 minuta na stronę
unsigned long lastDisplayTime = 0;
int currentUrlIndex = 0;

// Serwer do konfiguracji przez przeglądarkę
AsyncWebServer server(80);

void setup() {
  // Uruchomienie wyświetlacza TFT
  tft.init();
  tft.setRotation(1);
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

  // Inicjalizacja karty SD
  if (!SD.begin()) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED);
    tft.println("Blad inicjalizacji karty SD!");
    while (true); // Zatrzymanie programu, jeśli nie ma karty SD
  }

  // Wczytanie listy URL z karty SD
  loadUrlsFromSD();

  // Konfiguracja serwera HTTP do zmiany konfiguracji
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><body><h2>Konfiguracja URL-i</h2>";
    html += "<form action=\"/save\" method=\"POST\">";
    html += "Adresy URL:<br>";
    for (int i = 0; i < 5; i++) {
      html += "URL " + String(i+1) + ": <input type=\"text\" name=\"url" + String(i) + "\" value=\"" + urlList[i] + "\"><br>";
    }
    html += "<br><input type=\"submit\" value=\"Zapisz\">";
    html += "</form></body></html>";
    request->send(200, "text/html", html);
  });
  
  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request){
    // Aktualizacja URL-i z formularza
    for (int i = 0; i < 5; i++) {
      String paramName = "url" + String(i);
      if (request->hasParam(paramName, true)) {
        urlList[i] = request->getParam(paramName, true)->value();
      }
    }
    urlCount = 5;
    
    // Zapisanie URL-i na kartę SD
    saveUrlsToSD();

    // Restart urządzenia
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

void loadUrlsFromSD() {
  File file = SD.open("/urls.txt");
  if (file) {
    int i = 0;
    while (file.available() && i < 5) {
      urlList[i] = file.readStringUntil('\n');
      urlList[i].trim(); // Usunięcie zbędnych spacji
      i++;
    }
    urlCount = i;
    file.close();
  } else {
    tft.setTextColor(TFT_RED);
    tft.println("Brak pliku z URL-ami na SD");
  }
}

void saveUrlsToSD() {
  File file = SD.open("/urls.txt", FILE_WRITE);
  if (file) {
    for (int i = 0; i < urlCount; i++) {
      file.println(urlList[i]);
    }
    file.close();
  }
}