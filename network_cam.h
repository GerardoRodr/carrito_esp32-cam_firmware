#ifndef NETWORK_CAM_H
#define NETWORK_CAM_H

#include <Arduino.h>

// Inicializa la conexión WiFi en modo Access Point
void initWiFi();

// Inicializa la cámara OV2640 (AI-Thinker)
bool initCamera();

#endif // NETWORK_CAM_H
