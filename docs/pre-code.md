```C
#include <WiFi.h>
#include <WebServer.h>

// --- CONFIGURACION DE PINES ---
// Control de Direccion y Velocidad (PWM Directo)
const int IN1 = 13; // Motor Izquierdo Adelante
const int IN2 = 15; // Motor Izquierdo Atras
const int IN3 = 14; // Motor Derecho Adelante
const int IN4 = 2;  // Motor Derecho Atras

// Pines para los Sensores Infrarrojos
const int SENSOR_IZQ = 12; // Usamos el 12 pq el 16 por alguna razon dejo de funcionar JAJAJAAJAJAJ
const int SENSOR_DER = 0;  // Recuerda levantar el carro al prenderlo por este pin

// --- CONFIGURACION PWM ---
const int freq = 5000;     
const int resolucion = 8;  

// --- CONFIGURACION WIFI AP ---
const char* ssid = "ESP32_Carrito";
const char* password = "password123"; // Minimo 8 caracteres

WebServer server(80);

// --- VARIABLES GLOBALES ---
int velocidad = 150; // Empezamos a 150 (mas lento) para probar las curvas
bool modo_automatico = false;

// --- FUNCIONES DE MOVIMIENTO (PWM) ---
void parar() {
  // Para detener un motor, ambos pines de control deben tener un ciclo de trabajo de 0.
  // Al no haber diferencia de potencial (voltaje), el motor no gira.
  ledcWrite(IN1, 0); // Corta la energia de avance izquierdo
  ledcWrite(IN2, 0); // Corta la energia de retroceso izquierdo
  ledcWrite(IN3, 0); // Corta la energia de avance derecho
  ledcWrite(IN4, 0); // Corta la energia de retroceso derecho
}

void adelante() {
  // Para avanzar, el pin 1 de cada motor recibe el PWM (velocidad)
  // y el pin 2 recibe 0, haciendo que giren hacia adelante.
  ledcWrite(IN1, velocidad); // Motor Izq: Direccion adelante activada
  ledcWrite(IN2, 0);         // Motor Izq: Direccion atras desactivada
  ledcWrite(IN3, velocidad); // Motor Der: Direccion adelante activada
  ledcWrite(IN4, 0);         // Motor Der: Direccion atras desactivada
}

void atras() {
  // Para retroceder, invertimos la polaridad logica de los pines. 
  // Ahora el pin 2 recibe el PWM (velocidad) y el pin 1 recibe 0.
  ledcWrite(IN1, 0);         // Motor Izq: Direccion adelante desactivada
  ledcWrite(IN2, velocidad); // Motor Izq: Direccion atras activada
  ledcWrite(IN3, 0);         // Motor Der: Direccion adelante desactivada
  ledcWrite(IN4, velocidad); // Motor Der: Direccion atras activada
}

void izquierda() {
  // Para girar a la izquierda sobre su propio eje:
  // El motor izquierdo debe girar hacia atras y el derecho hacia adelante.
  ledcWrite(IN1, 0);         // Motor Izq hacia atras
  ledcWrite(IN2, velocidad); 
  ledcWrite(IN3, velocidad); // Motor Der hacia adelante
  ledcWrite(IN4, 0);  
}

void derecha() {
  // Para girar a la derecha sobre su propio eje:
  // El motor izquierdo debe girar hacia adelante y el derecho hacia atras.
  ledcWrite(IN1, velocidad); // Motor Izq hacia adelante
  ledcWrite(IN2, 0);  
  ledcWrite(IN3, 0);         // Motor Der hacia atras
  ledcWrite(IN4, velocidad); 
}

// --- LOGICA DEL SEGUIDOR DE LINEA CON MEMORIA ---
void seguirLinea() {
  int valIzq = digitalRead(SENSOR_IZQ);
  int valDer = digitalRead(SENSOR_DER);

  // 1. Detección normal de curvas
  if (valIzq == HIGH && valDer == LOW) {
    // Sensor izquierdo detecta negro
    izquierda();
  } 
  else if (valIzq == LOW && valDer == HIGH) {
    // Sensor derecho detecta negro
    derecha();
  } 
  // 2. Intersección o alto (Ambos en negro)
  else if (valIzq == HIGH && valDer == HIGH) {
    parar();
  } 
  // 3. El carrito se salió de la línea (Ambos en blanco)
  else if (valIzq == LOW && valDer == LOW) {
    adelante();
  }
}

// --- PAGINA WEB (INTERFAZ) ---
String getHTML() {
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  // Extraño tailwind firme
  html += "<style>";
  html += "body { font-family: Arial; text-align: center; margin: 0px; padding: 20px; background-color: #f4f4f9; }";
  html += "h1 { color: #333; }";
  html += ".btn { display: inline-block; padding: 15px 25px; font-size: 20px; cursor: pointer; text-align: center; text-decoration: none; outline: none; color: #fff; background-color: #4CAF50; border: none; border-radius: 10px; margin: 5px; box-shadow: 0 5px #999; width: 120px; }";
  html += ".btn:active { background-color: #3e8e41; box-shadow: 0 2px #666; transform: translateY(4px); }";
  html += ".btn-red { background-color: #f44336; }";
  html += ".btn-red:active { background-color: #da190b; }";
  html += ".btn-blue { background-color: #008CBA; width: 250px;}";
  html += ".btn-blue:active { background-color: #007399; }";
  html += ".panel { background: white; padding: 15px; border-radius: 10px; margin-bottom: 20px; box-shadow: 0px 0px 10px rgba(0,0,0,0.1); }";
  html += "input[type=range] { width: 80%; margin: 15px 0; }";
  html += "</style></head><body>";
  
  html += "<h1>Panel de Control</h1>";

  // Panel de estado y sensores (Para calibracion)
  html += "<div class='panel'>";
  html += "<h2>Calibracion (Sensores)</h2>";
  html += "<p>Izq: <span id='s_izq'>-</span> | Der: <span id='s_der'>-</span></p>";
  html += "<p><i>(Ajusta el tornillo azul hasta que leas 1 en negro y 0 en blanco)</i></p>";
  html += "</div>";

  // Panel de Slider de Velocidad
  html += "<div class='panel'>";
  html += "<h2>Control de Potencia</h2>";
  html += "<input type='range' min='80' max='255' value='" + String(velocidad) + "' id='sliderVel' onchange='cambiarVel(this.value)'>";
  html += "<p>Potencia actual: <span id='txtVel'>" + String(velocidad) + "</span> / 255</p>";
  html += "</div>";

  // Panel de modos
  html += "<div class='panel'>";
  html += "<h2>Modo de Operacion</h2>";
  if(modo_automatico) {
    html += "<button class='btn btn-red' style='width:250px' onclick=\"fetch('/modo?estado=manual')\">Detener Auto</button>";
  } else {
    html += "<button class='btn btn-blue' onclick=\"fetch('/modo?estado=auto')\">Iniciar Seguidor</button>";
  }
  html += "</div>";

  // Panel manual
  html += "<div class='panel'>";
  html += "<h2>Prueba de Motores</h2>";
  html += "<button class='btn' onclick=\"fetch('/adelante')\">Adelante</button><br>";
  html += "<button class='btn' onclick=\"fetch('/izquierda')\">Izq</button>";
  html += "<button class='btn' onclick=\"fetch('/derecha')\">Der</button><br>";
  html += "<button class='btn' onclick=\"fetch('/atras')\">Atras</button><br>";
  html += "<button class='btn btn-red' onclick=\"fetch('/parar')\">Parar</button>";
  html += "</div>";

  // Script para actualizar sensores y velocidad en tiempo real
  html += "<script>";
  html += "setInterval(function() {";
  html += "  fetch('/sensores').then(response => response.json()).then(data => {";
  html += "    document.getElementById('s_izq').innerText = data.izq;";
  html += "    document.getElementById('s_der').innerText = data.der;";
  html += "  });";
  html += "}, 500);"; 
  html += "function cambiarVel(valor) {";
  html += "  document.getElementById('txtVel').innerText = valor;";
  html += "  fetch('/velocidad?v=' + valor);";
  html += "}";
  html += "</script>";

  html += "</body></html>";
  return html;
}

// --- CONFIGURACION DEL SERVIDOR WEB ---
void configurarRutas() {
  
  // 1. Endpoint Raiz
  // Peticion: GET /
  // Devuelve: Todo el codigo HTML de la interfaz web
  server.on("/", []() {
    server.send(200, "text/html", getHTML());
  });

  // 2. Endpoint de Sensores
  // Peticion: GET /sensores
  // Devuelve: JSON con el estado en vivo de los pines
  // Ejemplo de respuesta: {"izq":1, "der":0}
  server.on("/sensores", []() {
    String json = "{\"izq\":" + String(digitalRead(SENSOR_IZQ)) + ", \"der\":" + String(digitalRead(SENSOR_DER)) + "}";
    server.send(200, "application/json", json);
  });

  // 3. Endpoint de Velocidad
  // Peticion: GET /velocidad?v=150
  // Recibe: La nueva velocidad en PWM (0-255)
  server.on("/velocidad", []() {
    if (server.hasArg("v")) {
      velocidad = server.arg("v").toInt();
      
      // Si el carro se esta moviendo en modo manual, aplicamos la velocidad de inmediato
      if (!modo_automatico) {
         if (digitalRead(IN1) == HIGH || digitalRead(IN3) == HIGH) adelante();
      }
    }
    server.send(200, "text/plain", "OK");
  });

  // 4. Endpoint de Modo de Operacion
  // Peticion: GET /modo?estado=auto   o   GET /modo?estado=manual
  // Recibe: El estado como un parametro en la URL
  // Devuelve: Redireccion HTTP 303 hacia la pagina principal "/"
  server.on("/modo", []() {
    if (server.hasArg("estado")) {
      String estado = server.arg("estado");
      if (estado == "auto") {
        modo_automatico = true;
      } else {
        modo_automatico = false;
        parar(); 
      }
    }
    server.sendHeader("Location", "/");
    server.send(303);
  });

  // 5. Endpoints de Movimiento Manual
  // Peticion: GET /adelante (o cualquiera de las otras direcciones)
  // Devuelve: JSON confirmando la accion que se acaba de ejecutar
  
  server.on("/adelante", []() { 
    modo_automatico = false; 
    adelante(); 
    server.send(200, "application/json", "{\"status\":\"OK\", \"accion\":\"adelante\"}"); 
  });
  
  server.on("/atras", []() { 
    modo_automatico = false; 
    atras(); 
    server.send(200, "application/json", "{\"status\":\"OK\", \"accion\":\"atras\"}"); 
  });
  
  server.on("/izquierda", []() { 
    modo_automatico = false; 
    izquierda(); 
    server.send(200, "application/json", "{\"status\":\"OK\", \"accion\":\"izquierda\"}"); 
  });
  
  server.on("/derecha", []() { 
    modo_automatico = false; 
    derecha(); 
    server.send(200, "application/json", "{\"status\":\"OK\", \"accion\":\"derecha\"}"); 
  });
  
  server.on("/parar", []() { 
    modo_automatico = false; 
    parar(); 
    server.send(200, "application/json", "{\"status\":\"OK\", \"accion\":\"parar\"}"); 
  });
}

void setup() {
  Serial.begin(115200);

  // Configurar pines de motores como salidas PWM (Sintaxis nueva de ESP32)
  ledcAttach(IN1, freq, resolucion);
  ledcAttach(IN2, freq, resolucion);
  ledcAttach(IN3, freq, resolucion);
  ledcAttach(IN4, freq, resolucion);

  // Configurar pines de sensores como entrada
  pinMode(SENSOR_IZQ, INPUT);
  pinMode(SENSOR_DER, INPUT);

  // Asegurar que los motores inicien apagados
  parar();

  // Iniciar WiFi en modo Access Point
  Serial.println("Iniciando Punto de Acceso...");
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Direccion IP del servidor: ");
  Serial.println(IP);

  // Configurar e iniciar servidor web
  configurarRutas();
  server.begin();
}

void loop() {
  server.handleClient(); // Mantener el servidor escuchando peticiones

  if (modo_automatico) {
    seguirLinea();
  }
}