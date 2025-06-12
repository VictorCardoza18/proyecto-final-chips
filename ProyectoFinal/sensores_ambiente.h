#ifndef SENSORES_AMBIENTE_H
#define SENSORES_AMBIENTE_H

#include "configuracion.h"
#include "DHT.h"

// ========== FUNCIONES PÚBLICAS ==========
void inicializarSensoresAmbiente();
DatosAmbiente leerSensoresAmbiente();
void mostrarDatosAmbiente(const DatosAmbiente &datos);

#endif