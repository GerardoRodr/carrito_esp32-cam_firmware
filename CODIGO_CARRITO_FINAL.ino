#include <Arduino.h>
#include "motor_control.h"
#include "network_cam.h"
#include "http_server.h"

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();
    
    // 1. Inicializar Motores (Usa canales LEDC 1-4)
    initMotors();
    Serial.println("Motores inicializados.");

    // 2. Inicializar Cámara (Usa canal LEDC 0)
    if (!initCamera()) {
        Serial.println("Fallo fatal: No se pudo inicializar la cámara.");
        // Podríamos reiniciar aquí, pero para debugging dejamos que siga
    }

    // 3. Inicializar WiFi (Modo AP)
    initWiFi();

    // 4. Iniciar Servidores HTTP (Stream en puerto 81, Control en puerto 80)
    startServers();
    
    Serial.println("==== SISTEMA INICIADO ====");
    Serial.println("- Control de motores: http://<IP>:80/");
    Serial.println("- Streaming de video: http://<IP>:81/stream");
}

void loop() {
    // El ciclo principal queda libre.
    // La cámara y los servidores HTTP funcionan mediante tareas manejadas por FreeRTOS.
    delay(10000); 
}