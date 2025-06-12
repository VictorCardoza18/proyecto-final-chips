#include <WiFi.h>
#include <HTTPClient.h>

const char *ssid = "POCO X6";
const char *password = "abcd1234";
const char *serverURL = "http://192.168.165.164:5000/data"; // IP del servidor Python

// Configuración del sensor MQ135
#define MQ135_PIN 34   // Pin analógico conectado al MQ135
#define RL 1000.0      // Resistencia de carga en ohmios (1k)
#define ADC_MAX 4095.0 // Resolución de 12 bits
#define V_REF 3.3      // Voltaje de referencia
#define A 116.6020682  // Constante A del sensor
#define B -2.769034857 // Constante B del sensor

float R0 = 0.0; // Valor de R0, se calibrará en tiempo de ejecución

// Función para calibrar el sensor MQ135 en aire limpio (~400 ppm de CO₂)
float calibrarSensor(int muestras = 20, int intervalo = 500)
{
    float sumaR0 = 0.0;
    for (int i = 0; i < muestras; i++)
    {
        int adcValue = analogRead(MQ135_PIN);
        float voltage = adcValue * (V_REF / ADC_MAX);
        float rs = ((V_REF - voltage) * RL) / voltage;
        float r0 = rs / pow(400.0 / A, 1.0 / B);
        sumaR0 += r0;
        delay(intervalo);
    }
    return sumaR0 / muestras;
}

// Función para leer y calcular la concentración de CO₂
float leerCO2(int muestras = 10, int intervalo = 200)
{
    float sumaPPM = 0.0;
    for (int i = 0; i < muestras; i++)
    {
        int adcValue = analogRead(MQ135_PIN);
        float voltage = adcValue * (V_REF / ADC_MAX);
        float rs = ((V_REF - voltage) * RL) / voltage;
        float ratio = rs / R0;
        float ppm = A * pow(ratio, B);
        sumaPPM += ppm;
        delay(intervalo);
    }
    return sumaPPM / muestras;
}

void setup()
{
    delay(1000); // Espera para estabilizar el monitor serial
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi conectado");

    delay(1000); // Espera para estabilizar el monitor serial
    // Configuración de la resolución del ADC
    analogReadResolution(12); // Resolución de 12 bits
    // Calibración del sensor MQ135
    Serial.println("Calibrando el sensor MQ135 en aire limpio...");
    R0 = calibrarSensor();
    // R0 = 90240.70;
    Serial.print("Valor de R0 calibrado: ");
    Serial.println(R0, 2);
}

void loop()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;

        // Lectura de la concentración de CO₂
        float ppmCO2 = leerCO2();
        Serial.print("Concentración de CO₂: ");
        Serial.print(ppmCO2, 2);
        Serial.println(" ppm");
        http.begin(serverURL);
        http.addHeader("Content-Type", "application/json");
        String jsonData = "{\"ppm\":" + String(ppmCO2, 2) + "}";
        Serial.println("Enviando a: " + String(serverURL));
        Serial.println("Datos: " + jsonData);
        int httpResponseCode = http.POST(jsonData);

        Serial.print("Código de respuesta: ");
        Serial.println(httpResponseCode);
        if (httpResponseCode <= 0)
        {
            Serial.print("Error HTTP: ");
            Serial.println(http.errorToString(httpResponseCode).c_str());
        }
        http.end();
    }
    delay(15000); // Espera 15 segundos
}
