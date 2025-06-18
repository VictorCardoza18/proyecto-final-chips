#ifndef SENSORES_GAS_H
#define SENSORES_GAS_H

#include "configuracion.h"

// ========== FUNCIONES PÚBLICAS ==========
void inicializarSensoresGas();
DatosGases leerSensoresGas();
void mostrarDatosGases(const DatosGases &datos);

// ========== FUNCIONES DE CALIBRACIÓN ==========
void calibrarMQ2();
void calibrarMQ135();

// ========== FUNCIONES ADICIONALES ==========
float leerCO2(int muestras = 5, int intervalo = 200);

#endif