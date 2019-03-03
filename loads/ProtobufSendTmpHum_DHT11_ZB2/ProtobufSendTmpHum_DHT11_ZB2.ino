/**
 * Copyright (c) 2009 Andrew Rapp. All rights reserved.
 *
 * This file is part of XBee-Arduino.
 *
 * XBee-Arduino is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * XBee-Arduino is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with XBee-Arduino.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <XBee.h>
// Include the libraries we need
#include <DHT.h>
#include <DHT_U.h>

#include <MemoryFree.h>



#include <pb_encode.h>
#include <pb_decode.h>
#include "weather.pb.h"

// Data wire is plugged into port 2 on the Arduino
#define DHTTYPE DHT11
#define DHT11_PIN 2
#define WEATHER_MESSAGE_ID 0

DHT dht(DHT11_PIN, DHTTYPE);

struct Message{
  size_t message_length;
  size_t payload_length;
  uint8_t* payload;
  bool isSuccessEncoded;

  Message::~Message(){
    delete payload;
  }

};

ZBTxStatusResponse txStatus = ZBTxStatusResponse();

/*
This example is for Series 2 XBee
 Sends a ZB TX request with the value of analogRead(pin5) and checks the status response for success
*/

XBee xbee = XBee();

unsigned long start = millis();


int statusLed = 11;
int errorLed = 12;

float currentTempF = 0;
float currentHumidity = 0;

void flashLed(int pin, int times, int wait) {
    
    for (int i = 0; i < times; i++) {
      digitalWrite(pin, HIGH);
      delay(wait);
      digitalWrite(pin, LOW);
      
      if (i + 1 < times) {
        delay(wait);
      }
    }
}

void setup() {
  pinMode(statusLed, OUTPUT);
  pinMode(errorLed, OUTPUT);
  Serial.begin(9600);

  xbee.setSerial(Serial);

  while(millis() - start < 15000){
    //loop and hold for setup to really complete
  }
}

Message getMessage(float temp, float humidity){

    uint8_t* payload = new uint8_t[128];
    Serial.println("Size of payload");
    Serial.println(sizeof(payload));
    WeatherMessage msg = WeatherMessage_init_zero;
    pb_ostream_t stream = pb_ostream_from_buffer(payload, 128);
    msg.temperatureF = temp;
    msg.humidity = humidity;
   
    /* REQUIRED: MARK Optional Fields as included or not included! */
    msg.has_temperatureF = true;
    msg.has_humidity = true;
    

    bool status = pb_encode(&stream, WeatherMessage_fields, &msg);
    Serial.print("encoding status");
    Serial.println(status);
    
    size_t payload_length = stream.bytes_written;

    //add on a weather message identifyer byte
    uint8_t id = WEATHER_MESSAGE_ID;

    uint8_t* readyPayload = new uint8_t[payload_length + 1];

    memcpy(&readyPayload[1], payload, payload_length);
    
    delete [] payload;

    Serial.print("Free memory: ");
    Serial.println(freeMemory());
    
    readyPayload[0] = WEATHER_MESSAGE_ID;

    //format a struct and return it
    Message m;
    m.payload_length = payload_length;
    m.message_length = payload_length + 1;
    m.payload = readyPayload;
    m.isSuccessEncoded = status;

    
    return m;
  
}

void transmitData(){
Serial.print("Free Memory:");
Serial.println(freeMemory());
Serial.println("Transmitting new data");
Serial.print("Temp:"); Serial.println(currentTempF);
Serial.print("Hum:"); Serial.println(currentHumidity);

    Message m = getMessage(currentTempF, currentHumidity);

    if (!m.isSuccessEncoded)
    {
        Serial.println("Encoding failed");
    }else{


        //Address of the receiving basestation XBee
        XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, 0x405C133A);


        /* REQUIRED: DO NOT SENT MORE BYTES THAN WHATS ACTUALLY IN THE PAYLOAD MESSAGE WRITTEN TO THE BUFFER! */
        /* DO NOT SEND THE WHOLE BUFFER SIZE! */
        ZBTxRequest zbTx = ZBTxRequest(addr64, m.payload, m.message_length);
        
        xbee.send(zbTx);
  
        // flash TX indicator
        flashLed(statusLed, 1, 100);
      }
    
      // after sending a tx request, we expect a status response
      // wait up to 5 seconds for the status response
      if (xbee.readPacket(500)) {
          // got a response!
  
          // should be a znet tx status              
        if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
           xbee.getResponse().getZBTxStatusResponse(txStatus);
          
           // get the delivery status, the fifth byte
             if (txStatus.getDeliveryStatus() == SUCCESS) {
                // success.  time to celebrate
                flashLed(statusLed, 5, 50);
             } else {
                // the remote XBee did not receive our packet. is it powered on?
                flashLed(errorLed, 3, 500);
             }
          }      
      } else if (xbee.getResponse().isError()) {
        //nss.print("Error reading packet.  Error code: ");  
        //nss.println(xbee.getResponse().getErrorCode());
        // or flash error led
      } else {
        // local XBee did not provide a timely TX Status Response.  Radio is not configured properly or connected
        flashLed(errorLed, 2, 50);
      }
}


void loop() {


      
      
    float tempF = dht.readTemperature(true);
    float hum = dht.readHumidity();

    //if(tempF != currentTempF || currentHumidity != hum){
      currentTempF = tempF;
      currentHumidity = hum;
      transmitData();
    //}       
      
   
    delay(30000);
}
