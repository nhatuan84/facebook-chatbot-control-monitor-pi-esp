#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "dd-wrt";
const char* password = "0000000000";
//ip address of Raspberry/orange pi
const char* mqtt_server = "192.168.1.106";
char msg[20];

/* create an instance of PubSubClient client */
WiFiClient espClient;
PubSubClient client(espClient);

/*LED GPIO pin*/
const char led = 4;
/* topics */
#define TEMP_TOPIC    "floor1/room1/temp1"
#define LED_TOPIC     "floor1/room1/led1" /* on, off */

void receivedCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("topic: ");
    Serial.println(topic);
      if(strcmp(topic, LED_TOPIC) == 0){
        Serial.print("payload: ");
        for (int i = 0; i < length; i++) {
          Serial.print((char)payload[i]);
        }
        Serial.println();
        /* we got '1' -> on */
        if ((char)payload[0] == '1') {
          digitalWrite(led, HIGH); 
          snprintf (msg, 20, "%s", "on");
        /* publish the response */
        client.publish(LED_TOPIC "/res", msg);
        } else {
          /* we got '0' -> on */
          digitalWrite(led, LOW);
          snprintf (msg, 20, "%s", "off");
          client.publish(LED_TOPIC "/res", msg);
        }
    }else {
        snprintf (msg, 20, "%d", random(0, 40));
        client.publish(TEMP_TOPIC "/res", msg);
    }
}

void mqttconnect() {
  /* Loop until reconnected */
  while (!client.connected()) {
    Serial.print("MQTT connecting ...");
    /* client ID */
    String clientId = "ESP32Client";
    /* connect now */
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      /* subscribe topic */
      client.subscribe(LED_TOPIC);
      client.subscribe(TEMP_TOPIC);
    } else {
      Serial.print("failed, status code =");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      /* Wait 5 seconds before retrying */
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  /* set led as output to control led on-off */
  pinMode(led, OUTPUT);

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  /* configure the MQTT server with IPaddress and port */
  client.setServer(mqtt_server, 1883);
  /* this receivedCallback function will be invoked 
  when client received subscribed topic */
  client.setCallback(receivedCallback);
}
void loop() {
  /* if client was disconnected then try to reconnect again */
  if (!client.connected()) {
    mqttconnect();
  }
  /* this function will listen for incomming 
  subscribed topic-process-invoke receivedCallback */
  client.loop();
}
