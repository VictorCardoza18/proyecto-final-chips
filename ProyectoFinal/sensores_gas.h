#ifndef SENSORES_GAS_H
#define SENSORES_GAS_H

#include "configuracion.h"

// ========== FUNCIONES PÚBLICAS ==========
void inicializarSensoresGas();
DatosGases leerSensoresGas();
void mostrarDatosGases(const DatosGases &datos);

// ========== FUNCIONES DE CALIBRACIÓN ==========
void calibrarSensoresMQ2();
void calibrarMQ2Principal();
void calibrarMQ135(); // ⭐ Nueva función ⭐

// ========== FUNCIONES ADICIONALES ==========
DatosMQ2Principal leerMQ2Principal();
DatosMQ135 leerMQ135();                                // ⭐ Nueva función ⭐
float leerCO2(int muestras, int intervalo); // ⭐ Nueva función ⭐

#endif