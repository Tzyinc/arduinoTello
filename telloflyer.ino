#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

// HARDWARE CONNECTION
const int ONBOARD_LED = 2;
const int BUTTON_TAKEOFF = D5;
const int BUTTON_LAND = D6;

// tello settings
const char* DEFAULT_SSID = "TELLO-AAC8EA";
const char* DEFAULT_PW = "";
const char* TELLO_IP = "192.168.10.1";
const int PORT = 8889;

// state constants
const int INIT = 0;
const int CONNECTED = 1;

WiFiUDP Udp;
char packetBuffer[255]; //buffer to hold incoming packet
int networkState = 0;
String lastMessage = "";

void setup() {
  Serial.begin(115200);
  pinMode(ONBOARD_LED, OUTPUT);
  pinMode(BUTTON_TAKEOFF, INPUT_PULLUP);
  pinMode(BUTTON_LAND, INPUT_PULLUP);
}

void loop() {
  String message = listenMessage();
  if (message != "") {
    lastMessage = message;
  }
  if (networkState == INIT) {
    initSequence();
  } else if (networkState == CONNECTED) {
    controlSequence();
  }
}

void initSequence() {
  connect();
}

// TODO: exit condition
void connect() {
  if (lastMessage == "OK") {
    //means this part has run and can skip
    lastMessage = "";
    networkState = CONNECTED;
    return;
  }
  WiFi.begin(DEFAULT_SSID, DEFAULT_PW);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Udp.begin(PORT);
  sendMessage("command");
}

void controlSequence() {
  if (digitalRead(BUTTON_TAKEOFF) == 0) {
    sendMessage("takeoff");
  }
  if (digitalRead(BUTTON_LAND) == 0) {
    sendMessage("land");
  }
}

String listenMessage() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remoteIp = Udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = 0;
    }
    Serial.println("Contents:");
    Serial.println(packetBuffer);
  }
  // this only works as tello's API doesn't return responses greater than 255 char
  return (char*) packetBuffer;
}

void sendMessage(char* ReplyBuffer) {
  Udp.beginPacket(TELLO_IP, PORT);
  Udp.write(ReplyBuffer);
  Udp.endPacket();
}


