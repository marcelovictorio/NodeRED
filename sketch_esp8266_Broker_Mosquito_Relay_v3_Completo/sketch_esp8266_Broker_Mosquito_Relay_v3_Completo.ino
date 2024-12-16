/**
 Marcelo Costa - 01-10-2024
 Controle placa ESP-01 Relay v4.0 TB-IOTMCU
 -> Selecionar placa Generic ESP8266 Module
*/
//#include <WiFi.h> // Biblioteca para a conexão WiFi
#include <ESP8266WiFi.h>          // Biblioteca para a conexão WiFi 8266
#include <WiFiManager.h>          // Incluindo WiFiManager versão 2.0.16-rc.2 ok
#include <PubSubClient.h> // Biblioteca para o MQTT
//#include <ESP8266WebServer.h>     // Para criar servidores web

// Configurações Wi-Fi
const char* ssid = "INTELBRAS"; // Insira o nome da sua rede Wi-Fi
const char* password = "12345678"; // Insira a senha da sua rede Wi-Fi
char mqtt_server[] = "192.168.15.8"; // IP do broker MQTT Mosquitto (Parametrizada)
const int mqtt_port = 1883; // Porta padrão do MQTT
const char* mqtt_topic = "/topico/relay"; // Tópico MQTT pode ser alterado
int relayPin = 0; // Pino GPIO 0/2

WiFiClient espClient;
PubSubClient client(espClient);

// Função para conectar ao Wi-Fi - WiFiManager solicita as credenciais na primeira vez
void setup_wifiManager() {

  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 15);

  WiFiManager wifiManager; // Acesso AP IP 192.168.4.1
  // Reseta as configurações de WiFi salvas (Manter comentado para testes)
  //wifiManager.resetSettings();

  // Parâmetro customizado para o servidor MQTT
  wifiManager.addParameter(&custom_mqtt_server);


  // Modo automático para conectar ou abrir o portal para configuração
  if (!wifiManager.autoConnect("AutoConnectAP","12345678")) {
    Serial.println("Falha ao conectar. Resetando...");
    //ESP.reset();
    delay(3000);
  }
  
  String ipString = custom_mqtt_server.getValue();    // Obter o valor como string
  IPAddress ipAddress;   // Convert the string to an IP address
  ipAddress.fromString(ipString);
  Serial.print("ipAddress.fromString: ");
  Serial.println(ipString);

  // Check if the IP address is valid
  if (!ipAddress.isValid(ipString)) {
    Serial.println("Invalid IP address");
    // Handle the error, e.g., restart the device
    //ESP.reset();
  } else {      
    strcpy(mqtt_server, ipString.c_str()); // Use the valid IP address   
  }
  
  // Caso conecte com sucesso
  Serial.print("Conectado à rede! IP: ");
  Serial.println(mqtt_server);
}

// Função de callback para receber mensagens MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico: ");
  Serial.print(mqtt_topic);
  Serial.print("/");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  Serial.print(". Valor: ");
  Serial.println((char)payload[0]);
  Serial.println();
  
  //==========================================
  // Verifica se o comando é válido
  if ((char)payload[0] == 'L' || (char)payload[0] == 'H') {
      if ((char)payload[0] == 'L') {
        digitalWrite(relayPin, LOW);
        Serial.println("Relay off");
      } else {
        digitalWrite(relayPin, HIGH);
        Serial.println("Relay on");
      }
    } else {
      Serial.println("Invalid command");
    }
  //===========================================
}

// Função para conectar ao broker MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando se conectar ao broker MQTT...");
    // Tentativa de conexão
    if (client.connect("ESP32")) {
      Serial.println("Conectado");
      // Inscrição no tópico "teste"
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("Falha na conexão, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin , HIGH);
  delay(3000);
  digitalWrite(relayPin , LOW);
  
  setup_wifiManager();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  delay(500);
}
