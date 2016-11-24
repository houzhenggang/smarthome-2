#include <ArduinoJson.h>
#include <SPI.h>
#include <Ethernet.h>
#include "PubSubClient.h"
#include "DHT.h"

// Update these with values suitable for your network.
byte mac[]    = {  0xAE, 0xED, 0xBA, 0xF2, 0xFE, 0xED };
IPAddress ip(192,168,70,133);
IPAddress server(192,168,70,31);

bool relay = 0;
bool call = 0;
char message_buf[100];
byte u_iter = 0;


#define DHTTYPE DHT22
#define DHTPIN 2

DHT dht(DHTPIN, DHTTYPE);

// handles message arrived on subscribed topic(s)
void callback(char* topic, byte* payload, unsigned int length) {

    int i = 0;

    //Serial.println("Message arrived:  topic: " + String(topic));
    //Serial.println("Length: " + String(length,DEC));

    // create character buffer with ending null terminator (string)
    for(i=0; i<length; i++) {
        message_buf[i] = payload[i];
    }
    message_buf[i] = '\0';

    String msgString = String(message_buf);

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
            client.subscribe("homehub/clients");
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
    Serial.begin(9600);
    
    dht.begin();


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

  StaticJsonBuffer<200> buf;

  JsonObject& root = buf.createObject();
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  root["temperature"] = t;
  root["humidity"] = h;
  root.prettyPrintTo(Serial);
  Serial.println("");
  
    if(u_iter++ == 10){
      char data[200];
      root.printTo(message_buf, root.measureLength() + 1);
      client.publish("homehub/livingroom/tempctl0", message_buf, true);
      u_iter = 0;
    }

    if(call){
        call = !call;
        client.publish("homehub/clients","0x01 ONLINE");
    }
    delay(1000);
}
