#include <FlowControl.h>
#include <flowStatus.pb.h>
#include <flowCommand.pb.h>
#include <Weather.h>
#include <NanoPbCodec.h>
#include <MemoryFree.h>

#include <DHT.h>
#include <XBee.h>


#define DHTTYPE DHT11
#define DHT11_PIN 2

flow::FlowController flowCtrl(2,3,4,5,6);



// create reusable response objects for responses we expect to handle 
ZBRxResponse rx = ZBRxResponse();
ZBTxStatusResponse txStatus = ZBTxStatusResponse();
XBee xbee = XBee();
DHT dht(DHT11_PIN, DHTTYPE);


void transmitData(messaging::Message* m){

    if (!m->isSuccessEncoded)
    {
        Serial.println("Encoding failed");
    }else{

        //Address of the receiving basestation XBee
        XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, 0x405C133A);

        /* REQUIRED: DO NOT SENT MORE BYTES THAN WHATS ACTUALLY IN THE PAYLOAD MESSAGE WRITTEN TO THE BUFFER! */
        /* DO NOT SEND THE WHOLE BUFFER SIZE! */
        ZBTxRequest zbTx = ZBTxRequest(addr64, m->payload, m->message_length);
        
        xbee.send(zbTx);
  
      }
    
      // after sending a tx request, we expect a status response
      // wait up to 5 seconds for the status response
      if (xbee.readPacket(500)) {
          // got a response!
  
          // should be a znet tx status              
        if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
           xbee.getResponse().getZBTxStatusResponse(txStatus);
          
          }      
      } else if (xbee.getResponse().isError()) {
        //nss.print("Error reading packet.  Error code: ");  
        //nss.println(xbee.getResponse().getErrorCode());
        // or flash error led
      } else {
        // local XBee did not provide a timely TX Status Response.  Radio is not configured properly or connected
        Serial.println("Error sending message.");
      }
}




void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  flowCtrl.iterateExecution(millis());

  /*******************************************************/
  // TEST
  flowCtrl.scheduleValveToRun(2, 5000);
  flowCtrl.scheduleValveToRun(3, 5000);
  flowCtrl.scheduleValveToRun(4, 5000);
  flowCtrl.scheduleValveToRun(5, 5000);
  /*******************************************************/
  
}


void updateFlowExecution(){

  flowCtrl.iterateExecution(millis());
  //update pins
  digitalWrite(flowCtrl.getMainValve()->pinNumber, flowCtrl.getMainValve()->isOn);
  flow::detail::ValveStatus* valves = flowCtrl.getValves();
  for(int i=0; i<4; ++i){
    if(valves[i].isOn){
    Serial.print("Valve "); Serial.print(valves[i].pinNumber); Serial.print(" is "); Serial.println(valves[i].isOn);
    }
    digitalWrite(valves[i].pinNumber, valves[i].isOn);
  }

  //write out statuses to xbee 
  /*
  if(flowCtrl.wasThereAChange()){
    Serial.println("Change detected, sending new message out.");
    //publish the flow status for each valve
    for(int i=0; i<flow::NUMBER_VALVES; ++i){
      uint8_t pinNumber = flowCtrl.getValves()[i].pinNumber;
      FlowStatusMessage msg = flowCtrl.getFlowStatusMessage(pinNumber);
      messaging::Message* m = messaging::nanopb::encodeMessage(messaging::FLOW_STATUS_MESSAGE_ID, &msg, FlowStatusMessage_fields);
  
      //write the messages
      transmitData(m);

      delete m;
    }
  }
  */
}


void readForNewMessages(){

  xbee.readPacket();
  if(xbee.getResponse().isAvailable()){
    if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
        // got a zb rx packet
        
        // now fill our zb rx class
        xbee.getResponse().getZBRxResponse(rx);

        uint8_t* data = rx.getData();
        uint8_t size = rx.getDataLength();

        Serial.print("Data: [");
        for(int i=0; i<size; ++i){
          Serial.print(data[i]);
          Serial.print(",");
        }
        Serial.println("]");
        Serial.println("done");

        int type = data[0];
        if(type == messaging::FLOW_CONTROL_MESSAGE_ID){
          Serial.println("Got flow control message.");
          FlowCommandMessage msg;
          bool success = messaging::nanopb::decodeMessage(rx.getData(), rx.getDataLength(), FlowCommandMessage_fields, msg);
          if(success){
            Serial.print("Turning valve on pin ");
            Serial.print(msg.pinNumber);
            Serial.print(" to ");
            Serial.print(msg.isOn);
            Serial.print(" for ");
            Serial.println(msg.runTimeMs);
            
  
            if(msg.isOn){
              flowCtrl.scheduleValveToRun(msg.pinNumber, msg.runTimeMs);
            }else if(!msg.isOn)
              flowCtrl.cancelRun(msg.pinNumber);
          }else{
            Serial.println("Message decoding failed.");
          }
          
        }
        }else{
          Serial.println("Unknown message recieved.");
        }
    }
  

  return 0;
}


long lastExecTime = 0;
void loop() {
  

  //listen for xbee commands
  readForNewMessages();

  //process the command and update the flow control execution
  updateFlowExecution();


  long millisTime = millis();
  //update temperature every 30ish seconds
  if((millisTime - lastExecTime) > 30 * 1000){
    lastExecTime = millisTime;

    //WeatherMessage weather = messaging::weather::getWeatherMessage(dht.readTemperature(), dht.readHumidity());
    //messaging::Message* weatherEncoded = messaging::nanopb::encodeMessage(messaging::WEATHER_MESSAGE_ID, &weather, WeatherMessage_fields);
    //transmitData(weatherEncoded);

    //update soil

  }else if((millisTime - lastExecTime) < 0){
    lastExecTime = 0;
  }


}
