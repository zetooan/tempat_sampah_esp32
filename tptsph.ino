#include <Servo.h>
#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <WebServer.h>
#include "DHT.h"

// Replace with your network credentials
const char* ssid = "SingleFighter";
const char* password = "minimalmandi";

// Initialize Telegram BOT
#define BOTtoken "5390158473:AAE6EfzdSQlNLYYCq1kQvTdWVVBgP9DJSY4"  // Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
#define CHAT_ID "-1001557804010"

#define TRIG_PIN  14  
#define ECHO_PIN  26 
#define TRIG_PIN01  15  
#define ECHO_PIN01  25 
#define TRIG_PIN02  5 
#define ECHO_PIN02  27
#define SERVO_PIN 14
#define DISTANCE_THRESHOLD  50 
#define DHTTYPE DHT11 

WebServer server(80);

SoftwareSerial mySoftwareSerial(16, 17); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

Servo servo;

uint8_t DHTPin = 4; 
int sensorAO_pin = 33;
int servoAO_pin = 5;
int val;

DHT dht(DHTPin, DHTTYPE); 
Servo servoAO;

float Temperature;
float Humidity;
float duration, distance;
float duration01, distance01;
float duration02, distance02;

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

const int ledPin = 2;
bool ledState = LOW;

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/led_on") {
      bot.sendMessage(chat_id, "LED state set to ON", "");
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
    }
    
    if (text == "/led_off") {
      bot.sendMessage(chat_id, "LED state set to OFF", "");
      ledState = LOW;
      digitalWrite(ledPin, ledState);
    }
    
    if (text == "/state") {
      if (digitalRead(ledPin)){
        bot.sendMessage(chat_id, "LED is ON", "");
      }
      else{
        bot.sendMessage(chat_id, "LED is OFF", "");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  mySoftwareSerial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT); 
  pinMode(ECHO_PIN, INPUT); 
  pinMode(TRIG_PIN01, OUTPUT); 
  pinMode(ECHO_PIN01, INPUT); 
  pinMode(TRIG_PIN02, OUTPUT);
  pinMode(ECHO_PIN02, INPUT);
  servo.attach(SERVO_PIN);
  servo.write(0);
  pinMode(sensorAO_pin,INPUT);
  servoAO.attach(servoAO_pin);
  servoAO.write(180);
  pinMode(DHTPin, INPUT);
  dht.begin();   
  
  Serial.println("-----------------------");
  Serial.println("---IOT Tempat Sampah---");
  Serial.println("--By : Single Fighter--");
  Serial.println("-----------------------");

  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
  
  Serial.println(F("Initializing Speaker ... (May take 3~5 seconds)"));
  if (!myDFPlayer.begin(mySoftwareSerial, false)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("Speaker Mini online."));
  
  myDFPlayer.volume(10);  //Set volume value. From 0 to 30
  //myDFPlayer.play(1);  //Play the first mp3
  
  bot.sendMessage(CHAT_ID, "IOT Bot Started", "");
  myDFPlayer.play(2);
  delay (2000);
}

void loop() {
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034/2;
  
  float Temperature = dht.readTemperature();
  float Humidity = dht.readHumidity();
  
  val = digitalRead(sensorAO_pin);

  digitalWrite(TRIG_PIN01, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN01, HIGH);
  delayMicroseconds(5);
  digitalWrite(TRIG_PIN01, LOW);
  duration01 = pulseIn(ECHO_PIN01, HIGH);
  distance01 = duration01 * 0.034/2; 
  
  digitalWrite(TRIG_PIN02, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN02, HIGH);
  delayMicroseconds(5);
  digitalWrite(TRIG_PIN02, LOW);
  duration02 = pulseIn(ECHO_PIN02, HIGH);
  distance02 = duration02 * 0.034/2;
  
  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read());
  }
  
  if (distance < DISTANCE_THRESHOLD){
     servo.write(90); // rotate servo motor to 0 degree
     delay (3000);
     servo.write(0);
     myDFPlayer.play (1);
     delay (6000);
     myDFPlayer.stop ();
     myDFPlayer.disableLoopAll();
  }

  if (val==0)
  {
    servoAO.write(180);
  }
  
  if(Humidity > 81)
    {
       servoAO.write(270);
       delay (6000);
       servoAO.write(180);
         if (val==0)
          {
            servoAO.write(180);
          }
    }

   if(Humidity < 81)
    {
       servoAO.write(90);
       delay (6000);
       servoAO.write(180);
         if (val==0)
          {
            servoAO.write(180);
          }
   
    }
  if (distance01 < 20) 
  {  
    bot.sendMessage(CHAT_ID, "Tempat Sampah Anorganik sudah penuh, Perlu dibersihkan !!!", "");
    Serial.print("Tempat Sampah Anorganik sudah penuh, Perlu dibersihkan !!!"); 
  }

  if (distance02 < 20) 
  {  
    bot.sendMessage(CHAT_ID, "Tempat Sampah Organik sudah penuh, Perlu dibersihkan !!!", "");
    Serial.print("Tempat Sampah Organik sudah penuh, Perlu dibersihkan !!!"); 
  }
  
  else {
    servo.write(0) ;
    // rotate servo motor to 90 degree
  }
      Serial.print("Temperature:");
      Serial.print(Temperature, 1);
      Serial.print("\n");

      Serial.print("Humidity:");
      Serial.print(Humidity, 1);
      Serial.print("\n");
      
      Serial.print("Jarak orang dengan tempat sampah : ");
      Serial.print(distance);
      Serial.println(" cm");
    
      Serial.print("Jarak Tempat Sampah Anorganik : ");
      Serial.print(distance01);
      Serial.println(" cm");
    
      Serial.print("Jarak Tempat Sampah Organik : ");
      Serial.print(distance02);
      Serial.println(" cm");
      delay(5);
    
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
} 

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
   // case DFPlayerPlayFinished:
   //  Serial.print(F("Number:"));
   //   Serial.print(value);
   //   Serial.println(F(" Play Finished!"));
   //   break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

