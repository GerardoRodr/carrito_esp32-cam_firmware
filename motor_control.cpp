#include "motor_control.h"

// Pines asignados
const int IN1 = 13; 
const int IN2 = 15; 
const int IN3 = 14; 
const int IN4 = 2;  

// Configuración PWM
const int pwmFreq = 5000;     
const int pwmResolution = 8;  
const int pwmMaxVelocidad = 255; 

// Canales LEDC: usar del 1 al 4 para no interferir con el canal 0 (Cámara XCLK)
const int canalIN1 = 1;
const int canalIN2 = 2;
const int canalIN3 = 3;
const int canalIN4 = 4;

void initMotors() {
    // Adjuntar pines a los canales de LEDC
    ledcAttach(IN1, pwmFreq, pwmResolution);
    ledcAttach(IN2, pwmFreq, pwmResolution);
    ledcAttach(IN3, pwmFreq, pwmResolution);
    ledcAttach(IN4, pwmFreq, pwmResolution);
    
    // Asegurar que inician detenidos
    parar();
}

void parar() {
    ledcWrite(IN1, 0); 
    ledcWrite(IN2, 0); 
    ledcWrite(IN3, 0); 
    ledcWrite(IN4, 0);
}

void adelante() {
    ledcWrite(IN1, pwmMaxVelocidad); 
    ledcWrite(IN2, 0); 
    ledcWrite(IN3, pwmMaxVelocidad); 
    ledcWrite(IN4, 0);
}

void atras() {
    ledcWrite(IN1, 0); 
    ledcWrite(IN2, pwmMaxVelocidad); 
    ledcWrite(IN3, 0); 
    ledcWrite(IN4, pwmMaxVelocidad);
}

void izquierda() {
    ledcWrite(IN1, 0); 
    ledcWrite(IN2, pwmMaxVelocidad); 
    ledcWrite(IN3, pwmMaxVelocidad); 
    ledcWrite(IN4, 0);
}

void derecha() {
    ledcWrite(IN1, pwmMaxVelocidad); 
    ledcWrite(IN2, 0); 
    ledcWrite(IN3, 0); 
    ledcWrite(IN4, pwmMaxVelocidad);
}
