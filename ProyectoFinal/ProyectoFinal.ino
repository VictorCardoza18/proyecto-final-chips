#include "configuracion.h"
#include "sensores_gas.h"
#include "sensores_ambiente.h"
#include "comunicaciones.h"

void setup()
{
    Serial.begin(115200);
    Serial.println("=== PROYECTO FINAL ESP32 - SENSORES SIMPLIFICADO ===");
    Serial.println("Sensores: CO₂, Butano, Humo, Humedad Aire, Humedad Tierra");
    
    // Inicializar todos los módulos
    inicializarComunicaciones();
    inicializarSensoresGas();
    inicializarSensoresAmbiente();
    
    Serial.println("✅ Sistema inicializado correctamente");
}

void loop()
{
    static unsigned long ultimaLectura = 0;
    
    if (millis() - ultimaLectura >= INTERVALO_LECTURAS)
    {
        ultimaLectura = millis();
        
        Serial.println("\n========== NUEVA LECTURA ==========");
        
        // Obtener timestamp
        String timestamp = obtenerTimestamp();
        
        // Leer sensores simplificados
        DatosGases datosGases = leerSensoresGas();
        DatosAmbiente datosAmbiente = leerSensoresAmbiente();
        
        // Mostrar datos por consola
        mostrarDatosGases(datosGases);
        mostrarDatosAmbiente(datosAmbiente);
        
        // Enviar a Firebase
        enviarDatosFirebase(datosGases, datosAmbiente, timestamp);
        
        Serial.println("===================================\n");
    }
    
    delay(100);
}