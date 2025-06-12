#include <WiFi.h>
#include <HTTPClient.h>

// CONFIGURACIÓN
const char *ssid = "POCO M6 Pro";
const char *password = "contrasena";
const char *firebaseHost = "https://gas-esp32-default-rtdb.firebaseio.com";
const char *firebasePath = "/sensores.json";

const int MQ2_PINS[5] = {32, 33, 34, 35, 36};
const float RL = 5.0;
const float VC = 5.0;

float R0[5] = {3.23, 3.41, 3.15, 3.33, 3.27};
float m[5] = {-0.36, -0.44, -0.47, -0.41, -0.38};
float b[5] = {0.48, 0.52, 0.36, 0.25, 0.74};

String gas_names[5] = {"CO", "Humo", "LPG", "Alcohol", "Metano"};

float getPPM(float rs_ro_ratio, float m, float b)
{
    float log_ppm = (log10(rs_ro_ratio) - b) / m;
    return pow(10, log_ppm);
}

void setup()
{
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    Serial.print("Conectando a WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n Conectado a WiFi");
}

void loop()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        String json = "{";
        for (int i = 0; i < 5; i++)
        {
            int adc = analogRead(MQ2_PINS[i]);
            float vout = adc * (VC / 4095.0);

            float ppm = 0.0;
            if (vout > 0)
            {
                float rs = RL * ((VC - vout) / vout);
                float rs_ro_ratio = rs / R0[i];
                ppm = getPPM(rs_ro_ratio, m[i], b[i]);
            }

            json += "\"" + gas_names[i] + "\":" + String(ppm, 2);
            if (i < 4)
                json += ",";
        }
        json += "}";

        HTTPClient http;
        String url = String(firebaseHost) + firebasePath;
        http.begin(url);
        http.addHeader("Content-Type", "application/json");

        int httpResponseCode = http.PUT(json);
        Serial.println("Datos enviados: " + json);
        Serial.println("Código de respuesta: " + String(httpResponseCode));

        http.end();
    }
    else
    {
        Serial.println("WiFi desconectado");
    }

    delay(3000);
}