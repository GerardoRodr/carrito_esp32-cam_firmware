#include <WiFi.h>
#include <WebServer.h>
#include "esp_camera.h"

// --- CONFIGURACION DE PINES DE MOTORES ---
const int IN1 = 13; 
const int IN2 = 15; 
const int IN3 = 14; 
const int IN4 = 2;  

// --- CONFIGURACION PWM ---
const int freq = 5000;     
const int resolucion = 8;  
int velocidad = 255; // Máxima potencia recomendada al inicio

// --- CONFIGURACION WIFI AP ---
const char* ssid = "ESP32_Carrito_Vision";
const char* password = "password123";

WebServer server(80);

// ==========================================
// CONFIGURACIÓN DE PINES DE LA CÁMARA (Modelo AI-Thinker)
// ==========================================
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// --- FUNCIONES DE MOVIMIENTO ---
// (Mantenemos tus funciones exactas)
void parar() {
  ledcWrite(IN1, 0); ledcWrite(IN2, 0); ledcWrite(IN3, 0); ledcWrite(IN4, 0);
}
void adelante() {
  ledcWrite(IN1, velocidad); ledcWrite(IN2, 0); ledcWrite(IN3, velocidad); ledcWrite(IN4, 0);
}
void atras() {
  ledcWrite(IN1, 0); ledcWrite(IN2, velocidad); ledcWrite(IN3, 0); ledcWrite(IN4, velocidad);
}
void izquierda() {
  ledcWrite(IN1, 0); ledcWrite(IN2, velocidad); ledcWrite(IN3, velocidad); ledcWrite(IN4, 0);
}
void derecha() {
  ledcWrite(IN1, velocidad); ledcWrite(IN2, 0); ledcWrite(IN3, 0); ledcWrite(IN4, velocidad);
}

// --- ENDPOINTS HTTP ---
void configurarRutas() {
  server.on("/adelante", []() { adelante(); server.send(200, "text/plain", "OK"); });
  server.on("/atras", []() { atras(); server.send(200, "text/plain", "OK"); });
  server.on("/izquierda", []() { izquierda(); server.send(200, "text/plain", "OK"); });
  server.on("/derecha", []() { derecha(); server.send(200, "text/plain", "OK"); });
  server.on("/parar", []() { parar(); server.send(200, "text/plain", "OK"); });
}

void setup() {
  Serial.begin(115200);

  // 1. Iniciar Motores
  ledcAttach(IN1, freq, resolucion);
  ledcAttach(IN2, freq, resolucion);
  ledcAttach(IN3, freq, resolucion);
  ledcAttach(IN4, freq, resolucion);
  parar();

  // 2. Configurar Cámara
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0; // Usar canal libre para la cámara
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  // Para enviar video rápido por WiFi, usamos baja resolución
  config.frame_size = FRAMESIZE_QVGA; // 320x240
  config.jpeg_quality = 12; // Menor número = mayor calidad, pero más pesado
  config.fb_count = 1;

  // Inicializar cámara
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Error al inicializar la cámara: 0x%x", err);
    return;
  }

  // 3. Iniciar WiFi
  WiFi.softAP(ssid, password);
  Serial.print("IP del Carrito: ");
  Serial.println(WiFi.softAPIP());

  // 4. Iniciar Servidor de Control
  configurarRutas();
  server.begin();
  
  // IMPORTANTE: Aquí faltaría iniciar el servidor de STREAMING (video).
  // Normalmente se hace llamando a una función como startCameraServer();
}

void loop() {
  server.handleClient(); // Escucha comandos de movimiento
}