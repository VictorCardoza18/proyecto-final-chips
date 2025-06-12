#include "comunicaciones.h"

// ========== FUNCIONES PÚBLICAS ==========
void inicializarComunicaciones()
{
    Serial.println("📡 Inicializando comunicaciones...");

    // Inicializar SPIFFS
    if (!SPIFFS.begin(true))
    {
        Serial.println("❌ Error al montar SPIFFS");
    }
    else
    {
        Serial.println("✅ SPIFFS inicializado");
    }

    // Conectar WiFi
    if (conectarWiFi())
    {
        // Configurar tiempo
        configTime(-6 * 3600, 0, "pool.ntp.org", "time.nist.gov");
        delay(2000);
        Serial.println("✅ Comunicaciones inicializadas");
    }
}

bool conectarWiFi()
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Conectando a WiFi");

    unsigned long inicio = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - inicio) < TIMEOUT_WIFI)
    {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println();
        Serial.print("✅ WiFi conectado - IP: ");
        Serial.println(WiFi.localIP());
        return true;
    }
    else
    {
        Serial.println("\n❌ Error conectando WiFi");
        return false;
    }
}

String obtenerTimestamp()
{
    struct tm timeinfo;
    char timestamp[25];

    if (getLocalTime(&timeinfo))
    {
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", &timeinfo);
    }
    else
    {
        sprintf(timestamp, "2024-01-01T00:00:00");
    }

    return String(timestamp);
}

void enviarDatosFirebase(const DatosGases &gases, const DatosAmbiente &ambiente, const String &timestamp)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("❌ WiFi desconectado - Intentando reconectar...");
        conectarWiFi();
        return;
    }

    // Crear JSON
    DynamicJsonDocument doc(2048);

    // Datos de gases
    doc["gases"]["CO"] = round(gases.co * 100) / 100.0;
    doc["gases"]["Humo"] = round(gases.humo * 100) / 100.0;
    doc["gases"]["LPG"] = round(gases.lpg * 100) / 100.0;
    doc["gases"]["Alcohol"] = round(gases.alcohol * 100) / 100.0;
    doc["gases"]["Metano"] = round(gases.metano * 100) / 100.0;

    // MQ-2 Principal
    doc["mq2_principal"]["ppm_logaritmico"] = round(gases.ppm_logaritmico);
    doc["mq2_principal"]["ppm_lineal"] = round(gases.ppm_lineal);
    doc["mq2_principal"]["ratio"] = round(gases.ratio_principal * 100) / 100.0;

    // Datos ambientales
    doc["ambiente"]["humedad_suelo"] = round(ambiente.humedad_suelo * 10) / 10.0;
    doc["ambiente"]["humedad_aire"] = round(ambiente.humedad_aire * 10) / 10.0;
    doc["ambiente"]["temperatura"] = round(ambiente.temperatura * 10) / 10.0;

    // Metadatos
    doc["timestamp"] = timestamp;
    doc["device_id"] = "ESP32_Unificado_v2";

    // Enviar a Firebase
    HTTPClient http;
    String firebaseURL = String(FIREBASE_HOST) + "/lecturas_unificadas.json";
    http.begin(firebaseURL);
    http.addHeader("Content-Type", "application/json");

    String jsonString;
    serializeJson(doc, jsonString);

    int httpResponseCode = http.PATCH(jsonString);

    if (httpResponseCode > 0)
    {
        Serial.printf("✅ Datos enviados a Firebase - Código: %d\n", httpResponseCode);
        guardarBackupLocal(jsonString);
    }
    else
    {
        Serial.printf("❌ Error Firebase: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end();
}

void guardarBackupLocal(const String &datos)
{
    File file = SPIFFS.open("/backup_lecturas.json", FILE_APPEND);
    if (file)
    {
        file.println(datos);
        file.close();
        Serial.println("💾 Backup guardado localmente");
    }
    else
    {
        Serial.println("❌ Error guardando backup");
    }
}