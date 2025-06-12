#ifndef COMUNICACIONES_H
#define COMUNICACIONES_H

#include "configuracion.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "time.h"

// ========== FUNCIONES PÚBLICAS ==========
void inicializarComunicaciones();
bool conectarWiFi();
String obtenerTimestamp();
void enviarDatosFirebase(const DatosGases &gases, const DatosAmbiente &ambiente, const String &timestamp);
void guardarBackupLocal(const String &datos);

#endif