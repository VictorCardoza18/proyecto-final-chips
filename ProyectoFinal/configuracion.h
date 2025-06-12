#ifndef CONFIGURACION_H
#define CONFIGURACION_H

#include <Arduino.h>

// ========== CONFIGURACIÓN WIFI ==========
#define WIFI_SSID "POCO M6 Pro"
#define WIFI_PASSWORD "contrasena"
#define FIREBASE_HOST "https://gas-esp32-default-rtdb.firebaseio.com"

// ========== PINES SENSORES ==========
// Sensores MQ-2 múltiples
#define PIN_MQ2_CO 32
#define PIN_MQ2_HUMO 33
#define PIN_MQ2_LPG 34
#define PIN_MQ2_ALCOHOL 35
#define PIN_MQ2_METANO 36

// MQ-2 Principal
#define PIN_MQ2_PRINCIPAL 39

// Sensores ambientales
#define PIN_DHT11 14
#define PIN_HUMEDAD_SUELO 25

// ========== CONSTANTES TEMPORALES ==========
#define INTERVALO_LECTURAS 10000  // 10 segundos
#define TIMEOUT_WIFI 10000        // 10 segundos

// ========== CONSTANTES SENSORES ==========
#define ADC_MAX 4095.0
#define VREF 3.3
#define VALOR_SECO 4095
#define VALOR_HUMEDO 1000

// ========== ESTRUCTURAS DE DATOS ==========
struct DatosGases {
    float co;
    float humo;
    float lpg;
    float alcohol;
    float metano;
    float ppm_logaritmico;
    float ppm_lineal;
    float ratio_principal;
};

struct DatosAmbiente {
    float humedad_suelo;
    float humedad_aire;
    float temperatura;
};

// ⭐ ESTRUCTURA FALTANTE AGREGADA ⭐
struct DatosMQ2Principal {
    float ppm_logaritmico;
    float ppm_lineal;
    float ratio;
};

#endif