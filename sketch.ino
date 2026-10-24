#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <time.h>


const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";

const char* MQTT_HOST = "test.mosquitto.org"; 

// Colocar o IP  da VM com o FIWARE no lugar do "AAA" (porta 1883).
// Para teste sem FIWARE use o "broker.hivemq.com".

const int   MQTT_PORT = 1883;

const char* DEVICE_ID = "agnelloiris"; 
const char* API_KEY   = "TEF";

// Faça o subscribe em /TEF/agnelloiris/attrs.
// Para dar comandos é só ir em publish, /TEF/agnelloiris/cmd em "Topic" e mandar agnelloiris@on ou agnelloiris@off em "Payload".


String TOPIC_ATTRS  = String("/") + API_KEY + "/" + DEVICE_ID + "/attrs";
String TOPIC_CMD    = String("/") + API_KEY + "/" + DEVICE_ID + "/cmd";
String TOPIC_CMDEXE = String("/") + API_KEY + "/" + DEVICE_ID + "/cmdexe";


#define DHT_PIN  15
#define LDR_PIN  34
#define LED_PIN  26
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);
WiFiClient espClient;
PubSubClient mqtt(espClient);

unsigned long lastSend = 0;
const unsigned long SEND_INTERVAL = 5000;


String getTimestamp() {
  struct tm t;
  if (!getLocalTime(&t, 2000)) return "";
  char buf[25];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &t);
  return String(buf);
}


void connectWiFi() {
  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASS, 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado! IP: " + WiFi.localIP().toString());
}


void onMessage(char* topic, byte* payload, unsigned int len) {
  String msg;
  for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];
  Serial.println("Comando recebido [" + String(topic) + "]: " + msg);

 
  String result = "";
  if (msg.indexOf("@on") >= 0) {
    digitalWrite(LED_PIN, HIGH);
    result = String(DEVICE_ID) + "@on|LED ligado";
  } else if (msg.indexOf("@off") >= 0) {
    digitalWrite(LED_PIN, LOW);
    result = String(DEVICE_ID) + "@off|LED desligado";
  }
  if (result.length()) mqtt.publish(TOPIC_CMDEXE.c_str(), result.c_str());
}

void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Conectando ao broker MQTT...");
    String clientId = String(DEVICE_ID) + "-" + String(random(0xffff), HEX);
    if (mqtt.connect(clientId.c_str())) {
      Serial.println(" conectado!");
      mqtt.subscribe(TOPIC_CMD.c_str());
    } else {
      Serial.println(" falhou, rc=" + String(mqtt.state()) + ". Tentando em 3s");
      delay(3000);
    }
  }
}


void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  dht.begin();
  analogReadResolution(12);

  connectWiFi();
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMessage);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  if (millis() - lastSend >= SEND_INTERVAL) {
    lastSend = millis();

    float temp = dht.readTemperature();
    float hum  = dht.readHumidity();
    int ldrRaw = analogRead(LDR_PIN);
    const float GAMMA = 0.7;
    const float RL10 = 50.0;

    float voltage = ldrRaw * 3.3 / 4095.0;
    float resistance = 10000.0 * voltage / (3.3 - voltage);
    float lum = pow((RL10 * 1000.0 * pow(10.0, GAMMA)) / resistance,
                    1.0 / GAMMA);
    String ts = getTimestamp();

    if (isnan(temp) || isnan(hum)) {
      Serial.println("Falha na leitura do DHT22");
      return;
    }


    String payload = "t|" + String(temp, 1) +
                     "|h|" + String(hum, 1) +
                     "|l|" + String(lum) +
                     "|TimeInstant|" + ts;

    mqtt.publish(TOPIC_ATTRS.c_str(), payload.c_str());
    Serial.println("Publicado em " + TOPIC_ATTRS + ": " + payload);
  }
}


//  Checkpoint 05 - Vinheria Agnello (IoT)
//  ESP32 + DHT22 (temperatura/umidade) + LDR (luminosidade)
//  Envia dados via MQTT para o IoT Agent do FIWARE,
//  com timestamp (TimeInstant) em cada leitura.
//  Recebe comandos pelo MyMQTT para ligar/desligar um LED.
//  RM: 570322
//  RM: 570979
//  RM: 569136
//  RM: 571942
