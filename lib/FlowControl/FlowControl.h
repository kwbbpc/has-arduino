#ifndef FLOWCONTROL_H
#define FLOWCONTROL_H

#include <stdint.h>
#include <flowStatus.pb.h>


namespace flow{
	/**********************************************
	 *
	 * CONSTANTS
	 *
	 */

	const uint8_t NUMBER_VALVES = 4;


	/*********************************************
	 * DECLARATIONS
	 */
	namespace detail{

		struct ChangeTrackerSignal {

		private:
			bool isChanged;

		public:
			void signalChange() {
				isChanged = true;
			}
			bool wasThereAChange() {
				bool change = isChanged;
				isChanged = false;
				return change;
			}

		};

		ChangeTrackerSignal _changeTrackerSignal;

		struct ValveStatus {


			bool isOn;
			bool scheduledOn;
			long runTimeRemainingMs;
			uint8_t pinNumber;

			ValveStatus() {
				isOn = false;
				scheduledOn = false;
				runTimeRemainingMs = 0;
				pinNumber = 0xFF;
			}

			void turnOn() {
				if (runTimeRemainingMs != 0) {
					if (!isOn) {
						_changeTrackerSignal.signalChange();
					}
					
					isOn = true;
				}
			}


			void reset() {
				if (isOn) {
					_changeTrackerSignal.signalChange();
				}
				isOn = false;
				scheduledOn = false;
				runTimeRemainingMs = 0;
			}

			void run(long timeEllapsedSinceLastRun) {
				runTimeRemainingMs -= timeEllapsedSinceLastRun;
				if (runTimeRemainingMs <= 0) {

					if (isOn) {
						_changeTrackerSignal.signalChange();
					}

					runTimeRemainingMs = 0;
					isOn = false;
					scheduledOn = false;
				}
				else {

					if (!isOn) {
						_changeTrackerSignal.signalChange();
					}

					isOn = true;
				}
			}

		};

		struct MainValve {

			MainValve() {
				isOn = false;
				runTime = 0;
				maxRunTimeMs = 18000000; //5 hours
				pinNumber = 0xFF;
			}

			bool isRunTimeExceeded() {
				return(runTime >= maxRunTimeMs);
			}

			bool turnOn() {
				if (runTime < maxRunTimeMs) {

					if (!isOn) {
						_changeTrackerSignal.signalChange();
					}

					isOn = true;
				}
				else {

					if (isOn) {
						_changeTrackerSignal.signalChange();
					}

					isOn = false;
				}
				return isOn;
			}

			bool isOn;
			long runTime;
			uint8_t pinNumber;
			uint32_t maxRunTimeMs;

			void run(long timeEllapsedSinceLastRun) {
				turnOn();
				runTime += timeEllapsedSinceLastRun;
				if (runTime >= maxRunTimeMs) {
					
					if (isOn) {
						_changeTrackerSignal.signalChange();
					}
					
					
					//terminate
					isOn = false;
				}
			}

			void reset() {

				if (isOn) {
					_changeTrackerSignal.signalChange();
				}

				isOn = false;
				runTime = 0;
			}
		};
	}





	








	class FlowController {

	private:
		detail::MainValve mainValve;
		detail::ValveStatus valves[4];
		long lastExecutionTime;
		bool isStarted;

	public:

		FlowController(uint8_t v1, uint8_t v2, uint8_t v3, uint8_t v4, uint8_t main) {

			isStarted = false;
			lastExecutionTime = 0;
			valves[0].pinNumber = v1;
			valves[1].pinNumber = v2;
			valves[2].pinNumber = v3;
			valves[3].pinNumber = v4;


			mainValve.pinNumber = main;
		};
		
		detail::ValveStatus* getValves() {
			return valves;
		};

		detail::ValveStatus* getValveByPinNumber(uint32_t pinNumber) {

			for (int i = 0; i < NUMBER_VALVES; ++i) {
				if (pinNumber == valves[i].pinNumber) {
					return &valves[i];
				}
			}
			return 0;
		};


		void scheduleValveToRun(int pinNumber, uint32_t runTime) {
			detail::ValveStatus* valve = getValveByPinNumber(pinNumber);
			valve->scheduledOn = true;
			valve->runTimeRemainingMs = runTime;
		};

