#ifndef netSwitch_h
#define netSwitch_h

#include "myfunctions.h"

int mycounter=0;

void theLoop(PubSubClient &theClient)
{
      if(!theClient.connected())
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Balance : ");
          lcd.setCursor(10, 0);
          lcd.print(current_balance);
          lcd.setCursor(0, 1);
          lcd.print("NOT CONNECTED");
          reconnect(theClient);
        }     
        initial_balance=EEPROM.read(50);
        Serial.println(initial_balance);
        static char tb[7];
        dtostrf(current_balance, 6, 2, tb);
        theClient.publish("Jkuat-grid/house1/balance", tb);
        Serial.println(tb);
        theClient.publish("Jkuat-grid/house1/status/now", relayer);
        Serial.println(relayer);
        delay(3000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Balance : ");
        lcd.setCursor(10, 0);
        lcd.print(current_balance);
        lcd.setCursor(0, 1);
        lcd.print("CONNECTED    ");
        theClient.loop();
        // ten second delay
        if (mycounter==10)
        {
          //publish consumtpion data and adjust the eeprom value
          float used_up = (initial_balance - current_balance);
          if (used_up>1.2)
          {
            static char tb1[7];
            dtostrf(used_up , 6, 2, tb1);
            theClient.publish("Jkuat-grid/house1/consumed", tb1 );
          }
           eeprom_adjust(1,current_balance);
           mycounter=0;
        } 
        mycounter+=1;
}

//this function switches connection between wifi and gsm based on the one that is available.
//the flag netSwitch = false, GSM is the gateway in use, else if the Switch= true WIFI is the default gateway

#endif
