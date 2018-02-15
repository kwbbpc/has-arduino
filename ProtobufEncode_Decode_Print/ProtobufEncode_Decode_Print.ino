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


ZBTxStatusResponse txStatus = ZBTxStatusResponse();

/*
This example is for Series 2 XBee
 Sends a ZB TX request with the value of analogRead(pin5) and checks the status response for success
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


   
      uint8_t payload[128];
      size_t message_length;
   // start transmitting after a startup delay.  Note: this will rollover to 0 eventually so not best way to handle
    if (millis() - start > 15000) {
      // break down 10-bit reading into two bytes and place in payload
 {     sensors.requestTemperatures();
      
      // allocate two bytes for to hold a 10-bit analog reading
      float temp = sensors.getTempFByIndex(0);
      Serial.println("Temp off sensor:");
      Serial.print(temp);

    
      WeatherMessage msg = WeatherMessage_init_zero;
      pb_ostream_t stream = pb_ostream_from_buffer(payload, sizeof(payload));
      msg.temperatureF = 99;
      msg.has_temperatureF = true;

      bool status = pb_encode(&stream, WeatherMessage_fields, &msg);
      message_length = stream.bytes_written;
      Serial.print("Encode Status: ");
      Serial.println(status);
      Serial.print("Value before decode: ");
      Serial.println(msg.temperatureF);
      /* Then just check for any errors.. */
 }
{
      /* Allocate space for the decoded message. */
        WeatherMessage messageDecoded = WeatherMessage_init_zero;
        
        /* Create a stream that reads from the buffer. */
        pb_istream_t streamD = pb_istream_from_buffer(payload, message_length);
        
        /* Now we are ready to decode the message. */
        bool status = pb_decode(&streamD, WeatherMessage_fields, &messageDecoded);

Serial.print("Decode Status: ");
      Serial.println(status);

        Serial.println("Weather Message");
        Serial.println(messageDecoded.temperatureF);
}
/*
      if (!status)
      {
          Serial.println("Encoding failed");
      }else{

          XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, 0x405C133A);
          ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
          
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
           */
      }

    delay(1000);
}
