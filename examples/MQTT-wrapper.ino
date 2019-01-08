/*
Basic ESP8266 MQTT example --  rewritten from pub sub example to use the wrapper class
MQTThandler
This sketch demonstrates the capabilities of the pubsub library in combination
with the ESP8266 board/library.

It connects to an MQTT server then:
- publishes "hello world" to the topic "outTopic" every two seconds
- subscribes to the topic "inTopic", printing out any messages
it receives. NB - it assumes the received payloads are strings not binary
- If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
else switch it off


*/

#include <MQTThandler.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

// Update these with values suitable for your network.

// const char* mqtt_server = "raspberrypi.local"; // don't work, don't know why
IPAddress MQTTIp(192,168,1,117); // IP oF the MQTT broker
WiFiClient espClient;
long lastMsg = 0;
String S_msg;
int value = 0;
uint8_t GotMail;
uint8_t statusCode;
MQTThandler MTQ(espClient, MQTTIp);

void configModeCallback(WiFiManager *myWiFiManager) {
	Serial.println("Entered config mode");
	Serial.println(WiFi.softAPIP());
	//if you used auto generated SSID, print it
	Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setup() {
	pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
	Serial.begin(115200);
	WiFiManager wifiManager;
	wifiManager.setAPCallback(configModeCallback);
	wifiManager.autoConnect("AutoConnectAP");
	Serial.println("Print IP:");
	Serial.println(WiFi.localIP());
	GotMail = false;
	MTQ.setClientName("ESP8266Client");
	MTQ.subscribeIncomming("inTopic");
	MTQ.subscribeOutgoing("outTopic");
}
// For toggle of led on/off
void LedCheck(char Ckc){
	if (Ckc == '1') {
		digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
										  // but actually the LED is on; this is because
										  // it is acive low on the ESP-01)
	}
	else {
		digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
	}
}

void loop() {
	GotMail = MTQ.update();
	if (GotMail == true){
		Serial.print("message is: ");
		S_msg = MTQ.GetMsg();
		Serial.println(S_msg);
		LedCheck(S_msg.charAt(1));
		GotMail = false;
	}
	long now = millis();
	if (now - lastMsg > 2000) {
		lastMsg = now;
		++value;
		S_msg = "string message # " + String(value);
		Serial.print("Publish message (main): ");
		Serial.println(S_msg);
		statusCode = MTQ.publish(S_msg);
	}
}