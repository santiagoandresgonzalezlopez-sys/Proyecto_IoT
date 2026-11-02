# 🏫 Monitor de Aula Conectado (IoT)

Proyecto integrador para la Semana 12. Implementación de un sistema IoT funcional para publicar datos ambientales de un aula hacia Adafruit IO y controlar actuadores remotamente mediante el protocolo MQTT.

**Autores:** Santiago y Sebastián (Unidades Tecnológicas de Santander - UTS)

## 🏗️ Arquitectura del Sistema
El sistema captura datos físicos a través de sensores, los procesa localmente en un microcontrolador y los publica en un broker MQTT en la nube. A su vez, un dashboard web bidireccional permite la visualización en tiempo real y el envío de comandos hacia los actuadores físicos.

[DHT11 / LDR] ───Lectura───► [ ESP8266 ] ──MQTT──► [ Adafruit IO Broker ]
                                 ▲                        │
                                 │                        ▼
[LEDs / Buzzer] ◄──Control───┴───────────────── [ Dashboard Web/Móvil ]


⚙️ Hardware Utilizado y Conexión de Pines
Microcontrolador: ESP8266 (NodeMCU / Wemos D1 Mini)

Sensor de Temperatura y Humedad DHT11: Pin D2 (GPIO 4)

Fotorresistencia LDR: Pin A0 (Entrada analógica)

LED Rojo (Actuador remoto): Pin D1 (GPIO 5)

LED Verde: Pin D3 (GPIO 0)

LED Amarillo: Pin D4 (GPIO 2)

Buzzer (Actuador remoto): Pin D5 (GPIO 14)

🚀 Instrucciones de Despliegue
Clonar este repositorio localmente.

Abrir el archivo MonitorAulaESP8266.ino en Arduino IDE.

Instalar las librerías necesarias desde el Gestor de Librerías:

PubSubClient por Nick O'Leary.

DHT sensor library por Adafruit.

Crear o renombrar el archivo config.h.example a config.h e introducir:

Credenciales de la red WiFi local (SSID y Password).

Credenciales de Adafruit IO (Username y Active Key).

Compilar y subir el código a la placa ESP8266.

Importar o crear los widgets correspondientes en el Dashboard de Adafruit IO utilizando los feeds: temperatura, humedad, luminosidad, led-rojo y buzzer.
