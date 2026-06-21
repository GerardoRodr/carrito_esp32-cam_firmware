#include "motor_control.h"

// Pines asignados
const int IN1 = 13; 
const int IN2 = 15; 
const int IN3 = 14; 
const int IN4 = 2;  

// Configuración PWM
const int pwmFreq = 5000;     
const int pwmResolution = 8;  
int pwmVelocidad = 255; 

// Estado actual del movimiento
enum EstadoMovimiento { PARADO, ADELANTE, ATRAS, IZQ, DER };
EstadoMovimiento estadoActual = PARADO;

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

void setVelocidad(int v) {
    // Restringir el valor entre 0 y 255
    if (v < 0) v = 0;
    if (v > 255) v = 255;
    pwmVelocidad = v;
    
    // Si el carrito se está moviendo, aplicar la nueva velocidad de inmediato
    if (estadoActual == ADELANTE) adelante();
    else if (estadoActual == ATRAS) atras();
    else if (estadoActual == IZQ) izquierda();
    else if (estadoActual == DER) derecha();
}

void parar() {
    estadoActual = PARADO;
    ledcWrite(IN1, 0); 
    ledcWrite(IN2, 0); 
    ledcWrite(IN3, 0); 
    ledcWrite(IN4, 0);
}

void adelante() {
    estadoActual = ADELANTE;
    ledcWrite(IN1, pwmVelocidad); 
    ledcWrite(IN2, 0); 
    ledcWrite(IN3, pwmVelocidad); 
    ledcWrite(IN4, 0);
}

void atras() {
    estadoActual = ATRAS;
    ledcWrite(IN1, 0); 
    ledcWrite(IN2, pwmVelocidad); 
    ledcWrite(IN3, 0); 
    ledcWrite(IN4, pwmVelocidad);
}

void izquierda() {
    estadoActual = IZQ;
    ledcWrite(IN1, 0); 
    ledcWrite(IN2, pwmVelocidad); 
    ledcWrite(IN3, pwmVelocidad); 
    ledcWrite(IN4, 0);
}

void derecha() {
    estadoActual = DER;
    ledcWrite(IN1, pwmVelocidad); 
    ledcWrite(IN2, 0); 
    ledcWrite(IN3, 0); 
    ledcWrite(IN4, pwmVelocidad);
}
