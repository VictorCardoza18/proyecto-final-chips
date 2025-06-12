#include "sensores_gas.h"

// ========== CONSTANTES PRIVADAS ==========
// Sensores MQ-2 múltiples
const float RL = 5.0;
const float VC = 5.0;
float R0[5] = {3.23, 3.41, 3.15, 3.33, 3.27};
float m[5] = {-0.36, -0.44, -0.47, -0.41, -0.38};
float b[5] = {0.48, 0.52, 0.36, 0.25, 0.74};
const int pines_mq2[5] = {PIN_MQ2_CO, PIN_MQ2_HUMO, PIN_MQ2_LPG, PIN_MQ2_ALCOHOL, PIN_MQ2_METANO};
String nombres_gases[5] = {"CO", "Humo", "LPG", "Alcohol", "Metano"};

// MQ-2 Principal
const float RL_PRINCIPAL = 1.0;
const float RATIO_AIRE_LIMPIO = 9.83;
const float A_LOG = 574.25;
const float B_LOG = -2.222;
const float A_LIN = 825.7;
const float B_LIN = 51.4;
float R0_principal = 1.0;

// ⭐ MQ135 Variables ⭐
float R0_mq135 = 0.0;

// ========== FUNCIONES PRIVADAS ==========
float calcularPPM(float rs_ro_ratio, float m, float b)
{
    float log_ppm = (log10(rs_ro_ratio) - b) / m;
    return pow(10, log_ppm);
}

float leerSensorMQ2Individual(int pin, int indice)
{
    int adc = analogRead(pin);
    float vout = adc * (VC / ADC_MAX);

    if (vout <= 0)
        return 0.0;

    float rs = RL * ((VC - vout) / vout);
    float rs_ro_ratio = rs / R0[indice];
    return calcularPPM(rs_ro_ratio, m[indice], b[indice]);
}

DatosMQ2Principal leerMQ2Principal()
{
    DatosMQ2Principal datos;

    int adc = analogRead(PIN_MQ2_PRINCIPAL);
    float voltage = adc * VREF / ADC_MAX;
    float rs = ((VREF - voltage) / voltage) * RL_PRINCIPAL;
    datos.ratio = rs / R0_principal;

    datos.ppm_logaritmico = A_LOG * pow(datos.ratio, B_LOG);
    float inv = (datos.ratio > 0) ? (1.0 / datos.ratio) : 0.0;
    datos.ppm_lineal = A_LIN * inv + B_LIN;

    return datos;
}

// ⭐ NUEVAS FUNCIONES PARA MQ135 ⭐
float calibrarSensorMQ135(int muestras = 20, int intervalo = 500)
{
    Serial.println("🌍 Calibrando MQ135 en aire limpio...");
    float sumaR0 = 0.0;

    for (int i = 0; i < muestras; i++)
    {
        int adcValue = analogRead(PIN_MQ135);
        float voltage = adcValue * (VREF / ADC_MAX);
        float rs = ((VREF - voltage) * RL_MQ135) / voltage;
        float r0 = rs / pow(CO2_AIRE_LIMPIO / A_MQ135, 1.0 / B_MQ135);
        sumaR0 += r0;

        Serial.printf("  Muestra %d/%d: R0 = %.2f\n", i + 1, muestras, r0);
        delay(intervalo);
    }

    return sumaR0 / muestras;
}

float leerCO2(int muestras = 10, int intervalo = 200)
{
    float sumaPPM = 0.0;

    for (int i = 0; i < muestras; i++)
    {
        int adcValue = analogRead(PIN_MQ135);
        float voltage = adcValue * (VREF / ADC_MAX);
        float rs = ((VREF - voltage) * RL_MQ135) / voltage;
        float ratio = rs / R0_mq135;
        float ppm = A_MQ135 * pow(ratio, B_MQ135);
        sumaPPM += ppm;
        delay(intervalo);
    }

    return sumaPPM / muestras;
}

