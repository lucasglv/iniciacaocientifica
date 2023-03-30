//Incluindo as bibliotecas:
#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

String umidade;
String temperatura;

//Credenciais para conexão à rede wifi:
const char* ssid = "SEU_SSID";
const char* password = "SUA_SENHA";

#define DHTPIN 4     // pino do sensor DHT22
#define DHTTYPE DHT22   // modelo do sensor
DHT dht(DHTPIN, DHTTYPE);   // cria o objeto do sensor

#define PIR_PIN 5   // pino do sensor PIR
int pirState = LOW;   // estado inicial do sensor PIR

//é a função que vai ser chamada sempre valores do tópicos forem atualizado (SINO DO YOUTUBE)-> USAR FUTURAMENTE
void retornofuncao(const char* topico, byte* pacote, unsigned int tamanho) {  
}

// MQTT client
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
char *mqttServer = "69106c95ae32471abc22c53e76a8112d.s2.eu.hivemq.cloud";
int mqttPort = 8883;
void setupMQTT() {
  mqttClient.setServer(mqttServer, mqttPort);
// define a função de retorno de chamada
  mqttClient.setCallback(retornofuncao);
}


void reconnect() {
  Serial.println("Conectando ao MQTT Broker..."); //BROKER É O SERVIDOR DOS QUAIS OS TÓPICOS FICAM PRESENTES.
    while (!mqttClient.connected()) {
     Serial.println("Reconectando ao MQTT Broker..");
      String clientId = "ESP32_Lucas";

    if (mqttClient.connect("LucasId", "LucasIC","100senha")) {
      Serial.println("Connectado!");
      // Criação e inscrição nos tópicos:
      mqttClient.subscribe("/presenca");
      mqttClient.subscribe("/temperatura");
      mqttClient.subscribe("/umidade");
    }
  }
}


void setup() {
  Serial.begin(9600);
  pinMode(PIR_PIN, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a rede WiFi...");
  }
  Serial.println("Conectado a rede WiFi!");

  dht.begin();
  
  setupMQTT();//Chamando a configuração para conexão 
}

void loop(){

  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  pirState = digitalRead(PIR_PIN);
  if (pirState == HIGH) {
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    if (isnan(temp) || isnan(hum)) {
      Serial.println("Erro ao ler o sensor DHT22!");
      return;
    }

    Serial.print("Temperatura: ");
    Serial.print(temp);
    Serial.print("C / Umidade: ");
    Serial.print(hum);
    Serial.println("%");
    umidade = String(hum);
    mqttClient.publish("/umidade", umidade.c_str());
    temperatura = String(temp);
    mqttClient.publish("/temperatura", temperatura.c_str());

  }
  delay(500);   // aguarda 500ms antes de verificar novamente o sensor PIR
}
