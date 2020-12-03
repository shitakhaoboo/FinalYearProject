#ifndef CORE_FUNCTIONS_H
#define CORE_FUNCTIONS_H

#include "netSwitcher.h"


TaskHandle_t Task1;
TaskHandle_t Task2;
static uint8_t taskCoreZero = 0;
static uint8_t taskCoreOne  = 1;


void Task1code( void * pvParameters )
{
  setup_GSM();      //function defined in GSM.h. This will initialize GSM and connect it to internet through gprs tech
    if(GprsConnected)      //this is a flag set if gprsConnected() returns true. it is set within the GSM function
    {
      use_gsm = true;
      use_wifi = false;
      mqtt.setServer(mqtt_server, 1883);
      mqtt.setCallback(callback);  
      mqtt.subscribe("Jkuat-grid/house1/status/change");
      mqtt.subscribe("Jkuat-grid/house1/load_data_meter",1);     
    }  
    
    else   //if GprsConnected evaluates to false, then wifi connection is attempted. 
    {
      use_wifi = true;
      use_gsm = false;
      Serial.println("Setting up wifi");
      setup_wifi();
      GprsConnected = false;
      client_WIFI.setServer(mqtt_server, 1883);
      client_WIFI.setCallback(callback);   
      client_WIFI.subscribe("Jkuat-grid/house1/status/change");
      client_WIFI.subscribe("Jkuat-grid/house1/load_data_meter",1);
      Serial.println("done setting up WiFi");
    }
    
    while(true)
    {
      if(!netSwitch)  //if netswitch is false, use gsm
      {
        theLoop(mqtt);
      }
      else
      {
        theLoop(client_WIFI);
      } delay(1000);
    } 
}



void Task2code( void * pvParameters ){
  // Serial.print("Task2 running on core ");
  // Serial.println(xPortGetCoreID());

  while(true){
    token_checker(current_balance);
    current_balance=power_adjust(current_balance);
    initial_balance=EEPROM.read(50);
    if ((initial_balance-current_balance)>=0.1)
    {
      eeprom_adjust(0,current_balance);
    }
    delay(1000);
  }
} 


void task_create()
{
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */                  
   delay(500); 

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Task2code,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    2,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */
     delay(500); 
}  

#endif
