#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <ESP8266HTTPClient.h>  // HTTP Client
#include <WiFiClientSecure.h>

const char* ssid     = "SSID";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "password";     // The password of the Wi-Fi network
const char* host = "example.com"; // Change this to your server's hostname
const uint8_t fingerprint[20] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const char* cookie = "aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa";

const int switchPin = 13;   // Pin connected to the button
int switchState = 0;       // Variable to store the button state
int oldswState = 0;
const int httpsPort = 443;  //HTTPS= 443 and HTTP = 80

void setup() {
  pinMode(switchPin, INPUT_PULLUP);    // Set the button pin as input
  oldswState = digitalRead(switchPin); // Save current state

  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');
  
  WiFi.begin(ssid, password);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
}

void loop() {
  switchState = digitalRead(switchPin);
  if (switchState != oldswState) {
    // pin has changed, do something
    Serial.println("btn pressed");
    open_door();
    delay (500);  // debounce might be useful
  }
  oldswState = switchState;  // remember for next time
}

void open_door() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure httpsClient;    //Declare object of class WiFiClient

    Serial.println(host);

    Serial.printf("Using fingerprint '%s'\n", fingerprint);
    httpsClient.setFingerprint(fingerprint);
    httpsClient.setTimeout(15000); // 15 Seconds

    Serial.print("HTTPS Connecting");
    int r=0; //retry counter
    while((!httpsClient.connect(host, httpsPort)) && (r < 10)){
        delay(100);
        Serial.print(".");
        r++;
    }
    if(r==10) {
      Serial.println("Connection failed");
    }
    else {
      Serial.println("Connected to web");
    }
    
    String getData, Link;
    
    //POST Data
    Link = "/control";
    httpsClient.print(String("POST ") + Link + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "Content-Type: application/x-www-form-urlencoded" + "\r\n" +
                "Cookie: ses=" + cookie + "\r\n"
                "Content-Length: 13" + "\r\n\r\n\r\n" +
                "Connection: close\r\n\r\n");

    Serial.println("request sent");
                    
    while (httpsClient.connected()) {
      String line = httpsClient.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("headers received");
        break;
      }
    }

    Serial.println("reply was:");
    Serial.println("==========");
    String line;
    while(httpsClient.available()){        
      line = httpsClient.readStringUntil('\n');  //Read Line by Line
      Serial.println(line); //Print response
    }
    Serial.println("==========");
    Serial.println("closing connection");

  }
  // curl 'https://example.com/control' -X POST -H 'Cookie: ses=aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa' 
}
