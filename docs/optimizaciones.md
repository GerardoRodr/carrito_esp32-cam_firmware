# Optimizaciones de Latencia (ESP32-CAM)

Este documento detalla las técnicas avanzadas implementadas en el firmware del ESP32-CAM para garantizar que la transmisión de video (pesada) no interfiera con el control de movimiento de los motores (tiempo real).

## 1. Desactivación de Modem Sleep (Wi-Fi)
Por defecto, el ESP32 en modo Station habilita el ahorro de energía "Modem Sleep", apagando la antena de radio periódicamente entre balizas (beacons). Esto puede generar retrasos intermitentes y erráticos de hasta 300ms al recibir peticiones HTTP.
* **Solución Implementada:** Se añadió `WiFi.setSleep(false);` al inicio de la conexión. Esto fuerza a la radio Wi-Fi a estar activa el 100% del tiempo. Aunque aumenta el consumo de batería ligeramente, es el paso más crítico para asegurar latencias súper estables (< 20ms) en la red local.

## 2. Tiempos de Respiro en el Streaming (Yielding FreeRTOS)
El servidor de streaming envía fotogramas JPEG mediante un bucle infinito `while(true)`.
* **Problema:** Un bucle infinito ininterrumpido acapara el procesador y ahoga el chip de red interno, encolando y bloqueando cualquier otra petición HTTP entrante.
* **Solución Implementada:** Se insertó un `vTaskDelay(pdMS_TO_TICKS(15));` tras enviar cada frame de video al cliente. Estos 15ms de "descanso" (que son imperceptibles en el flujo del video) obligan al planificador de FreeRTOS a ceder el procesador. En ese lapso, el servidor de control (puerto 80) puede interceptar y despachar rápidamente comandos de movimiento de forma asíncrona.

## 3. Prioridad y Afinidad de Núcleos (Multi-Core)
El ESP32 es un microcontrolador de doble núcleo. Se reescribió la estructura `httpd_config_t` para dividir inteligentemente la carga de trabajo entre ambos:
* **Servidor de Control de Motores (Puerto 80):** 
  * **Prioridad:** Alta (`tskIDLE_PRIORITY + 6`)
  * **Núcleo Asignado:** Core 1 (El núcleo de aplicaciones, donde corre el código principal de Arduino. Está más libre).
* **Servidor de Video (Puerto 81):** 
  * **Prioridad:** Baja (`tskIDLE_PRIORITY + 4`)
  * **Núcleo Asignado:** Core 0 (El núcleo del protocolo, encargado del Wi-Fi y tareas pesadas).

Esta configuración estricta le asegura al sistema operativo interno que los endpoints como `/adelante` tienen el "paso preferencial" absoluto por encima de cualquier fotografía que esté en proceso de compresión o envío.

## 4. Consejos para el Cliente (Reutilización TCP)
Dado que el ESP32 ha sido optimizado al límite físico, el cliente (Frontend / Backend Python) debe hacer su parte. En el protocolo HTTP convencional, iniciar y destruir una conexión TCP (TCP Handshake de 3 vías) puede demorar más de 100ms.
* **Backend en Python (`requests`):** No utilices llamadas aisladas como `requests.get()`. Utiliza instancias globales como `sesion = requests.Session()` y realiza las llamadas mediante `sesion.get()`. Esto mantiene la tubería TCP viva (Keep-Alive).
* **Frontend Web (JavaScript):** En la llamada nativa de JS, asegúrate de activar el re-uso de sockets pasándole la bandera `{ keepalive: true }` a la función `fetch()`. Ejemplo: `fetch(url, { keepalive: true })`.
