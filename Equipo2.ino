#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>
#include "time.h"
#include "DHT.h"

#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Variables para suavizado exponencial
float valorFiltrado = 0;
const float alpha = 0.1;

const char *ssid = "INFINITUM1B1E";
const char *password = "";
const char *firebaseUrl = "https://humedad-en-esp32-default-rtdb.firebaseio.com/lecturas.json";

#define PIN_HUMEDAD_SUELO 36
#define SLEEP_TIME_MS 600000 // 1 minuto en milisegundos

void setup()
{
    Serial.begin(115200);
    dht.begin();

    if (!SPIFFS.begin(true))
    {
        Serial.println("Error al montar SPIFFS");
        return;
    }

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi conectado");

    configTime(-6 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    delay(1000); // esperar sincronización de hora
}

void loop()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Error obteniendo la hora");
        delay(SLEEP_TIME_MS);
        return;
    }

    // Formatear la hora actual
    char timestamp[25];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:00", &timeinfo);
    Serial.print("Timestamp actual: ");
    Serial.println(timestamp);

    // Lectura del sensor FC-28 con suavizado exponencial
    int lecturaActual = analogRead(PIN_HUMEDAD_SUELO);
    valorFiltrado = alpha * lecturaActual + (1 - alpha) * valorFiltrado;

    // Cálculo del porcentaje de humedad del suelo
    float humedadSuelo = ((float)(valorSeco - valorFiltrado) / (valorSeco - valorHumedo)) * 100.0;
    humedadSuelo = constrain(humedadSuelo, 0.0, 100.0);

    // Lectura del DHT11
    float humedadAire = dht.readHumidity();
    float temperatura = dht.readTemperature();

    // Validar lecturas del DHT11
    if (isnan(humedadAire) || isnan(temperatura))
    {
        Serial.println("Error al leer del DHT11");
        delay(SLEEP_TIME_MS);
        return;
    }

    Serial.printf("Humedad suelo: %d%% | Humedad aire: %.1f%% | Temperatura: %.1f°C\n",
                  humedadSuelo, humedadAire, temperatura);

    // Crear JSON
    DynamicJsonDocument doc(512);
    JsonObject lectura = doc.createNestedObject(timestamp);
    lectura["humedad_suelo"] = humedadSuelo;
    lectura["humedad_aire"] = round(humedadAire);
    lectura["temperatura"] = round(temperatura * 10) / 10.0;
    lectura["timestamp"] = timestamp;

    // Guardar localmente
    File file = SPIFFS.open("/lecturas.json", FILE_APPEND);
    if (file)
    {
        serializeJson(lectura, file);
        file.println();
        file.close();
        Serial.println("Guardado localmente en lecturas.json");
    }
    else
    {
        Serial.println("Error al guardar localmente");
    }

    // Enviar a Firebase
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        http.begin(firebaseUrl);
        http.addHeader("Content-Type", "application/json");

        String jsonStr;
        serializeJson(doc, jsonStr);

        int httpCode = http.PATCH(jsonStr);
        if (httpCode > 0)
        {
            Serial.printf("Datos enviados, código: %d\n", httpCode);
        }
        else
        {
            Serial.printf("Error HTTP: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    }
    else
    {
        Serial.println("Sin conexión WiFi, no se envió a Firebase");
    }

    Serial.printf("Esperando %d segundos...\n", SLEEP_TIME_MS / 1000);
    delay(SLEEP_TIME_MS);
}