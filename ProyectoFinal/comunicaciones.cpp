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
        sprintf(timestamp, "2025-06-17T22:02:20"); // Fecha actualizada como fallback
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

    // Crear JSON simplificado
    DynamicJsonDocument doc(1024);

    // Solo los 3 gases que necesitas
    doc["gases"]["Humo"] = round(gases.humo_ppm * 100) / 100.0;
    doc["gases"]["Butano"] = round(gases.butano_ppm * 100) / 100.0;
    doc["gases"]["CO2"] = round(gases.co2_ppm * 100) / 100.0;

    // Solo los datos ambientales que necesitas
    doc["ambiente"]["humedad_suelo"] = round(ambiente.humedad_suelo * 10) / 10.0;
    doc["ambiente"]["humedad_aire"] = round(ambiente.humedad_aire * 10) / 10.0;
    doc["ambiente"]["temperatura"] = round(ambiente.temperatura * 10) / 10.0;

    // Metadatos simplificados
    doc["timestamp"] = timestamp;
    doc["device_id"] = "ESP32_Simplificado";
    doc["sensores"]["humo"] = "MQ2";
    doc["sensores"]["butano"] = "MQ2";
    doc["sensores"]["co2"] = "MQ135";
    doc["sensores"]["ambiente"] = "DHT11";

    // 🔥 CAMBIO PRINCIPAL: Usar POST para crear registros únicos
    HTTPClient http;
    String firebaseURL = String(FIREBASE_HOST) + "/historial_lecturas.json"; // Nueva colección
    http.begin(firebaseURL);
    http.addHeader("Content-Type", "application/json");

    String jsonString;
    serializeJson(doc, jsonString);

    // 🔥 USAR POST en lugar de PATCH - Firebase auto-genera IDs únicos
    int httpResponseCode = http.POST(jsonString);

    if (httpResponseCode > 0)
    {
        Serial.printf("✅ Registro creado en Firebase - Código: %d\n", httpResponseCode);
        
        // Mostrar el ID generado por Firebase
        String response = http.getString();
        Serial.println("📝 Respuesta Firebase: " + response);
        
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