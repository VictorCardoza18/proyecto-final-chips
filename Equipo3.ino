#include <WiFi.h>
#include <HTTPClient.h>

// Pines y constantes del MQ-2
#define PIN_MQ2 34         // GPIO34 (ADC1_CH6)
#define ADC_MAX 4095.0     // Resolución ADC (12 bits)
#define VREF 3.3           // Voltaje de referencia ESP32
#define RL 1.0             // Resistencia de carga (kΩ)
#define RatioCleanAir 9.83 // Rs/R0 en aire limpio

// Coeficientes del modelo logarítmico
#define A_LOG 574.25
#define B_LOG -2.222

// Modelo lineal PPM = A_LIN*(1/ratio) + B_LIN
#define A_LIN 825.7
#define B_LIN 51.4

// —————— Configuración Wi-Fi y servidor ——————
const char *ssid = "IZZI-8460";
const char *password = "V05WGQIFGQDA";
const char *serverURL = "http://192.168.0.27:3000/api/ppm";

float R0 = 1.0; // Se calibrará en setup()

void setup()
{
    Serial.begin(9600);
    delay(1000);

    // Conexión Wi-Fi
    Serial.print("Conectando a Wi-Fi ");
    Serial.println(ssid);
    if (strlen(password) == 0)
    {
        WiFi.begin(ssid);
    }
    else
    {
        WiFi.begin(ssid, password);
    }
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("Wi-Fi conectado, IP: ");
    Serial.println(WiFi.localIP());

    // Calibración de R0
    Serial.println(F("\n=== Sensor MQ-2 RAW (sin biblioteca) ==="));
    Serial.println(F("Iniciando calibración de R0 en aire limpio..."));

    float rs_sum = 0;
    for (int i = 0; i < 10; i++)
    {
        int adc = analogRead(PIN_MQ2);
        float voltage = adc * VREF / ADC_MAX;
        float rs = ((VREF - voltage) / voltage) * RL;
        rs_sum += rs;
        Serial.printf("  Lectura %d: Rs = %.2f kΩ\n", i + 1, rs);
        delay(500);
    }

    float avg_rs = rs_sum / 10.0;
    R0 = avg_rs / RatioCleanAir;
    Serial.printf("✅ R0 calibrado = %.4f kΩ\n\n", R0);
}

void loop()
{
    // Lectura sensor y cálculo de PPM
    int adc = analogRead(PIN_MQ2);
    float voltage = adc * VREF / ADC_MAX;
    float rs = ((VREF - voltage) / voltage) * RL;
    float ratio = rs / R0;

    float ppm_log = A_LOG * pow(ratio, B_LOG);
    float inv = (ratio > 0) ? (1.0 / ratio) : 0.0;
    float ppm_lin = A_LIN * inv + B_LIN;

    // Mostrar por consola
    Serial.printf(
        "ADC:%4d  V:%.2f V  Rs:%.2f kΩ  Ratio:%.2f  PPM_Log:%.0f  PPM_Lin:%.0f\n",
        adc, voltage, rs, ratio, ppm_log, ppm_lin);

    // Envío por HTTP POST al servidor
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        http.begin(serverURL);
        http.addHeader("Content-Type", "application/json");

        String payload = "{\"ppm_lin\":" + String(ppm_lin, 1) + "}";
        int httpCode = http.POST(payload);

        if (httpCode > 0)
        {
            Serial.printf("HTTP POST exitoso, código: %d\n", httpCode);
        }
        else
        {
            Serial.printf("Error en HTTP POST: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    }
    else
    {
        Serial.println("Wi-Fi desconectado. Intentando reconectar...");
        WiFi.reconnect();
    }

    delay(1000);
}