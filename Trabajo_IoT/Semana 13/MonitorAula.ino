/*
 * Sistema de Monitoreo de Aula (Semana 12 + Semana 13)
 * Hardware: ESP8266 + DHT11 + LDR + LEDs + Buzzer
 * Integración: Adafruit IO + IFTTT (Alertas Push)
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
// REQUISITO S13: Intervalo de 10 a 15 segundos para IFTTT
const unsigned long INTERVALO_ENVIO = 12000; 

// --- DECLARACIÓN DE FUNCIONES ---
void conectarWiFi();
void reconnectMQTT();
void publicarDatos();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  digitalWrite(LED_ROJO, LOW);
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AMARILLO, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  dht.begin();
  conectarWiFi();
  
  client.setServer(AIO_SERVER, AIO_PORT);
  client.setCallback(callback); 
  Serial.println("[OK] Sistema Integrado S12+S13 Iniciado");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n[ERROR] ¡Conexión WiFi perdida!");
    conectarWiFi(); 
  }
  
  if (!client.connected() && WiFi.status() == WL_CONNECTED) {
    reconnectMQTT();
  }
  
  client.loop();

  if (millis() - ultimoEnvio > INTERVALO_ENVIO) {
    ultimoEnvio = millis();
    publicarDatos();
  }
}

// --- FUNCIÓN DE RECEPCIÓN DE DATOS (CONTROL REMOTO S12) ---
void callback(char* topic, byte* payload, unsigned int length) {
  String mensaje;
  for (unsigned int i = 0; i < length; i++) {
    mensaje += (char)payload[i];
  }
  
  if (String(topic) == topic_led) {
    if (mensaje == "1" || mensaje == "ON") digitalWrite(LED_ROJO, HIGH);
    else digitalWrite(LED_ROJO, LOW);
  }
  
  if (String(topic) == topic_buzzer) {
    if (mensaje == "1" || mensaje == "ON") digitalWrite(BUZZER_PIN, HIGH);
    else digitalWrite(BUZZER_PIN, LOW);
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
  Serial.println("\n[OK] WiFi conectado.");
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando a Adafruit IO...");
    String clientId = "ESP8266Client-" + String(ESP.getChipId());
    
    if (client.connect(clientId.c_str(), AIO_USERNAME, AIO_KEY)) {
      Serial.println("[OK]");
      client.subscribe(topic_led.c_str());
      client.subscribe(topic_buzzer.c_str());
    } else {
      Serial.print("[FALLO] rc=");
      Serial.print(client.state());
      Serial.println(" -> Intentando en 5 segundos");
      delay(5000);
    }
  }
}

// --- ENVÍO DE DATOS Y LECTURA LDR PARA IFTTT (S13) ---
void publicarDatos() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  
  // Leemos el pin analógico. 
  // Restamos de 1024 para invertirlo: Así "oscuridad" será cercano a 0, y "luz" será alto.
  int l_crudo = analogRead(LDR_PIN);
  int l_invertido = 1024 - l_crudo; 

  // Publicamos el LDR independientemente de si el DHT11 falla, para no afectar a IFTTT
  client.publish(topic_luz.c_str(), String(l_invertido).c_str());

  if (!isnan(t) && !isnan(h)) {
    client.publish(topic_temp.c_str(), String(t, 1).c_str());
    client.publish(topic_hum.c_str(), String(h, 1).c_str());
  }

  Serial.println("--- Datos enviados a Adafruit IO ---");
  Serial.print("Temp: "); Serial.print(t); Serial.println(" C");
  Serial.print("Hum:  "); Serial.print(h); Serial.println(" %");
  Serial.print("Luz:  "); Serial.print(l_invertido); Serial.println(" (Invertido para IFTTT)");
}