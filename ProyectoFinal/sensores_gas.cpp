#include "sensores_gas.h"

// ========== VARIABLES PRIVADAS ==========
// Calibración MQ-2
float R0_butano = 5.5;   // Valor inicial para butano
float R0_humo = 6.2;     // Valor inicial para humo

// Coeficientes para butano y humo (MQ-2)
float m_butano = -0.45;  // Pendiente para butano
float b_butano = 0.65;   // Intercepto para butano
float m_humo = -0.42;    // Pendiente para humo
float b_humo = 0.55;     // Intercepto para humo

// MQ135 para CO₂
float R0_co2 = 10.0;

// ========== FUNCIONES PRIVADAS ==========
float calcularPPM_MQ2(float rs_ro_ratio, float m, float b)
{
    if (rs_ro_ratio <= 0) return 0.0;
    float log_ppm = (log10(rs_ro_ratio) - b) / m;
    return pow(10, log_ppm);
}

float leerSensorMQ2(int pin, float r0, float m, float b)
{
    int adc = analogRead(pin);
    float vout = adc * (VC_MQ2 / ADC_MAX);
    
    if (vout <= 0) return 0.0;
    
    float rs = RL_MQ2 * ((VC_MQ2 - vout) / vout);
    float rs_ro_ratio = rs / r0;
    return calcularPPM_MQ2(rs_ro_ratio, m, b);
}

float calibrarSensorMQ135(int muestras = 20, int intervalo = 500)
{
    Serial.println("🌍 Calibrando MQ135 para CO₂...");
    float sumaR0 = 0.0;
    
    for (int i = 0; i < muestras; i++)
    {
        int adcValue = analogRead(PIN_MQ135_CO2);
        float voltage = adcValue * (VREF / ADC_MAX);
        if (voltage > 0) {
            float rs = ((VREF - voltage) * RL_MQ135) / voltage;
            float r0 = rs / pow(CO2_AIRE_LIMPIO / A_MQ135, 1.0 / B_MQ135);
            sumaR0 += r0;
        }
        
        Serial.printf("  Muestra %d/%d\n", i + 1, muestras);
        delay(intervalo);
    }
    
    return sumaR0 / muestras;
}

// ========== FUNCIONES PÚBLICAS ==========
void inicializarSensoresGas()
{
    Serial.println("🔧 Inicializando sensores de gas...");
    
    // Configurar resolución ADC
    analogReadResolution(12);
    
    // Configurar pines como entrada
    pinMode(PIN_MQ2_BUTANO, INPUT);
    pinMode(PIN_MQ2_HUMO, INPUT);
    pinMode(PIN_MQ135_CO2, INPUT);
    
    // Calibrar sensores
    calibrarMQ135();
    calibrarMQ2();
    
    Serial.println("✅ Sensores de gas inicializados");
}

void calibrarMQ135()
{
    Serial.println("🌍 Iniciando calibración MQ135 para CO₂...");
    R0_co2 = calibrarSensorMQ135();
    Serial.printf("✅ MQ135 R0 calibrado = %.2f\n", R0_co2);
}

void calibrarMQ2()
{
    Serial.println("🔥 Calibrando sensores MQ-2...");
    
    int lecturas = 10;
    float suma_butano = 0, suma_humo = 0;
    
    for (int i = 0; i < lecturas; i++) {
        int adc_butano = analogRead(PIN_MQ2_BUTANO);
        int adc_humo = analogRead(PIN_MQ2_HUMO);
        
        float v_butano = adc_butano * (VC_MQ2 / ADC_MAX);
        float v_humo = adc_humo * (VC_MQ2 / ADC_MAX);
        
        if (v_butano > 0) suma_butano += RL_MQ2 * ((VC_MQ2 - v_butano) / v_butano);
        if (v_humo > 0) suma_humo += RL_MQ2 * ((VC_MQ2 - v_humo) / v_humo);
        
        delay(300);
    }
    
    R0_butano = (suma_butano / lecturas) / 9.5;  // Factor típico aire limpio
    R0_humo = (suma_humo / lecturas) / 9.5;
    
    Serial.printf("✅ R0 Butano = %.2f, R0 Humo = %.2f\n", R0_butano, R0_humo);
}

float leerCO2(int muestras, int intervalo)
{
    float sumaPPM = 0.0;
    
    for (int i = 0; i < muestras; i++)
    {
        int adcValue = analogRead(PIN_MQ135_CO2);
        float voltage = adcValue * (VREF / ADC_MAX);
        
        if (voltage > 0 && R0_co2 > 0) {
            float rs = ((VREF - voltage) * RL_MQ135) / voltage;
            float ratio = rs / R0_co2;
            float ppm = A_MQ135 * pow(ratio, B_MQ135);
            sumaPPM += ppm;
        }
        delay(intervalo);
    }
    
    return sumaPPM / muestras;
}

DatosGases leerSensoresGas()
{
    DatosGases datos;
    
    // Leer solo los 3 gases que necesitas
    datos.butano_ppm = leerSensorMQ2(PIN_MQ2_BUTANO, R0_butano, m_butano, b_butano);
    datos.humo_ppm = leerSensorMQ2(PIN_MQ2_HUMO, R0_humo, m_humo, b_humo);
    datos.co2_ppm = leerCO2();
    
    // Validar valores
    if (datos.butano_ppm < 0) datos.butano_ppm = 0;
    if (datos.humo_ppm < 0) datos.humo_ppm = 0;
    if (datos.co2_ppm < 0) datos.co2_ppm = 0;
    
    return datos;
}

void mostrarDatosGases(const DatosGases& datos)
{
    Serial.println("🔥 === SENSORES DE GAS ===");
    Serial.printf("   🟦 Butano: %.2f PPM\n", datos.butano_ppm);
    Serial.printf("   💨 Humo: %.2f PPM\n", datos.humo_ppm);
    Serial.printf("   🌍 CO₂: %.2f PPM\n", datos.co2_ppm);
}