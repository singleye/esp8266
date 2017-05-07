#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define BAUD_RATE               115200
#define LED_PIN                 16
#define RECV_BUFFER_LEN         128
#define SEND_BUFFER_LEN         128
#define TOPIC_LEN               128
#define CLIENT_ID_LEN           64
#define UPLINK_INTERVAL         10000
#define BLINK_QUICKLY           20
#define BLINK_SLOWLY            1000


const char *server = "your_mqtt_server";
const uint16_t port = 1883;
const char *wifi_ssid = "your_wifi_ssid";
const char *wifi_pwd = "your_wifi_password";

char downlink_topic[TOPIC_LEN];
char uplink_topic[TOPIC_LEN];
char recv_buffer[RECV_BUFFER_LEN];
char send_buffer[SEND_BUFFER_LEN];
unsigned long last_uplink_tick = 0;

bool mqtt_connected = false;
WiFiClient wifi_client;
PubSubClient mq_client(server, port, wifi_client);


void blink(unsigned long interval)
{
  int half_interval = 0;

  if (interval <= 0) {
    half_interval = 500;
  }else {
    half_interval = interval/2;
  }
  digitalWrite(LED_PIN, LOW);
  delay(half_interval);
  digitalWrite(LED_PIN, HIGH);
  delay(half_interval);
}

/*
 * Command format: command#parameter
 */
void parse_cmd(char *buffer)
{
  char *start = NULL, *word = NULL;
  int blink_time = 0, i = 0;

  if (buffer == NULL)
    return;

  word = strtok(buffer, "#");
  if (word == NULL) {
    return;
  }

  // Execute command like "blink#3" or "#blink#3" or "#blink#3#"
  if (strcmp(word, "blink") == 0) {
    Serial.println("Received blink command");
    word = strtok(NULL, "#");
    if (word != NULL) {
      blink_time = atoi(word);
      for (i=0; i<blink_time; i++) {
        blink(BLINK_SLOWLY);
      }
    }
  }else {
    Serial.printf("Received unknown command: %s\n", word);
  }
}

void mqtt_callback(char *topic, uint8_t* buffer, unsigned int len) {
  blink(BLINK_QUICKLY);
  memset(recv_buffer, 0, RECV_BUFFER_LEN);
  strncpy(recv_buffer, (char *)buffer, (RECV_BUFFER_LEN<len ? RECV_BUFFER_LEN-1:len));
  Serial.printf("Received [topic:%s]:%s\n", topic, (char*)recv_buffer);
  parse_cmd((char *)recv_buffer);
}

bool setup_mqtt_connection()
{
  char client_id[CLIENT_ID_LEN];

  snprintf(client_id, CLIENT_ID_LEN, "%s", WiFi.macAddress().c_str());
  Serial.printf("Client[%s] is connecting MQTT server!\n", client_id);
  mqtt_connected = mq_client.connect(client_id);
  if (!mqtt_connected) {
    Serial.println("MQTT connection failure");
    return false;
  }

  mq_client.setCallback(mqtt_callback);

  memset(uplink_topic, 0, TOPIC_LEN);
  snprintf(uplink_topic, TOPIC_LEN, "%s/uplink", WiFi.macAddress().c_str());
  
  memset(downlink_topic, 0, TOPIC_LEN);
  snprintf(downlink_topic, TOPIC_LEN, "%s/downlink", WiFi.macAddress().c_str());
  Serial.printf("Subscribing to topic: %s\n", downlink_topic);
  mqtt_connected = mq_client.subscribe(downlink_topic);
  if (!mqtt_connected) {
    Serial.println("MQTT connection failure");
    return false;
  }

  Serial.println("MQTT connection is setup");
  return true;
}

void setup() {
  Serial.println("Board setup()");

  Serial.begin(BAUD_RATE);
  pinMode(LED_PIN, OUTPUT);

  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true);
  WiFi.begin(wifi_ssid, wifi_pwd);

  Serial.printf("Connecting to AP(%s), password(%s)\n", wifi_ssid, wifi_pwd);
  while (WL_CONNECTED != WiFi.status()) {
    Serial.print(".");
    blink(BLINK_SLOWLY);
  }
  Serial.printf("\nWifi connection is setup!\n");
  Serial.printf("MAC: %s, IP: %s\n", WiFi.macAddress().c_str(), WiFi.localIP().toString().c_str());

  while (!setup_mqtt_connection()) {
    Serial.print(".");
    blink(BLINK_SLOWLY);
  }
}

void loop() {
  // send message every 10 second
  if (millis() - last_uplink_tick >= UPLINK_INTERVAL) {
    memset(send_buffer, 0, SEND_BUFFER_LEN);
    snprintf(send_buffer, SEND_BUFFER_LEN, "Client[%s@%s]: uptime:%ld",
             WiFi.macAddress().c_str(),
             WiFi.localIP().toString().c_str(),
             millis());
    Serial.printf("Sending: %s\n", send_buffer);
    blink(BLINK_QUICKLY);
    mq_client.publish(uplink_topic, send_buffer);
    last_uplink_tick = millis();
  }

  mq_client.loop();
}
