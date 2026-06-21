#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>

// Definición de pines para el L298N
extern const int IN1;
extern const int IN2;
extern const int IN3;
extern const int IN4;

// Inicializa los pines y PWM
void initMotors();

// Configurar velocidad (0 - 255)
void setVelocidad(int v);

// Funciones de movimiento
void adelante();
void atras();
void izquierda();
void derecha();
void parar();

#endif // MOTOR_CONTROL_H