		void cancelRun(int pinNumber) {
			getValveByPinNumber(pinNumber)->reset();
		}


		void stop() {
			isStarted = false;
			lastExecutionTime = 0;
			mainValve.reset();
			for (int i = 0; i < NUMBER_VALVES; ++i) {
				valves[i].reset();
			}
		};

		detail::MainValve* getMainValve() {
			return &mainValve;
		}
		
		detail::ValveStatus* getActiveValve() {
			for (int i = 0; i < NUMBER_VALVES; ++i) {
				if (valves[i].isOn) {
					return &valves[i];
				}
			}
			return 0;
		};

		bool wasThereAChange() {
			return detail::_changeTrackerSignal.wasThereAChange();
		};

		void iterateExecution(long currentTime) {
			
			if (lastExecutionTime == 0) {
				lastExecutionTime = currentTime;
				//turn the valves on without running
				for (int i = 0; i < NUMBER_VALVES; ++i) {
					if (valves[i].scheduledOn && valves[i].runTimeRemainingMs > 0) {
						valves[i].run(0);
						mainValve.run(0);
						return; //return early so we only turn on 1 valve.
					}
				}

				return;
			}

			long elapsedTimeMs = currentTime - lastExecutionTime;
			lastExecutionTime = currentTime;


			//should the main valve be turned on?
			for (int i = 0; i < NUMBER_VALVES; ++i) {
				if (valves[i].scheduledOn && valves[i].runTimeRemainingMs > 0) {
					//turn on the main valve
					mainValve.run(elapsedTimeMs);
					break;
				}
			}


			//loop through valves and find one that's still scheduled with time remaining
			for (int i = 0; i < NUMBER_VALVES; ++i) {

				//if there's an active valve, skip to it to run it down
				if (getActiveValve() != 0) {
					if (getActiveValve()->pinNumber != valves[i].pinNumber) {
						continue;
					}
				}

				if (valves[i].scheduledOn && valves[i].runTimeRemainingMs > 0) {
					
					//turn this valve on
					if (!valves[i].isOn) {
						valves[i].turnOn();
					}
					else if(valves[i].isOn){
						valves[i].run(elapsedTimeMs);
					}

					//turn the main valve on unless it's max run time is exceeded
					mainValve.turnOn();

					//if no valves are on after running, we need to re-run to turn on the next valve.
					if (this->getActiveValve() == 0) {
						continue;
					}
					else {
						break;
					}
				}

				
			}

			//check for stop after all the runs have been updated.
			if (mainValve.isRunTimeExceeded()){
				stop();
				return;
			}

			for (int i = 0; i < NUMBER_VALVES; ++i) {
				if (valves[i].scheduledOn && valves[i].runTimeRemainingMs > 0) {
					//there's still more to run, don't stop yet.
					return;
				}
			}

			//no more valves are running.  turn things off.
			stop();
		};

		bool areValvesRunning() {
			for (int i = 0; i < NUMBER_VALVES; ++i) {
				if (valves[i].isOn) {
					return true;
				}
			}

			return false;
		}

		FlowStatusMessage getFlowStatusMessage(uint8_t pinNumber) {

			FlowStatusMessage msg = FlowStatusMessage_init_zero;

			detail::ValveStatus* vstatus = getValveByPinNumber(pinNumber);

			ValveStatus vmsg = ValveStatus_init_zero;
			vmsg.flowTimeRemainingMs = vstatus->runTimeRemainingMs;
			vmsg.has_flowTimeRemainingMs = true;
			vmsg.isOn = vstatus->isOn;
			vmsg.has_isOn = true;
			vmsg.isScheduled = vstatus->scheduledOn;
			vmsg.pinNumber = vstatus->pinNumber;
			
			msg.valveStatus = vmsg;
			
			msg.mainValveIsOn = mainValve.isOn;
			msg.mainValvePinNumber = mainValve.pinNumber;
			msg.mainValveRunTimeMs = mainValve.runTime;
			msg.maxRunTimeMs = mainValve.maxRunTimeMs;

			return msg;
		};

		

	};
	
};


#endif