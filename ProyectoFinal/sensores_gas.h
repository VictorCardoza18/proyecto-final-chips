#ifndef SENSORES_GAS_H
#define SENSORES_GAS_H

#include "configuracion.h"

// ========== FUNCIONES PÚBLICAS ==========
void inicializarSensoresGas();
DatosGases leerSensoresGas();
void mostrarDatosGases(const DatosGases& datos);

// ========== FUNCIONES DE CALIBRACIÓN ==========
void calibrarSensoresMQ2();
void calibrarMQ2Principal();

// ========== FUNCIONES ADICIONALES ==========
DatosMQ2Principal leerMQ2Principal(); // ⭐ Ahora funciona correctamente ⭐

#endif