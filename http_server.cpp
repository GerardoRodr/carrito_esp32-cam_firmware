#include "http_server.h"
#include <Arduino.h>
#include "esp_http_server.h"
#include "esp_camera.h"
#include "motor_control.h"

// Variables para los servidores HTTP
httpd_handle_t stream_httpd = NULL;
httpd_handle_t control_httpd = NULL;

// --- CONFIGURACIÓN DE STREAMING MJPEG ---
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static esp_err_t stream_handler(httpd_req_t *req) {
    Serial.println("[HTTP] Cliente conectado al stream de video (/stream)");
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len;
    uint8_t * _jpg_buf;
    char * part_buf[64];

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK) {
        return res;
    }
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    while (true) {
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
            res = ESP_FAIL;
            break;
        } else {
            // El ESP32-CAM retorna JPEG directamente si la configuración pixel_format es PIXFORMAT_JPEG
            _jpg_buf_len = fb->len;
            _jpg_buf = fb->buf;
        }

        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if (res == ESP_OK) {
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        
        if (fb) {
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        } else if (_jpg_buf) {
            free(_jpg_buf);
            _jpg_buf = NULL;
        }

        if (res != ESP_OK) {
            break;
        }
    }
    return res;
}

// --- HANDLERS DE CONTROL DE MOTORES ---
static esp_err_t cmd_adelante_handler(httpd_req_t *req) {
    Serial.println("[HTTP] Comando recibido: /adelante");
    adelante();
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
}

static esp_err_t cmd_atras_handler(httpd_req_t *req) {
    Serial.println("[HTTP] Comando recibido: /atras");
    atras();
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
}

static esp_err_t cmd_izquierda_handler(httpd_req_t *req) {
    Serial.println("[HTTP] Comando recibido: /izquierda");
    izquierda();
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
}

static esp_err_t cmd_derecha_handler(httpd_req_t *req) {
    Serial.println("[HTTP] Comando recibido: /derecha");
    derecha();
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
}

static esp_err_t cmd_parar_handler(httpd_req_t *req) {
    Serial.println("[HTTP] Comando recibido: /parar");
    parar();
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
}

static esp_err_t cmd_velocidad_handler(httpd_req_t *req) {
    size_t buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        char *buf = (char *)malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            char value[16];
            if (httpd_query_key_value(buf, "v", value, sizeof(value)) == ESP_OK) {
                int val = atoi(value);
                Serial.print("[HTTP] Cambio de velocidad recibido: ");
                Serial.println(val);
                setVelocidad(val);
            }
        }
        free(buf);
    }
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
}

// --- CONFIGURACIÓN DE LOS SERVIDORES ---
void startServers() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    
    // 1. INICIAR SERVIDOR DE CONTROL (PUERTO 80)
    config.server_port = 80;
    config.ctrl_port = 80;
    if (httpd_start(&control_httpd, &config) == ESP_OK) {
        httpd_uri_t uri_adelante = { .uri = "/adelante", .method = HTTP_GET, .handler = cmd_adelante_handler, .user_ctx = NULL };
        httpd_uri_t uri_atras = { .uri = "/atras", .method = HTTP_GET, .handler = cmd_atras_handler, .user_ctx = NULL };
        httpd_uri_t uri_izquierda = { .uri = "/izquierda", .method = HTTP_GET, .handler = cmd_izquierda_handler, .user_ctx = NULL };
        httpd_uri_t uri_derecha = { .uri = "/derecha", .method = HTTP_GET, .handler = cmd_derecha_handler, .user_ctx = NULL };
        httpd_uri_t uri_parar = { .uri = "/parar", .method = HTTP_GET, .handler = cmd_parar_handler, .user_ctx = NULL };
        httpd_uri_t uri_velocidad = { .uri = "/velocidad", .method = HTTP_GET, .handler = cmd_velocidad_handler, .user_ctx = NULL };

        httpd_register_uri_handler(control_httpd, &uri_adelante);
        httpd_register_uri_handler(control_httpd, &uri_atras);
        httpd_register_uri_handler(control_httpd, &uri_izquierda);
        httpd_register_uri_handler(control_httpd, &uri_derecha);
        httpd_register_uri_handler(control_httpd, &uri_parar);
        httpd_register_uri_handler(control_httpd, &uri_velocidad);
        
        Serial.println("Servidor de control iniciado en el puerto 80");
    }

    // 2. INICIAR SERVIDOR DE STREAMING (PUERTO 81)
    config.server_port = 81;
    config.ctrl_port = 81;
    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_uri_t uri_stream = { .uri = "/stream", .method = HTTP_GET, .handler = stream_handler, .user_ctx = NULL };
        httpd_register_uri_handler(stream_httpd, &uri_stream);
        
        Serial.println("Servidor de streaming iniciado en el puerto 81");
        Serial.println("Usa http://<IP>:81/stream para ver la cámara");
    }
}
