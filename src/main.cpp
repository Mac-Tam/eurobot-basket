#include <WiFi.h>
#include <Arduino_JSON.h>
#include <WebSocketsClient.h>
#include <LiquidCrystal.h>
#include <Ticker.h>

// Function headers
String toDigits(int num);
void publish_num_balls(WebSocketsClient &webSocket);
void publish_turtle_vel(WebSocketsClient &webSocket);
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);

// Global vars
volatile int num_balls = 0;
volatile int score = 0;

// Network settings
const char *ssid = "ssid";
const char *pass = "pass";
const char *serverIP = "192.168.0.0";

// Display
const int rs = 32, en = 33, d4 = 26, d5 = 27, d6 = 14, d7 = 12;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// WS connection
WebSocketsClient webSocket;

/////////////////////Periodic tasks///////////////////////////

/////////////////////Periodic tasks///////////////////////////

void setup()
{
  Serial.begin(9600);
  lcd.begin(16, 2);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    lcd.clear();
    lcd.print("wifi ...");
    delay(1000);
  }

  lcd.clear();
  lcd.print("wifi ok!");
  delay(1000);

  lcd.clear();
  delay(1000);

  webSocket.begin(serverIP, 9090);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(1000);

  while (!webSocket.isConnected())
  {
    webSocket.loop();
    lcd.clear();
    lcd.print("server ...");
    delay(100);
  }

  lcd.clear();
  lcd.print("server ok!");
  delay(1000);
  lcd.clear();
  ////////Setup communication with rosbridge package
  //webSocket.sendTXT("{\"op\": \"advertise\", \"topic\": \"/turtle1/cmd_vel\", \"type\": \"geometry_msgs/msg/Twist\"}");
  webSocket.sendTXT("{\"op\": \"advertise\", \"topic\": \"/basket/num_balls\", \"type\": \"std_msgs/Int32\"}");
  webSocket.sendTXT("{\"op\":\"subscribe\",\"topic\":\"/basket/score\",\"type\":\"std_msgs/msg/Int32\"}");

  lcd.print("finished setup");
  delay(1000);
  lcd.clear();
}

void loop()
{
  //collect received messages
  webSocket.loop();

  num_balls = random(0, 50);
  lcd.setCursor(0, 0);
  lcd.print("Balls: ");
  lcd.print(toDigits(num_balls));
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(toDigits(score));
  // publish_turtle_vel(webSocket);
  publish_num_balls(webSocket);
  delay(500);
}

// a function that takes an int and returns a three digit string
String toDigits(int num)
{
  String str = "";
  if (num < 10)
  {
    str = "00" + String(num);
  }
  else if (num < 100)
  {
    str = "0" + String(num);
  }
  else
  {
    str = String(num);
  }
  return str;
}

// Publisher function for num_balls
void publish_num_balls(WebSocketsClient &webSocket)
{
  JSONVar doc;
  doc["op"] = "publish";
  doc["topic"] = "/basket/num_balls";
  JSONVar msg;
  msg["data"] = num_balls;
  doc["msg"] = msg;
  String json_str = JSON.stringify(doc);
  webSocket.sendTXT(json_str);
}

//control turtlesim from esp
void publish_turtle_vel(WebSocketsClient &webSocket)
{
  JSONVar doc;
  doc["op"] = "publish";
  doc["topic"] = "/turtle1/cmd_vel";
  JSONVar msg;
  JSONVar linear;
  JSONVar angular;
  angular["x"] = 0.0;
  angular["y"] = 0.0;
  angular["z"] = 1.0;
  linear["x"] = 2.0;
  linear["y"] = 0.0;
  linear["z"] = 0.0;
  msg["angular"] = angular;
  msg["linear"] = linear;
  doc["msg"] = msg;
  String json_str = JSON.stringify(doc);
  webSocket.sendTXT(json_str);
}

// event handlers for ws connection
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  JSONVar doc;
  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.println("disconnected from ros-bridge");
    break;
  case WStype_CONNECTED:
    Serial.println("connected to ros-bridge");
    break;
  case WStype_TEXT:
    Serial.println("received text");
    Serial.println((char *)payload);
    doc = JSON.parse((char *)payload);
    //Serial.println("received score");
    score = doc["msg"]["data"];
    //Serial.println(score);
    break;

  case WStype_BIN:
  case WStype_ERROR:
  case WStype_FRAGMENT_TEXT_START:
  case WStype_FRAGMENT_BIN_START:
  case WStype_FRAGMENT:
  case WStype_FRAGMENT_FIN:
    break;
  }
}