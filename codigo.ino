#include <WiFi.h>
#include <PubSubClient.h>

// Configurações de Wi-Fi
const char* ssid = "SEU_WIFI";
const char* password = "SENHA_WIFI";

// Configurações MQTT
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// Tópicos MQTT
const char* topicPublish = "senai/esp32/dados";
const char* topicSubscribe = "senai/esp32/led";

// LED
const int ledPin = 2;  // LED onboard do ESP32

// Função callback ao receber mensagens do MQTT
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);
  
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  Serial.print("Conteúdo: ");
  Serial.println(msg);

  if (msg.equalsIgnoreCase("Acender")) {
    digitalWrite(ledPin, HIGH);
    Serial.println("LED LIGADO");
  } else if (msg.equalsIgnoreCase("Apagar")) {
    digitalWrite(ledPin, LOW);
    Serial.println("LED DESLIGADO");
  }
}

// Reconexão MQTT caso caia
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.println("Reconectando MQTT...");
    if (client.connect("esp32Client")) {
      Serial.println("Conectado ao broker");
      client.subscribe(topicSubscribe);
      Serial.println("Inscrito no tópico de controle");
    } else {
      Serial.print("Falha. Estado: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Conexão ao Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConectado ao Wi-Fi!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallback);
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  static unsigned long lastMsg = 0;
  unsigned long now = millis();

  // Publicação periódica
  if (now - lastMsg > 3000) {
    lastMsg = now;
    String message = "ESP32 funcionando";
    client.publish(topicPublish, message.c_str());
    Serial.println("Mensagem publicada: " + message);
  }
}
