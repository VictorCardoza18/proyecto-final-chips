#include "sensores_ambiente.h"

// ========== VARIABLES PRIVADAS ==========
DHT dht(PIN_DHT11, DHT11);
float valorFiltradoSuelo = 0;
const float alpha = 0.1; // Factor de suavizado exponencial

// ========== FUNCIONES PÚBLICAS ==========
void inicializarSensoresAmbiente()
{
    Serial.println("🌡️ Inicializando sensores ambientales...");

    // Inicializar DHT11
    dht.begin();

    // Configurar pin de humedad del suelo
    pinMode(PIN_HUMEDAD_SUELO, INPUT);

    // Inicializar filtro con primera lectura
    valorFiltradoSuelo = analogRead(PIN_HUMEDAD_SUELO);

    Serial.println("✅ Sensores ambientales inicializados");
}

DatosAmbiente leerSensoresAmbiente()
{
    DatosAmbiente datos;

    // ========== HUMEDAD DEL SUELO ==========
    int lecturaActual = analogRead(PIN_HUMEDAD_SUELO);
    valorFiltradoSuelo = alpha * lecturaActual + (1 - alpha) * valorFiltradoSuelo;

    datos.humedad_suelo = ((float)(VALOR_SECO - valorFiltradoSuelo) /
                           (VALOR_SECO - VALOR_HUMEDO)) *
                          100.0;
    datos.humedad_suelo = constrain(datos.humedad_suelo, 0.0, 100.0);

    // ========== DHT11 ==========
    datos.humedad_aire = dht.readHumidity();
    datos.temperatura = dht.readTemperature();

    // Validar lecturas DHT11
    if (isnan(datos.humedad_aire))
        datos.humedad_aire = 0;
    if (isnan(datos.temperatura))
        datos.temperatura = 0;

    return datos;
}

void mostrarDatosAmbiente(const DatosAmbiente &datos)
{
    Serial.println("🌿 === SENSORES AMBIENTALES ===");
    Serial.printf("   Humedad Suelo: %.1f%%\n", datos.humedad_suelo);
    Serial.printf("   Humedad Aire: %.1f%%\n", datos.humedad_aire);
    Serial.printf("   Temperatura: %.1f°C\n", datos.temperatura);
}