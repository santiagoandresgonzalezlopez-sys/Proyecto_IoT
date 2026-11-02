/*
 * Sistema de Monitoreo de Aula - MQTT Adafruit IO
 * Hardware: ESP8266 + DHT11 + LDR + LED Rojo, Verde, Amarillo + Buzzer
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include "config.h"

// --- CONFIGURACIÓN DE SENSORES Y ACTUADORES ---
#define DHT_PIN 4         // Pin D2 en NodeMCU
#define DHT_TYPE DHT11
#define LDR_PIN A0        // Pin A0 en NodeMCU
#define LED_ROJO 5        // Pin D1 en NodeMCU
#define LED_VERDE 0       // Pin D3 en NodeMCU
#define LED_AMARILLO 2    // Pin D4 en NodeMCU
#define BUZZER_PIN 14     // Pin D5 en NodeMCU

// --- TÓPICOS MQTT (Jerarquía Adafruit IO) ---
String topic_temp   = String(AIO_USERNAME) + "/feeds/temperatura";
String topic_hum    = String(AIO_USERNAME) + "/feeds/humedad";
String topic_luz    = String(AIO_USERNAME) + "/feeds/luminosidad";
String topic_led    = String(AIO_USERNAME) + "/feeds/led-rojo";
String topic_buzzer = String(AIO_USERNAME) + "/feeds/buzzer";

DHT dht(DHT_PIN, DHT_TYPE);
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long ultimoEnvio = 0;
const unsigned long INTERVALO_ENVIO = 10000; // Enviar cada 10 segundos

// --- DECLARACIÓN DE FUNCIONES ---
void conectarWiFi();
void reconnectMQTT();
void publicarDatos();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Configuración de pines físicos de salida
  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Apagar todo al iniciar
  digitalWrite(LED_ROJO, LOW);
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AMARILLO, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  dht.begin();
  conectarWiFi();
  
  client.setServer(AIO_SERVER, AIO_PORT);
  client.setCallback(callback); // Asigna la función que escucha comandos remotos
  Serial.println("[OK] Sistema Iniciado");
}

void loop() {
  // 1. Verificar conexión WiFi constantemente
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n[ERROR] ¡Conexión WiFi perdida!");
    Serial.println("Intentando reconectar a la red...");
    conectarWiFi(); 
  }
  
  // 2. Verificar conexión con el broker MQTT
  if (!client.connected() && WiFi.status() == WL_CONNECTED) {
    reconnectMQTT();
  }
  
  // Mantener viva la conexión MQTT para escuchar suscripciones
  client.loop();

  // 3. Enviar datos cada 5 segundos
  if (millis() - ultimoEnvio > INTERVALO_ENVIO) {
    ultimoEnvio = millis();
    publicarDatos();
  }
}

// --- FUNCIÓN DE RECEPCIÓN DE DATOS (DASHBOARD -> ESP8266) ---
void callback(char* topic, byte* payload, unsigned int length) {
  String mensaje;
  for (unsigned int i = 0; i < length; i++) {
    mensaje += (char)payload[i];
  }
  
  Serial.print("Comando recibido en [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(mensaje);

  // Lógica de encendido/apagado para el LED ROJO
  if (String(topic) == topic_led) {
    if (mensaje == "1" || mensaje == "ON") {
      digitalWrite(LED_ROJO, HIGH);
      Serial.println("-> LED Rojo: ENCENDIDO");
    } else {
      digitalWrite(LED_ROJO, LOW);
      Serial.println("-> LED Rojo: APAGADO");
    }
  }
  
  // Lógica de encendido/apagado para el Buzzer
  if (String(topic) == topic_buzzer) {
    if (mensaje == "1" || mensaje == "ON") {
      digitalWrite(BUZZER_PIN, HIGH);
      Serial.println("-> Buzzer: ENCENDIDO");
    } else {
      digitalWrite(BUZZER_PIN, LOW);
      Serial.println("-> Buzzer: APAGADO");
    }
  }
}

void conectarWiFi() {
  Serial.print("\nConectando a WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[OK] WiFi conectado. IP: " + WiFi.localIP().toString());
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando al broker Adafruit IO...");
    String clientId = "ESP8266Client-" + String(ESP.getChipId());
    
    // Autenticación obligatoria para Adafruit
    if (client.connect(clientId.c_str(), AIO_USERNAME, AIO_KEY)) {
      Serial.println("[OK]");
      // Suscribirse a los tópicos de actuadores una vez conectados
      client.subscribe(topic_led.c_str());
      client.subscribe(topic_buzzer.c_str());
    } else {
      Serial.print("[FALLO] rc=");
      Serial.print(client.state());
      Serial.println(" -> Intentando en 2 segundos");
      delay(2000);
    }
  }
}

void publicarDatos() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  int l = analogRead(LDR_PIN);

  if (isnan(t) || isnan(h)) {
    Serial.println("[ERROR] Fallo lectura DHT11");
    return;
  }

  // Publicar en Adafruit IO
  client.publish(topic_temp.c_str(), String(t, 1).c_str());
  client.publish(topic_hum.c_str(), String(h, 1).c_str());
  client.publish(topic_luz.c_str(), String(l).c_str());

  Serial.println("--- Datos publicados en Adafruit IO ---");
  Serial.print("Temp: "); Serial.print(t); Serial.println(" C");
  Serial.print("Hum:  "); Serial.print(h); Serial.println(" %");
  Serial.print("Luz:  "); Serial.println(l);
}