DatosMQ135 leerMQ135()
{
    DatosMQ135 datos;

    int adcValue = analogRead(PIN_MQ135);
    datos.voltage = adcValue * (VREF / ADC_MAX);
    datos.rs = ((VREF - datos.voltage) * RL_MQ135) / datos.voltage;
    datos.ratio = datos.rs / R0_mq135;
    datos.co2_ppm = A_MQ135 * pow(datos.ratio, B_MQ135);

    return datos;
}

// ========== FUNCIONES PÚBLICAS ==========
void inicializarSensoresGas()
{
    Serial.println("🔧 Inicializando sensores de gas...");

    // Configurar resolución ADC
    analogReadResolution(12);

    // Configurar pines como entrada
    for (int i = 0; i < 5; i++)
    {
        pinMode(pines_mq2[i], INPUT);
    }
    pinMode(PIN_MQ2_PRINCIPAL, INPUT);
    pinMode(PIN_MQ135, INPUT); // ⭐ Nuevo pin ⭐

    // Calibrar sensores
    calibrarMQ2Principal();
    calibrarMQ135();

    Serial.println("✅ Sensores de gas inicializados");
}

void calibrarMQ2Principal()
{
    Serial.println("📊 Calibrando MQ-2 principal...");

    float rs_sum = 0;
    for (int i = 0; i < 10; i++)
    {
        int adc = analogRead(PIN_MQ2_PRINCIPAL);
        float voltage = adc * VREF / ADC_MAX;
        float rs = ((VREF - voltage) / voltage) * RL_PRINCIPAL;
        rs_sum += rs;
        Serial.printf("  Calibración %d/10: Rs = %.2f kΩ\n", i + 1, rs);
        delay(500);
    }

    float avg_rs = rs_sum / 10.0;
    R0_principal = avg_rs / RATIO_AIRE_LIMPIO;
    Serial.printf("✅ R0 principal = %.4f kΩ\n", R0_principal);
}

void calibrarMQ135()
{
    Serial.println("🌍 Iniciando calibración MQ135...");
    R0_mq135 = calibrarSensorMQ135();
    Serial.printf("✅ MQ135 R0 calibrado = %.2f\n", R0_mq135);
}

DatosGases leerSensoresGas()
{
    DatosGases datos;

    // Leer sensores MQ-2 múltiples
    datos.co = leerSensorMQ2Individual(PIN_MQ2_CO, 0);
    datos.humo = leerSensorMQ2Individual(PIN_MQ2_HUMO, 1);
    datos.lpg = leerSensorMQ2Individual(PIN_MQ2_LPG, 2);
    datos.alcohol = leerSensorMQ2Individual(PIN_MQ2_ALCOHOL, 3);
    datos.metano = leerSensorMQ2Individual(PIN_MQ2_METANO, 4);

    // Leer MQ-2 principal
    int adc = analogRead(PIN_MQ2_PRINCIPAL);
    float voltage = adc * VREF / ADC_MAX;
    float rs = ((VREF - voltage) / voltage) * RL_PRINCIPAL;
    datos.ratio_principal = rs / R0_principal;

    datos.ppm_logaritmico = A_LOG * pow(datos.ratio_principal, B_LOG);
    float inv = (datos.ratio_principal > 0) ? (1.0 / datos.ratio_principal) : 0.0;
    datos.ppm_lineal = A_LIN * inv + B_LIN;

    // ⭐ Leer MQ135 para CO₂ ⭐
    datos.co2_ppm = leerCO2();

    return datos;
}

void mostrarDatosGases(const DatosGases &datos)
{
    Serial.println("🔥 === SENSORES DE GAS ===");
    Serial.printf("   CO: %.2f PPM\n", datos.co);
    Serial.printf("   Humo: %.2f PPM\n", datos.humo);
    Serial.printf("   LPG: %.2f PPM\n", datos.lpg);
    Serial.printf("   Alcohol: %.2f PPM\n", datos.alcohol);
    Serial.printf("   Metano: %.2f PPM\n", datos.metano);
    Serial.printf("   MQ-2 Principal - Log: %.0f PPM, Lin: %.0f PPM\n",
                  datos.ppm_logaritmico, datos.ppm_lineal);
    Serial.printf("   🌍 CO₂ (MQ135): %.2f PPM\n", datos.co2_ppm); // ⭐ Nuevo ⭐
}