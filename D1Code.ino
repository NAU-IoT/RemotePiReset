//This code is written in C, and is intended to be used with the Arduino IDE

//The code's function is to connect a D1 Mini to a wifi network, connect to MQTT, then subscribe to a specified topic.

//Once subscribed, the D1 mini will constantly look for new messages being published to the topic. If a certain message(in this case "reset")
// is published to the topic, the D1 Mini will send a signal to a 5V relay, cutting power to the load(RaspberryPi) for 5 seconds. After 5 seconds, the 
// relay will switch back and the RaspberryPi will turn back on.

// This project is intended as a solution to the problem of a raspberrypi locking up and ssh is not an option for remote resetting the Rpi.

//Code:



#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//set wifi credentials
const char* WIFI_SSID = "YourOwnNetwork";
const char* WIFI_PASSWORD = "YourOwnPassword";

//mqtt info
const char* mqtt_server = "test.mosquitto.org"; // can change to your own server, can be d1 IP address, just make sure when subsribing, use sub -h (d1 IP)
const char* topic = "RemotePiReset";
const char* test_message = "Hello from D1mini";

WiFiClient espClient;
PubSubClient client(espClient);

char msg[50];

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.print(topic);
  Serial.print("\n");
  Serial.print("Message: ");
  
  String msg;
  for (int i = 0; i < length; i++) {
     Serial.print((char)payload[i]);
    msg += (char)payload[i];    //copy payload into msg
  }

  //copy msg into msgstr to get comparable format for strcmp
  const char* msgstr = msg.c_str();
  
  // Reset Rpi if "reset" was published to server
  if (strcmp("reset", msgstr) == 0) {
    digitalWrite(D1, HIGH);   // Turns off the Rpi via relay

    delay(5000); //waits 5 seconds

    digitalWrite(D1, LOW); // Turns Rpi back on via relay
  }

  
  Serial.println();
  
}


void setup() {

  //set pin to output to control relay
  pinMode(D1, OUTPUT);
  
  //set up serial port
  Serial.begin(115200);
  Serial.println();

  //begin wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  //Connecting to WiFi
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  Serial.print("\n");

  //loop this message while WiFi is not connected
  while (WiFi.status() != WL_CONNECTED){
    delay(2000);
    Serial.print("\n");
    Serial.print("Connecting...");
  }

  //Conected to WiFi
  Serial.println();
  Serial.print("Connected, D1's IP address: ");
  Serial.println(WiFi.localIP());

  //initializing more client info
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
    reconnect();
}

void reconnect() {
  // Loop until reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    uint32_t chipid=ESP.getChipId();
    char clientid[25];
    snprintf(clientid,25,"D1MiniPro-%08X",chipid); //this adds the chipid to the client for a unique id
    Serial.print("Client ID: ");
    Serial.println(clientid);
    if (client.connect(clientid)) {
      Serial.println("Connected");
      // Once connected, publish a test message
      client.publish(topic, test_message);
      // Resubscribe after publishing test message
      client.subscribe("RemotePiReset");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  
  client.loop();

}


