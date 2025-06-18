#ifndef CONFIGURACION_H
#define CONFIGURACION_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <DHT.h>
#include <time.h>

// ========== CONFIGURACIÓN WIFI ==========
#define WIFI_SSID "UrbanDataIsland_2.4"
#define WIFI_PASSWORD "FZAzXBnqEcP4"
#define FIREBASE_HOST "https://humedad-en-esp32-default-rtdb.firebaseio.com/"

// ========== PINES SENSORES SIMPLIFICADOS ==========
// Solo los sensores que necesitas
#define PIN_MQ2_HUMO 34       // MQ-2 para humo
#define PIN_MQ2_BUTANO 35     // MQ-2 para butano (usar pin LPG)
#define PIN_MQ135_CO2 32      // MQ135 para CO₂

// Sensores ambientales
#define PIN_DHT11 33
#define PIN_HUMEDAD_SUELO 13

// ========== CONSTANTES TEMPORALES ==========
#define INTERVALO_LECTURAS 15000 // 15 segundos
#define TIMEOUT_WIFI 10000       // 10 segundos

// ========== CONSTANTES SENSORES ==========
#define ADC_MAX 4095.0
#define VREF 3.3
#define VALOR_SECO 4095
#define VALOR_HUMEDO 1000

// Constantes MQ-2
#define VC_MQ2 3.3
#define RL_MQ2 1000.0

// Constantes MQ135
#define RL_MQ135 1000.0
#define A_MQ135 116.6020682
#define B_MQ135 -2.769034857
#define CO2_AIRE_LIMPIO 400.0

// Configuración DHT11
#define DHT_TYPE DHT11

// ========== ESTRUCTURAS DE DATOS SIMPLIFICADAS ==========
struct DatosGases
{
    float humo_ppm;      // Solo humo
    float butano_ppm;    // Solo butano 
    float co2_ppm;       // Solo CO₂
};

struct DatosAmbiente
{
    float humedad_suelo;    // Humedad del suelo
    float humedad_aire;     // Humedad del aire
    float temperatura;      // Temperatura del aire
};

#endif