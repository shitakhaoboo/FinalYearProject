#ifndef MYFUNCTIONS_H
#define MYFUNCTIONS_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include "other_func.h"
#include "GSM.h"



void setup_wifi() ;
void callback(char*, byte*, unsigned int);
void reconnect();
void token_checker(float);

const char* ssid = "Oz";
const char* password = "qwertyuiop1234567";
const char* mqtt_server = "broker.hivemq.com";
//const char* online_server = "broker.hivemq.com";
const char* willTopic = "device/Fail";
int willQos = 1;
int willRetain = 1;
const char* willMessage = "Disconnected";
boolean cleanSession = false;
int relay = 16;
char* relayer = "OFF";

long lastMsg = 0;
char msg[50];
bool controlstatus=true;
boolean netSwitch = false;
int DisconnectCounter = 0;
boolean use_wifi = false;
boolean use_gsm = false;
int dis = 0;
float current_balance=0;
float initial_balance=0;

LiquidCrystal_I2C lcd(0x27, 16, 2); 
WiFiClient espClient;
PubSubClient client_WIFI(espClient);
void theSwitch();;

void general_setup()
{
  pinMode(relay,OUTPUT);
  pinMode(ACS_Pin,INPUT);
      // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();
  EEPROM.begin(EEPROM_SIZE);
  current_balance=EEPROM.read(0);
  initial_balance=current_balance;
  lcd.setCursor(0, 0);
  lcd.print("Balance : ");
  lcd.setCursor(10, 0);
  lcd.print(current_balance);
  Serial.begin(9600);
  SerialAT.begin(9600);
//  Serial.print("on general setup, balance is: ");
//  Serial.println(token_balance1);
  

}
// 
// 
// 
// 
//to set up the wifi connection

void setup_wifi() 
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  if (WiFi.status() != WL_CONNECTED) {          //if wifi connection is not available, then this the program will never leave this loop
    delay(500);                                    //consider changing it to an if(WiFi.status() != WL_CONNECTED)
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Balance : ");
    lcd.setCursor(10, 0);
    lcd.print(current_balance);
    lcd.setCursor(0, 1);
    lcd.print("NOT CONNECTED");
    Serial.print(".");
    netSwitch = true;
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
// 
// 
// 
//to turn on/off based on the message that has been received
void status_changer(String message)
{
  token_checker(current_balance);
    if (message=="on")
  {
    controlstatus=true;    
  }
    if (message=="off")
  {
    controlstatus=false;
      }
  
}
//check which topic had been received either change or load_data
void topic_check(String topic, String message)
{
  if (topic== "Jkuat-grid/house1/status/change")
  {
      status_changer(message); 
  }
    else if(topic== "Jkuat-grid/house1/load_data_meter")
    {
      String token = String(message.charAt(1)) + String(message.charAt(4))+ String(message.charAt(9))  + String(message.charAt(13)) + "." + String(message.charAt(17));
      //String token = message;
      Serial.print("Received token: ");
      Serial.println(token);
      current_balance = EEPROM.read(0);
      current_balance +=(token.toFloat());
      initial_balance=EEPROM.read(50);
      initial_balance= initial_balance + (token.toFloat());
      Serial.print("Initial balance: ");
      Serial.println(initial_balance);
      if(current_balance<0)
      {
        current_balance = 0.00;
       }
      if(initial_balance<0)
      {
        initial_balance =0.00;
      }
      eeprom_adjust(0,current_balance);
      eeprom_adjust(1,initial_balance);
      
    }

}
void callback(char* topic, byte* payload, unsigned int length)
 {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String messager;
  for (int i = 0; i < length; i++) {
    messager = messager + (char)payload[i];  //Conver *byte to String
  }
  Serial.print(messager);
  //to turn on/off remotely
  topic_check(String(topic),messager);
}

void reconnect(PubSubClient &theClient)  //pass the object reference as a variable 
{
  // Loop until we're reconnected
  if (!theClient.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Balance : ");
    lcd.setCursor(10, 0);
    lcd.print(current_balance);
    lcd.setCursor(0, 1);
    lcd.print("NOT CONNECTED");
    // Create a random client ID
     String clientId = "house1meter";
    // Attempt to connect
    
      if (theClient.connect(clientId.c_str(), NULL , NULL, willTopic, willQos, willRetain, willMessage, cleanSession))
      {
        Serial.println("connected to broker");
        theClient.subscribe("Jkuat-grid/house1/status/change",1);
        theClient.subscribe("Jkuat-grid/house1/load_data_meter",1);
      }
      else
      {
        Serial.print("failed, rc=");
        Serial.println(theClient.state()); 
        DisconnectCounter++;
        Serial.print(DisconnectCounter);
        Serial.println("disconnected counter");
      }

      if(DisconnectCounter > 10)      //if the number of attempted reconnection is greater than 3, then switch the network and check if the other one is available
      {                              //and use it. WHEN SWITCHING TO WIFI, THE SETUP WIFI HAS A 
        DisconnectCounter = 0;
        theSwitch();   
      }//end if   
   }
   else
   {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Balance : ");
    lcd.setCursor(10, 0);
    lcd.print(current_balance);
    lcd.setCursor(0, 1);
    lcd.print("CONNECTED");
   }
   
} //end reconnect


//this function will turn on the relay depending on the control conditions. these are 
// the balance and the control command from the user interface
void token_checker(float token)
{
  if (token<=0)
  {
    digitalWrite(relay,HIGH);  //disconnect customer
    relayer = "OFF";
  }
  else if (controlstatus)
  {
    //goes on
    digitalWrite(relay,LOW);
    relayer = "ON";
  }
  else if (!controlstatus)
  {
    digitalWrite(relay,HIGH);
    relayer = "OFF";
  }
}


void theSwitch()
{  
  if(netSwitch)  //netSwitch = true means we are using wifi. hence try and connect to GSM
  {
    netSwitch = false;
    setup_GSM();
    mqtt.setServer(mqtt_server, 1883);
    mqtt.setCallback(callback);  
    mqtt.subscribe("Jkuat-grid/house1/status/change",1);
    mqtt.subscribe("Jkuat-grid/house1/load_data_meter",1);  
    Serial.println("done setting up GSM"); 
  }
  
  else if(!netSwitch)    //if gsm is not connected to gprs, switch back to wifi. netSwitch = false
  {
    netSwitch = true;     //netSwitch = true menas wifi is the default gateway. This is important as it also decides on the object reference to pass to pubsubclient
    Serial.println("Switching to WiFi");
    setup_wifi();
    client_WIFI.setServer(mqtt_server, 1883);
    client_WIFI.setCallback(callback);   
    client_WIFI.subscribe("Jkuat-grid/house1/status/change",1);
    client_WIFI.subscribe("Jkuat-grid/house1/load_data_meter",1);
    Serial.println("done setting up WiFi");
  }
   
  else  //wifi is not connected.so try connecting with GSM
  {
    Serial.println("doing nothing in netSwitch");
    
  }
}

#endif
