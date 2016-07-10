
#include <SPI.h>
#include <Ethernet.h>
#include "PubSubClient.h"

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xAD, 0xBE, 0xEE, 0xFE, 0xED };
IPAddress ip(192,168,70,134);
IPAddress server(192,168,70,12);

bool relay = 0;
bool call = 0;
char message_buff[100];

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

    //Serial.println("Payload: " + msgString);

    if (msgString.equals("CALLHOME")) {
        call = 1;
    } else if (msgString.equals("1")) {
        relay = 1;
    } else if (msgString.equals("0")) {
        relay = 0;
    }        
}

EthernetClient ethClient;
PubSubClient client(ethClient);

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect("tempct")) {
            Serial.println("connected");
            // Once connected, publish an announcement...
            client.publish("homehub/clients","homehub/livingroom/relayctl0 CONNECTED");
            // ... and resubscribe
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
    Serial.begin(57600);
    pinMode(2, OUTPUT);

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
    digitalWrite(2, relay);
    if(call){
        call = !call;
        client.publish("homehub/clients","homehub/livingroom/relayctl0 ONLINE");
    }
}
