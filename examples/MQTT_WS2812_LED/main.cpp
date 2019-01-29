
/*
ESP8266 MQTT example with RGB led--  rewritten from pub sub example to use the wrapper class
MQTThandler
Control an RGB led from NodeRed MQTT

*/
#include <Arduino.h>
#include <MQTThandler.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <fastled.h>

#define NUM_LEDS 1 // one LED in kit
#define LED_PIN 2 // GPIO on ESP8266

// for FastLed
CRGB leds[NUM_LEDS];
uint8_t gRed;
uint8_t gGreen;
uint8_t gBlue;

// Update these with values suitable for the broker used.
const char* svrName = "raspberrypi.local";// if zeroconfig works change as needed
WiFiClient espClient;
long lastMsg = 0;
String S_msg;
int value = 0;
uint8_t GotMail;
uint8_t statusCode;
MQTThandler MTQ(espClient, svrName);
String Debug;

//captive portal config for ESP8266
void configModeCallback(WiFiManager *myWiFiManager) {
	Serial.println("Entered config mode");
	Serial.println(WiFi.softAPIP());
	//if you used auto generated SSID, print it
	Serial.println(myWiFiManager->getConfigPortalSSID());
}

// FastLed, set RGB led color
void SetLed()
{
	leds[0].setRGB(gRed, gGreen, gBlue);
	FastLED.show();
}

// FastLed, turn RGB led off
void SetLedOff()
{
	leds[0] = CRGB::Black;
	FastLED.show();
}

// turns the color hex string into color tuple for FastLed
// return string for debug only
String parseColor(String inColor)
{
	unsigned long tempCol;
	String DebgMsg;
	DebgMsg = "";
	
	if (inColor.startsWith("RGB")){

		// need a properly formatted hex string from input from nodered
		char colStr[8];
		colStr[0]='0';
		colStr[1]='x';
		for (int i=0;i<7;i++)
		{
			colStr[i+2] = inColor.charAt(i+3);
		}
		tempCol = strtoul(colStr, NULL, 16); // convert to long
		// pull out RGB to send to fastled
		gRed = ((tempCol >> 16) & 0xFF);
		gGreen = ((tempCol >> 8) & 0xFF);
		gBlue = ((tempCol) & 0xFF);
		SetLed();
		// debug message
		DebgMsg = "test int is ";  // test code
		DebgMsg = DebgMsg + String(tempCol);
		DebgMsg = DebgMsg + String("\ntest red val is ");
		DebgMsg = DebgMsg + String(gRed);
		DebgMsg = DebgMsg + String ("\ntest green val is ");
		DebgMsg = DebgMsg + String(gGreen);
		DebgMsg = DebgMsg + String("\ntest blue val is ");
		DebgMsg = DebgMsg + String(gBlue);
	}
	return DebgMsg;
}

void setup() {
	pinMode(LED_BUILTIN, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
	Serial.begin(115200); // for debug - demo purposes
	WiFiManager wifiManager;
	wifiManager.setAPCallback(configModeCallback);
	wifiManager.autoConnect("AutoConnectAP");
	Serial.println("Print IP:");
	Serial.println(WiFi.localIP());
	// inint FastLed 
	gRed = 128;
	gBlue = 128;
	gGreen = 128;
	FastLED.addLeds<WS2811, LED_PIN, RGB>(leds, NUM_LEDS);
	SetLed();

	GotMail = false;
	MTQ.setClientName("ESP8266Client");
	MTQ.subscribeIncomming("LEDset");
	MTQ.subscribeOutgoing("msgOut");
}

// For toggle of onboard led on/off
void LedCheck(char Ckc){
	if (Ckc == '1') {
		digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
										  // but actually the LED is on; this is because
										  // it is acive low on the ESP-01)
	}
	else if (Ckc == '0') {
		digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
	}
}

void loop() {
	GotMail = MTQ.update();
	// first check for message
	if (GotMail == true){
		Serial.print("message is: ");
		S_msg = MTQ.GetMsg();
		Serial.println(S_msg);
		LedCheck(S_msg.charAt(0));
		// will set led color here
		Debug = parseColor(S_msg);
		if (Debug != "")
			Serial.println(Debug);
			statusCode = MTQ.publish(Debug);
		GotMail = false;
	}

	long now = millis();
	// publish outgoing once every 2 sec
	if (now - lastMsg > 2000) {
		lastMsg = now;
		++value;
		S_msg = "string message # " + String(value);
		Serial.print("Publish message (main): ");
		Serial.println(S_msg);
		statusCode = MTQ.publish(S_msg);
	}
}