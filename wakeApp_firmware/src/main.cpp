/* 
ESP32 based WiFi-connected app-controlled LED lamp, with dual-way push-putton communication.
Monitors the serial output to see what IP the ESP32/webserver is on
Then send a get request to (visit) http://ip-adress/on and http://ip-adress/off to turn led on and off
If a button is pressed on the ESP32 the ESP32 will send "AWAKE" to the client

wakeApp Firmware

Created: 09.09.25
Updated: 10.09.25
Version: 1.0

@author: smokeB0x
*/
#include <Arduino.h>                                 //Load arduino specific libraries
#include <WiFi.h>                                    //Used to connect the ESP32 to a Wi-Fi network (SSID, password, IP info).

const char* internet_ssid = "***";                   //Input the SSID here
const char* internet_pass = "***";                   //Input the password here
const int ledPin = 2;                                //Use ESP32 Pin 2 for the LED
const int buttonPin = 12;                            //Use ESP32 Pin 12 for the button 

bool buttonPressed = false;                          //Variable to store button state 
bool buttonState = HIGH;                             
bool lastButtonState = HIGH;

unsigned long lastDebounceTime = 0;                 //Variable to store the last debounce time
unsigned long debounceDelay = 50;                   //The debounce time; increase if the output flickers

void IRAM_ATTR handleButtonPress() {
  buttonPressed = true;                              //Set buttonPressed to true when the button is pressed 
}

WiFiServer esp32_server(80);                         //Set webserver port to 80 (HTTP)

String ledState = "OFF";                             //Variable to store the output state 

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);                           //Define the ledPin as output
  digitalWrite(ledPin, LOW);                         //Set the LED to LOW (off)

  pinMode(buttonPin, INPUT_PULLUP);                  //Define the buttonPin as input with pullup resistor 

  WiFi.begin(internet_ssid, internet_pass);          //Connect to the WiFi
  while (WiFi.status() != WL_CONNECTED) {            //Wait for the connection to the WiFi
    delay(500);
    Serial.print(".");
  }
  String esp32_IP = WiFi.localIP().toString();       //Get the IP of the ESP32 and save to a variable (so it can be sent/used/etc later) 
  Serial.println("The IP-address: ");
  Serial.println(esp32_IP);                          //Print the IP to serial 
  esp32_server.begin();                              //Start the server on the ESP32



  Serial.println("Setup complete, starting loop");
}

void loop() {
  bool reading = digitalRead(buttonPin);             //Read the state of the button
 
  if (reading != lastButtonState) {                  //If the button state has changed due to noise or pressing - reset it
  lastDebounceTime = millis();
  }
 
  if ((millis() - lastDebounceTime) > debounceDelay) { //If the button state has been stable for longer than the debounce delay, take it as the actual current state
    if (reading != buttonState) {
        buttonState = reading;
       if (buttonState == LOW) {
        buttonPressed = true;                        //If the button is pressed, set buttonPressed to true
      }
    }
  } 

  lastButtonState = reading;                         //Save the reading. Next time through the loop, it will be the lastButtonState
 
  WiFiClient client = esp32_server.available();      // Listen for incoming clients

  if (client) {
    String request = client.readStringUntil('\r');   //Read the first line of the request
    client.flush();

    if (request.indexOf("/on") != -1) {              //Check if the request contains /on
      digitalWrite(ledPin, HIGH);
      ledState = "ON";
    }
    else if (request.indexOf("/off") != -1) {        //Check if the request contains /off
      digitalWrite(ledPin, LOW);
      ledState = "OFF";
    }

    client.println("HTTP/1.1 200 OK");               //Tells the client that the request succeeded
    client.println("Content-Type: text/html");       //Tells the client that the content is HTML
    client.println();                                //Blank line 


    if (buttonPressed) {
      client.println("AWAKE");                       //If the button was pressed, send "AWAKE" to the client
      buttonPressed = false;                         //Reset the buttonPressed variable
    } else {  
      client.print("LED is ");                       //Send the current state of the LED (the body of the response)          
      client.print(ledState);
    }
    client.stop();                                   //Close the connection                 
  }
}


/*
TODO
- Find a way to set up SSID and password in the app (WiFiManager?).
- Make the actual app for android and make them talk to each other.
- Create the 3d-models for 3d-printing.
*/
