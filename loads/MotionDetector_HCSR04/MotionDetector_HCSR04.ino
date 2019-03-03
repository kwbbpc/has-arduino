#include <SPI.h>

#include <XBee.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include "motion_detect.pb.h"


// defines pins numbers
const int trigPin = 2;
const int echoPin = 3;



/******************************************* XBEE SECTION **************************************************************/


#define MOTIONDETECT_MESSAGE_ID 1

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


XBee xbee = XBee();

unsigned long start = millis();

Message getMessage(int steadyDistance, int triggeredDistance){

    uint8_t* payload = new uint8_t[128];
    Serial.println("Size of payload");
    Serial.println(sizeof(payload));
    MotionDetectMessage msg = MotionDetectMessage_init_zero;
    pb_ostream_t stream = pb_ostream_from_buffer(payload, 128);
    msg.expectedDistanceCm = steadyDistance;
    msg.triggeredDistanceCm = triggeredDistance;
   
    /* REQUIRED: MARK Optional Fields as included or not included! */
    msg.has_expectedDistanceCm = true;
    msg.has_triggeredDistanceCm = true;
    

    bool status = pb_encode(&stream, MotionDetectMessage_fields, &msg);
    Serial.print("encoding status");
    Serial.println(status);
    
    size_t payload_length = stream.bytes_written;

    //add on a motion detect message identifyer byte
    uint8_t id = MOTIONDETECT_MESSAGE_ID;

    uint8_t* readyPayload = new uint8_t[payload_length + 1];

    memcpy(&readyPayload[1], payload, payload_length);
    
    delete [] payload;

    
    readyPayload[0] = id;

    //format a struct and return it
    Message m;
    m.payload_length = payload_length;
    m.message_length = payload_length + 1;
    m.payload = readyPayload;
    m.isSuccessEncoded = status;

    
    return m;
  
}

void transmitData(int steadyDistance, int triggeredDistance){
    //Serial.print("Free Memory:");
    //Serial.println(freeMemory());
    //Serial.println("Transmitting new data");

    Message m = getMessage(steadyDistance, triggeredDistance);

    if (!m.isSuccessEncoded)
    {
        Serial.println("Encoding failed");
    }else{


        //Address of the receiving basestation XBee
        //XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, 0x405C133A);
        XBeeAddress64 addr64 = XBeeAddress64(0x00000000,   0x00000000);


        /* REQUIRED: DO NOT SENT MORE BYTES THAN WHATS ACTUALLY IN THE PAYLOAD MESSAGE WRITTEN TO THE BUFFER! */
        /* DO NOT SEND THE WHOLE BUFFER SIZE! */
        ZBTxRequest zbTx = ZBTxRequest(addr64, m.payload, m.message_length);
        
        xbee.send(zbTx);
        Serial.println("Done sending.");
  
      }
    
      // after sending a tx request, we expect a status response
      // wait up to 5 seconds for the status response
      if (xbee.readPacket(5000)) {
          // got a response!
          Serial.println("Got response back from send!");
          // should be a znet tx status              
        if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
          xbee.getResponse().getZBTxStatusResponse(txStatus);
           // get the delivery status, the fifth byte
             if (txStatus.getDeliveryStatus() == SUCCESS) {
                // success.  time to celebrate
                Serial.println("Successfully delivered.");
             } else {
                // the remote XBee did not receive our packet. is it powered on?
                Serial.println("Did not complete delivery.");
             }
          
        }      
      } else if (xbee.getResponse().isError()) {
        //nss.print("Error reading packet.  Error code: ");  
        //nss.println(xbee.getResponse().getErrorCode());
        // or flash error led
        Serial.print("Error reading packet.  Error:");
        Serial.println(xbee.getResponse().getErrorCode());
      } else {
        // local XBee did not provide a timely TX Status Response.  Radio is not configured properly or connected
        Serial.println("Local XBee didn't respond.  Not setup correctly.");
      }
}

/******************************************* XBEE SECTION **************************************************************/


long getDistance(){
  long duration;
  int distance;
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance= duration*0.034/2;
  return distance;
}


void setup() {
  // put your setup code here, to run once:
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  Serial.begin(9600); // Starts the serial communication



  xbee.setSerial(Serial);
  while(millis() - start < 15000){
      //loop and hold for setup to really complete
    }

}

/*
class State{

  public:
    virtual void execute() = 0;
  
};
*/



