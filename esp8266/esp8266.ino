/*
**************************PINAGEM****************************************************************************
  
  RFID-RC522   Wemos          Nodemcu
RST/Reset RST  D1  [1*]        D1 [1*]      GPIO-5  [1*]
SPI SS  SDA[3] D2  [2*]        D2 [2*]      GPIO-4 [2*]
SPI MOSI MOSI  D7              D7           GPIO-13
SPI MISO MISO  D6              D6           GPIO-12
SPI SCK SCK    D5              D5           GPIO-14

[1*] (1, 2) Configuracao tipica definindo como RST_PIN no sketch / programa.
[2*] (1, 2) Configuracao, tipica definindo como SS_PIN no sketch / programa.
[3*] O pino SDA pode ser rotulado como SS em algumas placas de MFRC522 / mais antigas, significam a mesma coisa.
******************************************************************************************************************
 */

// LIBRARYS
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>

// DEFINES
#define RST_PIN 5
#define SS_PIN 4
#define LED D3

// VARIABLES
const char* SSID = "YOUR_SSID"; // rede wifi
const char* PASSWORD = "YOUR_PASS"; // senha da rede wifi

const char* BROKER_MQTT = "host_do_broker ou host_do_docker"; // ip/host do broker
int BROKER_PORT = 1883; // porta do broker

const char* TOPIC_PING = "/rfid/entrada/ping";

const char* TOPIC_PONG = "/rfid/entrada/pong";

// PROTOTYPES
void initPins();
void initSerial();
void initRfid();
void initWiFi();
void initMQTT();

// OBJECTS
WiFiClient client;
PubSubClient MQTT(client); // instancia o mqtt
MFRC522 mfrc522(SS_PIN, RST_PIN); // instancia o rfid

// setup
void setup() {
  initSerial();
  initWiFi();
  initMQTT();
  initRfid();
  pinMode(LED,OUTPUT);
}

void loop() {
  if (!MQTT.connected()) {
    reconnectMQTT();
  }
  recconectWiFi();
  MQTT.loop();

  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    delay(500);
    return;
  }

  if ( ! mfrc522.PICC_ReadCardSerial()) {
    delay(500);
    return;
  }

  rfidProcess();

}

// implementacao dos prototypes

void initSerial() {
  Serial.begin(115200);
}

void initRfid() {
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Aproxime o seu cartao do leitor...");
  Serial.println();
}

void initWiFi() {
  delay(10);
  Serial.println("Conectando-se em: " + String(SSID));

  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado na Rede " + String(SSID) + " | IP => ");
  Serial.println(WiFi.localIP());
}

// Funcão para se conectar ao Broker MQTT
void initMQTT() {
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(mqtt_callback);
}

//Função que recebe as mensagens publicadas
void mqtt_callback(char* topic, byte* payload, unsigned int length) {

  String message;
  for (int i = 0; i < length; i++) {
    char c = (char)payload[i];
    message += c;
  }
  Serial.println("Tópico => " + String(topic) + " | Valor => " + String(message));

Serial.println(String(message));

if (String(message) == "1") //UID 1 - Cartao
  {
    Serial.println("Liberado !");
    Serial.println();
    digitalWrite(LED, LOW);     // LIGA LED OU/ ativa rele, abre trava solenoide
    delay(3000);              // DELAY /espera 3 segundos
    digitalWrite(LED, HIGH);  // DESlIGA LED OU /desativa rele, fecha  trava solenoide
  }

  Serial.flush();
}

void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.println("Tentando se conectar ao Broker MQTT: " + String(BROKER_MQTT));
    if (MQTT.connect("ESP8266-ESP12-E")) {
      Serial.println("Conectado");
      MQTT.subscribe("/rfid/entrada/pong");

    } else {
      Serial.println("Falha ao Reconectar");
      Serial.println("Tentando se reconectar em 2 segundos");
      delay(2000);
    }
  }
}

void recconectWiFi() {
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
}

void rfidProcess()
{
  Serial.print("UID da tag : ");
  String conteudo = "";
  byte letra;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  char UUID[9];
  conteudo.toCharArray(UUID, 9);
  Serial.println(conteudo);
  MQTT.publish(TOPIC_PING, UUID);


}
