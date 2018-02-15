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
#include <OneWire.h>
#include <DallasTemperature.h>

#include <pb_encode.h>
#include <pb_decode.h>
#include "weather.pb.h"

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);


TxStatusResponse txStatus = TxStatusResponse();

/*
This example is for Series 1 XBee
Sends a TX16 or TX64 request with the value of analogRead(pin5) and checks the status response for success
Note: In my testing it took about 15 seconds for the XBee to start reporting success, so I've added a startup delay
*/

XBee xbee = XBee();

unsigned long start = millis();


int statusLed = 11;
int errorLed = 12;

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
  sensors.begin();
  xbee.setSerial(Serial);
}

void loop() {
   
   // start transmitting after a startup delay.  Note: this will rollover to 0 eventually so not best way to handle
    if (millis() - start > 15000) {
      // break down 10-bit reading into two bytes and place in payload
      sensors.requestTemperatures();
      
      // allocate two bytes for to hold a 10-bit analog reading
      //String temp(sensors.getTempFByIndex(0), 2);
      float temp = 777.7;

    
      uint8_t payload[128];
      WeatherMessage msg = WeatherMessage_init_zero;
      pb_ostream_t stream = pb_ostream_from_buffer(payload, sizeof(payload));
      msg.temperatureF = temp;

      bool status = pb_encode(&stream, WeatherMessage_fields, &msg);
      size_t message_length = stream.bytes_written;
      
      /* Then just check for any errors.. */
      if (!status)
      {
          Serial.println("Encoding failed");
      }else{
        Serial.println("Done");
        String str = (char*)payload;
        Serial.println(str);
      }

      
      //String payloadStr = "TMP," + temp;
      //payloadStr.getBytes(payload, sizeof(payload));
      
      // with Series 1 you can use either 16-bit or 64-bit addressing
      // 16-bit addressing: Enter address of remote XBee, typically the coordinator
      //Tx16Request tx = Tx16Request(0x1634, payload, sizeof(payload));
      


      
      //xbee.send(tx);

      // flash TX indicator
      //flashLed(statusLed, 1, 100);
    }
  
    // after sending a tx request, we expect a status response
    // wait up to 5 seconds for the status response
    if (xbee.readPacket(5000)) {
        // got a response!

        // should be a znet tx status              
      if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
         xbee.getResponse().getTxStatusResponse(txStatus);
        
         // get the delivery status, the fifth byte
           if (txStatus.getStatus() == SUCCESS) {
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
    
    delay(1000);
}
