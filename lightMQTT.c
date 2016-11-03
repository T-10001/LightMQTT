/*
 MQTT IOT Example
 - continuously obtains values from the TSL237-J light to frequency sensor
 - formats the results as a JSON string
 - connects to an MQTT server
 - and publishes the JSON String to the topic named readings/
 */

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>

// Update this to either the MAC address found on the sticker on your Ethernet shield (newer shields)
// or a different random hexadecimal value (change at least the last four bytes)
byte mac[]    = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// just a value for display
char macstr[] = "deedbafefeed";
// Update this value to an appropriate open IP on your local network
//IPAddress ip(192, 168, 1, 12);
byte ip[]     = {192, 168, 1, 20 };

// use either the char or byte type for the server address you want to connect to
char servername[]="ec2-35-161-249-176.us-west-2.compute.amazonaws.com";
//byte servername[] = {35,161,249,176};
String clientName = String("d:quickstart:arduino:") + macstr;
String topicName = String("readings/");

volatile unsigned long cnt = 0;
unsigned long intensity;

EthernetClient ethClient;

// Uncomment this next line and comment out the line after it to test against a local MQTT server
//PubSubClient client(localserver, 1883, 0, ethClient);
PubSubClient client(servername, 1883, 0, ethClient);

void setup()
{ 
  // higher baud rates can handle the higher frequencies of light
  // match the serial monitor baud rate to this number
  Serial.begin(250000);
  while(!Serial)
  {
  } 
  // Start the Ethernet client, open up serial port for debugging, and attach the sensor
  Ethernet.begin(mac, ip);
  // pin 2 is input from light to frequency sensor
  pinMode(2, INPUT);
  digitalWrite(2, HIGH);
  attachInterrupt(0, irq1, RISING);
}

void loop()
{
  char clientStr[34];
  clientName.toCharArray(clientStr,34);
  char topicStr[26];
  topicName.toCharArray(topicStr,26);

  // json string and getting the reading
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& object = jsonBuffer.createObject();
  
  getData();
  // making the JSON string
  object["device_name"] = "Human_PC";
  object["sensor_unit"] = "mW/m2";
  object.set("reading", intensity);
  object["sensor_type"] = "Light to frequency";

  // connecting through ethernet
  if (!client.connected()) {
    Serial.print("Trying to connect to: ");
    Serial.println(clientStr);
    client.connect(clientStr);
  }
  
  if (client.connected() ) {
    // turn json to printable form
    char jsonStr[256];
    object.prettyPrintTo(jsonStr, sizeof(jsonStr));

    // publish and check if published
    int pubresult = client.publish(topicStr,jsonStr);
    
    Serial.print("attempt to send ");
    Serial.println(jsonStr);
    Serial.print("to ");
    Serial.println(topicStr);
    if (pubresult)
      Serial.println("successfully sent");
    else
      Serial.println("unsuccessfully sent");
  }

  // delay processing in seconds
  // change value to lower or higher to change the frequency of publishing
  delay(5000);
}

// calculate the intensity
void getData() {
  intensity = (cnt+50)/100;
  cnt = 0;
}

// interrupt which gets frequency of the light
void irq1() {
  cnt++;
}