void handle_OnConnect() {

 Temperature = dht.readTemperature(); // Gets the values of the temperature
 Humidity = dht.readHumidity(); // Gets the values of the humidity 
 server.send(200, "text/html", SendHTML(Temperature,Humidity)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float TempCstat,float Humiditystat){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<link href=\"https://fonts.googleapis.com/css?family=Open+Sans:300,400,600\" rel=\"stylesheet\">\n";
  ptr +="<title>ESP32 Weather Report</title>\n";
  ptr +="<style>html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #333333;}\n";
  ptr +="body{margin-top: 50px;}\n";
  ptr +="h1 {margin: 50px auto 30px;}\n";
  ptr +=".side-by-side{display: inline-block;vertical-align: middle;position: relative;}\n";
  ptr +=".humidity-icon{background-color: #3498db;width: 30px;height: 30px;border-radius: 50%;line-height: 36px;}\n";
  ptr +=".humidity-text{font-weight: 600;padding-left: 15px;font-size: 19px;width: 160px;text-align: left;}\n";
  ptr +=".humidity{font-weight: 300;font-size: 60px;color: #3498db;}\n";
  ptr +=".temperature-icon{background-color: #f39c12;width: 30px;height: 30px;border-radius: 50%;line-height: 40px;}\n";
  ptr +=".temperature-text{font-weight: 600;padding-left: 15px;font-size: 19px;width: 160px;text-align: left;}\n";
  ptr +=".temperature{font-weight: 300;font-size: 60px;color: #f39c12;}\n";
  ptr +=".superscript{font-size: 17px;font-weight: 600;position: absolute;right: -20px;top: 15px;}\n";
  ptr +=".data{padding: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  
   ptr +="<div id=\"webpage\">\n";
   
   ptr +="<h1>ESP32 Weather Report</h1>\n";
   ptr +="<div class=\"data\">\n";
   ptr +="<div class=\"side-by-side temperature-icon\">\n";
   ptr +="<svg version=\"1.1\" id=\"Layer_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n";
   ptr +="width=\"9.915px\" height=\"22px\" viewBox=\"0 0 9.915 22\" enable-background=\"new 0 0 9.915 22\" xml:space=\"preserve\">\n";
   ptr +="<path fill=\"#FFFFFF\" d=\"M3.498,0.53c0.377-0.331,0.877-0.501,1.374-0.527C5.697-0.04,6.522,0.421,6.924,1.142\n";
   ptr +="c0.237,0.399,0.315,0.871,0.311,1.33C7.229,5.856,7.245,9.24,7.227,12.625c1.019,0.539,1.855,1.424,2.301,2.491\n";
   ptr +="c0.491,1.163,0.518,2.514,0.062,3.693c-0.414,1.102-1.24,2.038-2.276,2.594c-1.056,0.583-2.331,0.743-3.501,0.463\n";
   ptr +="c-1.417-0.323-2.659-1.314-3.3-2.617C0.014,18.26-0.115,17.104,0.1,16.022c0.296-1.443,1.274-2.717,2.58-3.394\n";
   ptr +="c0.013-3.44,0-6.881,0.007-10.322C2.674,1.634,2.974,0.955,3.498,0.53z\"/>\n";
   ptr +="</svg>\n";
   ptr +="</div>\n";
   ptr +="<div class=\"side-by-side temperature-text\">Temperature</div>\n";
   ptr +="<div class=\"side-by-side temperature\">";
   ptr +=(int)TempCstat;
   ptr +="<span class=\"superscript\">°C</span></div>\n";
   ptr +="</div>\n";
   ptr +="<div class=\"data\">\n";
   ptr +="<div class=\"side-by-side humidity-icon\">\n";
   ptr +="<svg version=\"1.1\" id=\"Layer_2\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n\"; width=\"12px\" height=\"17.955px\" viewBox=\"0 0 13 17.955\" enable-background=\"new 0 0 13 17.955\" xml:space=\"preserve\">\n";
   ptr +="<path fill=\"#FFFFFF\" d=\"M1.819,6.217C3.139,4.064,6.5,0,6.5,0s3.363,4.064,4.681,6.217c1.793,2.926,2.133,5.05,1.571,7.057\n";
   ptr +="c-0.438,1.574-2.264,4.681-6.252,4.681c-3.988,0-5.813-3.107-6.252-4.681C-0.313,11.267,0.026,9.143,1.819,6.217\"></path>\n";
   ptr +="</svg>\n";
   ptr +="</div>\n";
   ptr +="<div class=\"side-by-side humidity-text\">Humidity</div>\n";
   ptr +="<div class=\"side-by-side humidity\">";
   ptr +=(int)Humiditystat;
   ptr +="<span class=\"superscript\">%</span></div>\n";
   ptr +="</div>\n";

  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
