# 🏫 Monitor de Aula Conectado (IoT)

Proyecto integrador para la Semana 12. Implementación de un sistema IoT funcional para publicar datos ambientales de un aula hacia Adafruit IO y controlar actuadores remotamente mediante el protocolo MQTT.

**Autores:** Santiago y Sebastián (Unidades Tecnológicas de Santander - UTS)

## 🏗️ Arquitectura del Sistema
El sistema captura datos físicos a través de sensores, los procesa localmente en un microcontrolador y los publica en un broker MQTT en la nube. A su vez, un dashboard web bidireccional permite la visualización en tiempo real y el envío de comandos hacia los actuadores físicos.

```text
[DHT11 / LDR] ───Lectura───► [ ESP8266 ] ──MQTT──► [ Adafruit IO Broker ]
                                 ▲                        │
                                 │                        ▼
[LEDs / Buzzer] ◄──Control───┴───────────────── [ Dashboard Web/Móvil ]
