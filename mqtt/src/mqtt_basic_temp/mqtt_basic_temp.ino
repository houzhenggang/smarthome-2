
#include <SPI.h>
#include <Ethernet.h>
#include "PubSubClient.h"
#include "dht11.h"

// Update these with values suitable for your network.
byte mac[]    = {  0xAE, 0xED, 0xBA, 0xF2, 0xFE, 0xED };
IPAddress ip(192,168,70,133);
IPAddress server(192,168,70,12);

bool relay = 0;
bool call = 0;
char message_buff[100];
byte u_iter = 0;

dht11 DHT11;
#define DHT11PIN 2

// handles message arrived on subscribed topic(s)
void callback(char* topic, byte* payload, unsigned int length) {

    int i = 0;

    //Serial.println("Message arrived:  topic: " + String(topic));
    //Serial.println("Length: " + String(length,DEC));

    // create character buffer with ending null terminator (string)
    for(i=0; i<length; i++) {
        message_buff[i] = payload[i];
    }
    message_buff[i] = '\0';

    String msgString = String(message_buff);

    Serial.println("Payload: " + msgString);

    if (msgString.equals("CALLHOME")) {
        call = 1;
    }
}

EthernetClient ethClient;
PubSubClient client(ethClient);

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect("tempctl0")) {
            Serial.println("connected");
            // Once connected, publish an announcement...
            //client.publish("homehub/clients","homehub/livingroom/tempctl0 CONNECTED");
            // ... and resubscribe
            //client.subscribe("homehub/livingroom/tempctl0");
            client.subscribe("homehub/clients");
            //client.publish("home/garden/fountain", "Hello from Arduino");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setup()
{
    Serial.begin(57600);

    client.setServer(server, 1883);
    client.setCallback(callback);

    Ethernet.begin(mac, ip);
    // Allow the hardware to sort itself out
    delay(1500);
}

void loop()
{
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    
  Serial.println("\n");

  int chk = DHT11.read(DHT11PIN);

  Serial.print("Read sensor: ");
  switch (chk)
  {
    case DHTLIB_OK: 
		Serial.println("OK"); 
		break;
    case DHTLIB_ERROR_CHECKSUM: 
		Serial.println("Checksum error"); 
		break;
    case DHTLIB_ERROR_TIMEOUT: 
		Serial.println("Time out error"); 
		break;
    default: 
		Serial.println("Unknown error"); 
		break;
  }
  int temperature = DHT11.temperature;
  int humidity = DHT11.humidity;
  String s_temp = String(temperature - 2);
  char c_temp[sizeof(s_temp)];
  s_temp.toCharArray(c_temp, sizeof(c_temp));
  String s_humi = String(humidity);
  char c_humi[sizeof(s_humi)];
  s_temp.toCharArray(c_humi, sizeof(c_humi));
    if(u_iter++ == 10){
      client.publish("homehub/livingroom/tempctl0", c_temp);
      client.publish("homehub/livingroom/humictl0", c_humi);
      u_iter = 0;
    }

    if(call){
        call = !call;
        client.publish("homehub/clients","0x01 ONLINE");
    }
    delay(1000);
}