class StandbyState{


  private:
    long steadyDistance = 0;
    long currentDistance = 0;

  public:
    StandbyState(){
      
    }

    void resetSteadyStateDistance(){
      //get the current distance
      steadyDistance = getDistance();
      Serial.print("Resetting steady state distance: ");
      Serial.println(steadyDistance);

      return;
    }

    long execute(){

      Serial.println("Delaying for startup....");
      delay(2000);

      const int SteadyStateCutoff = 5;
      const int ResetStateCutoff = 3;

      const long DistanceDifferenceAllowableForSteadyState = 10;

      //get the steady state distance.
      resetSteadyStateDistance();

      int steadyStateCounter = 0;
      int resetStateCounter = 0;

      //loop
      while(true){
      
          //delay
          delay(2000);
    
          //get the distance again
          currentDistance = getDistance();
    
          //compare the distances
          long diff = currentDistance - steadyDistance;
          if(diff < 1){
            diff = diff * -1.0;
          }
          
          Serial.print("Distance difference while waiting for steady state: ");
          Serial.println(diff);
    
          //if it's less than the steady state requriement, increment the steady state counter.
          if(diff < DistanceDifferenceAllowableForSteadyState){
            ++steadyStateCounter;
          }
    
          //if the steady state counter is greater than cutoff, transition to next state.    
          if(steadyStateCounter == SteadyStateCutoff){
            Serial.println("Steady State found, transition to next state!");
            break;  
          }
    
          //if it's greater, continue waiting.
          if(diff >= DistanceDifferenceAllowableForSteadyState){
            ++resetStateCounter;
          }
    
          //if we've waiting for more than the cutoff, reset the steady distance.
          if(resetStateCounter == ResetStateCutoff){
            Serial.println("Steady state not met, resetting counters and distance to gain steady state.");
            resetSteadyStateDistance();
            steadyStateCounter = 0;
            resetStateCounter = 0;
          }

      }

      return steadyDistance;
      
    }
};


class WaitForDistanceTriggerState{


  private:

    long SteadyDistance;
    const long TriggerThresholdDistance = 10;

  public: 

    WaitForDistanceTriggerState(long steadyDistance){
      SteadyDistance = steadyDistance;
    }

    long getDistanceOffset(){
      long distance = getDistance();
      long diff = distance - SteadyDistance;
      if(diff < 0){
        diff = diff * -1.0;
      }
      return diff;
    }

    void execute(){

      while(true){
          //take frequent measurements and wait for a distance change
          
    
          //if the distance changes below trigger threshold, send a message and wait.
          long diff = getDistanceOffset();
          long triggeredDistance = getDistance();
          Serial.print("Difference between current distance (");
          Serial.print(triggeredDistance);
          Serial.print(") and steady distance (");
          Serial.print(SteadyDistance);
          Serial.print(") is ");
          Serial.println(diff);
          
          if(diff >= TriggerThresholdDistance){

            //check the difference again for confirmation
            long diff2 = getDistanceOffset();
            if(diff2 >= TriggerThresholdDistance){
                
                //send message.
                Serial.println("Message send on trigger!");
                transmitData(SteadyDistance, triggeredDistance);
                delay(3000);
                  
            }
            
            
          }else{
            continue;
          }
    
          //if the distance is still triggered, wait until the cutoff
          int triggeredResetCounter = 0;
          const int ResetTriggerCount = 60;
          diff = getDistanceOffset();
          while(diff >= TriggerThresholdDistance){
            ++triggeredResetCounter;
            Serial.print("Triggered reset counter is ");
            Serial.println(triggeredResetCounter);
            delay(1000);
            
            diff = getDistanceOffset();
            if(triggeredResetCounter == ResetTriggerCount){
              Serial.println("Reverting back to finding steady state.");
              //if the cutoff wait is met while distance is triggered, reset the steady state.
              return;
            }
            
          }
      }

      
      
    }
  
};




void loop() {

  //wait for steady state
  StandbyState* s = new StandbyState();

  long distance = s->execute();

  Serial.print("Steady distance is ");
  Serial.println(distance);

  delete s;

  //wait for changes in distance to send a message
  WaitForDistanceTriggerState* s2 = new WaitForDistanceTriggerState(distance);
  s2->execute();

  delete s2;
  

  Serial.println("Execution finished.");

}
