#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define DHTPIN 15
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "IoTB";
const char* password = "inventaronelVAR";

const char* apiKey = "TU_API_KEY";
const char* city = "Mexico City";
const char* apiURL = "http://api.openweathermap.org/data/2.5/weather?q=MEXICO_CITY&appid=TU_API_KEY&units=metric";

WebServer server(80);

float apiTemperature = 0.0;
float apiHumidity = 0.0;

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado a WiFi. Dirección IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();
  Serial.println("Servidor HTTP iniciado");

  getWeatherData();
}

void loop() {
  server.handleClient();

  if (millis() % 60000 == 0) //? Pregunta a la API cada minuto
  {
    getWeatherData();
  }
}

void handleRoot() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    server.send(500, "text/plain", "Error al leer el sensor DHT");
    return;
  }

  String html = "<!DOCTYPE html><html>";
  html += "<head><meta http-equiv='refresh' content='10'/>";
  html += "<title>ESP32 DHT Server</title>";
  html += "</head>";

  html += "<body>";
  html += "<h1>Valores de Temperatura y Humedad</h1>";
  html += "<p><strong>Datos del sensor DHT:</strong></p>";
  html += "<p>Temperatura: <strong>" + String(t) + " °C</strong></p>";
  html += "<p>Humedad: <strong>" + String(h) + " %</strong></p>";

  html += "<h2>Datos de la API</h2>";
  html += "<p>Temperatura: <strong>" + String(apiTemperature) + " °C</strong></p>";
  html += "<p>Humedad: <strong>" + String(apiHumidity) + " %</strong></p>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void getWeatherData() //* Obtener los datos de la API
{
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(apiURL);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      StaticJsonDocument<1024> doc;
      deserializeJson(doc, payload);

      apiTemperature = doc["main"]["temp"];
      apiHumidity = doc["main"]["humidity"];
    }
    http.end();
  } else {
    Serial.println("Error de conexión WiFi");
  }
}