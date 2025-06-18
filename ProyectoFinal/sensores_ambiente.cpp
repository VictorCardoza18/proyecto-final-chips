#include "sensores_ambiente.h"

// ========== VARIABLES PRIVADAS ==========
DHT dht(PIN_DHT11, DHT_TYPE);

// ========== FUNCIONES PÚBLICAS ==========
void inicializarSensoresAmbiente()
{
    Serial.println("🌡️ Inicializando sensores ambientales...");
    
    // Inicializar DHT11
    dht.begin();
    
    // Configurar pin de humedad del suelo
    pinMode(PIN_HUMEDAD_SUELO, INPUT);
    
    Serial.println("✅ Sensores ambientales inicializados");
}

DatosAmbiente leerSensoresAmbiente()
{
    DatosAmbiente datos;
    
    // Leer DHT11 (humedad del aire y temperatura)
    datos.humedad_aire = dht.readHumidity();
    datos.temperatura = dht.readTemperature();
    
    // Leer humedad del suelo
    int valorSuelo = analogRead(PIN_HUMEDAD_SUELO);
    datos.humedad_suelo = map(valorSuelo, VALOR_SECO, VALOR_HUMEDO, 0, 100);
    
    // Validar lecturas del DHT11
    if (isnan(datos.humedad_aire)) datos.humedad_aire = 0;
    if (isnan(datos.temperatura)) datos.temperatura = 0;
    
    // Validar humedad del suelo
    if (datos.humedad_suelo < 0) datos.humedad_suelo = 0;
    if (datos.humedad_suelo > 100) datos.humedad_suelo = 100;
    
    return datos;
}

void mostrarDatosAmbiente(const DatosAmbiente& datos)
{
    Serial.println("🌡️ === SENSORES AMBIENTALES ===");
    Serial.printf("   💧 Humedad Suelo: %.1f%%\n", datos.humedad_suelo);
    Serial.printf("   🌊 Humedad Aire: %.1f%%\n", datos.humedad_aire);
    Serial.printf("   🌡️ Temperatura: %.1f°C\n", datos.temperatura);
}