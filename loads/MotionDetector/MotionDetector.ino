#include <SPI.h>
//#include <map.h>


// defines pins numbers
const int trigPin = 2;
const int echoPin = 3;


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
          Serial.print("Difference between current distance (");
          Serial.print(getDistance());
          Serial.print(") and steady distance (");
          Serial.print(SteadyDistance);
          Serial.print(") is ");
          Serial.println(diff);
          
          if(diff >= TriggerThresholdDistance){
            //send message.
            Serial.println("Message send on trigger!");
            delay(3000);
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
