#include <boost/test/unit_test.hpp>
#include "FlowControl.h"

BOOST_AUTO_TEST_SUITE(flow_test_suite)

class Checker{
public: 

	flow::FlowController* ctrl;
	long timestamp;
	long totalRunTimeMs;

	Checker(flow::FlowController* tctrl) {
		ctrl = tctrl;
		timestamp = 0;
		totalRunTimeMs = 0;
	}

	bool mainValveIsOff() {
		return !ctrl->getMainValve()->isOn;
	}

	bool mainValveIsOn() {
		return ctrl->getMainValve()->isOn;
	}

	bool valveIsOff(int pin) {
		return !ctrl->getValveByPinNumber(pin)->isOn;
	}

	bool valveIsOn(int pin) {
		return ctrl->getValveByPinNumber(pin)->isOn;
	}

	bool valveIsScheduled(int pin) {
		return ctrl->getValveByPinNumber(pin)->scheduledOn;
	}

	bool valveHasRunTimeRemaining(int pin) {
		return (ctrl->getValveByPinNumber(pin)->runTimeRemainingMs > 0);
	}

	bool valveHasMsRunTimeRemaining(int pin, long timeLeftMs) {
		return (ctrl->getValveByPinNumber(pin)->runTimeRemainingMs == timeLeftMs);
	}

	bool mainValveRunTimeShouldEqual(long runTimeMs) {
		return (ctrl->getMainValve()->runTime == runTimeMs);
	}

	bool checkAllValvesAreOff() {
		bool isOff = true;
		for (int i = 0; i < flow::NUMBER_VALVES; ++i) {
			isOff = isOff && !ctrl->getValves()[i].isOn;
		}
		return isOff;
	}

	bool checkAllOtherValvesAreOff(int* valves, int size) {
		bool isOff = true;
		for (int i = 0; i < size; ++i) {
			isOff = isOff && !ctrl->getValveByPinNumber(valves[i])->isOn;
		}
		return isOff;
	}

	bool noValvesAreScheduled() {
		bool isNotScheduled = true;
		for (int i = 0; i < flow::NUMBER_VALVES; ++i) {
			isNotScheduled = isNotScheduled && !ctrl->getValves()[i].scheduledOn;
		}
		return isNotScheduled;
	}

	long getTotalRunTime() {
		return totalRunTimeMs;
	}


	void advanceTime(long ms) {
		if (timestamp == 0) {
			timestamp = ms;
		}
		else {
			timestamp += ms;
			totalRunTimeMs += ms;
		}
		ctrl->iterateExecution(timestamp);
	}

	void resetRunTime() {
		totalRunTimeMs = 0;
	}

	bool checkForReset() {
		bool isReset = true;
		for (int i = 0; i < flow::NUMBER_VALVES; ++i) {
			flow::detail::ValveStatus* v = &(ctrl->getValves()[i]);
			isReset = isReset && !v->isOn;
			isReset = isReset && (v->runTimeRemainingMs == 0);
			isReset = isReset && (!v->scheduledOn);
		}
		isReset = isReset && !ctrl->getMainValve()->isOn;
		isReset = isReset && (ctrl->getMainValve()->runTime == 0);
		return isReset;
	}
};

BOOST_AUTO_TEST_CASE(test_single_valve)
{

	flow::FlowController controller(2, 3, 4, 5, 6);

	Checker chk(&controller);

	const long valveRunTimeMs = 10000;
	controller.scheduleValveToRun(2, valveRunTimeMs);

	flow::detail::ValveStatus* v2 = controller.getValveByPinNumber(2);

	//check that valve 2 is scheduled to start, but not actually on
	BOOST_REQUIRE(chk.valveIsOff(2));
	BOOST_REQUIRE(chk.valveIsScheduled(2));
	BOOST_REQUIRE(chk.valveHasRunTimeRemaining(2));

	BOOST_REQUIRE(chk.checkAllValvesAreOff());

	//check that the main valve is off
	BOOST_REQUIRE(chk.mainValveIsOff());

	//now start the system
	long startTimeMs = 1000;
	controller.iterateExecution(startTimeMs);

	//check that valve 2 is on
	BOOST_REQUIRE(chk.valveIsOn(2));
	int v[] = { 3,4,5 };
	BOOST_REQUIRE(chk.checkAllOtherValvesAreOff(v, 3));
	BOOST_REQUIRE(chk.valveIsScheduled(2));
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(2, valveRunTimeMs));

	//check that the main valve is on
	BOOST_REQUIRE(chk.mainValveIsOn());
	BOOST_REQUIRE(chk.mainValveRunTimeShouldEqual(0));

	//elapse time
	long timeAdvanceMs = 5000;
	controller.iterateExecution(timeAdvanceMs);
	BOOST_REQUIRE(chk.valveIsOn(2));
	BOOST_REQUIRE(chk.checkAllOtherValvesAreOff(v, 3));
	BOOST_REQUIRE(chk.valveIsScheduled(2));
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(2, valveRunTimeMs - (timeAdvanceMs - startTimeMs)));

	//check that the main valve is on
	BOOST_REQUIRE(chk.mainValveIsOn());
	BOOST_REQUIRE(chk.mainValveRunTimeShouldEqual(timeAdvanceMs - startTimeMs));


	//elapse time to end
	long timeAdvanceToEndMs = 50000;
	controller.iterateExecution(timeAdvanceToEndMs);
	BOOST_REQUIRE(chk.checkAllValvesAreOff());
	BOOST_REQUIRE(chk.noValvesAreScheduled());
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(2, 0));

	//check that the main valve is off
	BOOST_REQUIRE(chk.mainValveIsOff());
	BOOST_REQUIRE(chk.mainValveRunTimeShouldEqual(0));

	   
}

BOOST_AUTO_TEST_CASE(test_multi_valve)
{

	flow::FlowController controller(2, 3, 4, 5, 6);

	Checker chk(&controller);

	const long valveRunTimeMs = 10000;
	controller.scheduleValveToRun(2, valveRunTimeMs);
	controller.scheduleValveToRun(3, valveRunTimeMs);
	controller.scheduleValveToRun(4, valveRunTimeMs);
	controller.scheduleValveToRun(5, valveRunTimeMs);


	BOOST_REQUIRE(chk.checkAllValvesAreOff());
	BOOST_REQUIRE(chk.valveIsScheduled(2));
	BOOST_REQUIRE(chk.valveIsScheduled(3));
	BOOST_REQUIRE(chk.valveIsScheduled(4));
	BOOST_REQUIRE(chk.valveIsScheduled(5));
	BOOST_REQUIRE(chk.mainValveIsOff());

	chk.advanceTime(10000);
	
	flow::detail::ValveStatus* av = controller.getActiveValve();
	BOOST_REQUIRE(av->pinNumber == 2);
	BOOST_REQUIRE(av->isOn);
	int others[] = { 3,4,5 };
	BOOST_REQUIRE(chk.checkAllOtherValvesAreOff(others,3));
	chk.advanceTime(9000);
	BOOST_REQUIRE(av->isOn);
	BOOST_REQUIRE(chk.mainValveIsOn());
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(2, 1000));
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(3, valveRunTimeMs));
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(4, valveRunTimeMs));
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(5, valveRunTimeMs));
	BOOST_REQUIRE(chk.mainValveRunTimeShouldEqual(chk.getTotalRunTime()));
	BOOST_REQUIRE(chk.checkAllOtherValvesAreOff(others, 3));
	
	
	chk.advanceTime(2000);

	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(2, 0));
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(3, valveRunTimeMs));
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(4, valveRunTimeMs));
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(5, valveRunTimeMs));

	BOOST_REQUIRE(chk.mainValveRunTimeShouldEqual(chk.getTotalRunTime()));
	av = controller.getActiveValve();
	BOOST_REQUIRE(av->pinNumber == 3);
	BOOST_REQUIRE(av->isOn);
	int others2[] = { 2,4,5 };
	BOOST_REQUIRE(chk.checkAllOtherValvesAreOff(others2, 3));

	chk.advanceTime(1000);
	av = controller.getActiveValve();
	BOOST_REQUIRE(av->pinNumber == 3);
	BOOST_REQUIRE(av->isOn);
	BOOST_REQUIRE(chk.mainValveIsOn());
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(2, 0));
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(3, valveRunTimeMs-1000));
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(4, valveRunTimeMs));
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(5, valveRunTimeMs));
	BOOST_REQUIRE(chk.mainValveRunTimeShouldEqual(chk.getTotalRunTime()));
	BOOST_REQUIRE(chk.checkAllOtherValvesAreOff(others2, 3));

	chk.advanceTime(9000);
	av = controller.getActiveValve();
	BOOST_REQUIRE(av->pinNumber == 4);
	BOOST_REQUIRE(av->isOn);
	int others3[] = { 2,3,5 };
	BOOST_REQUIRE(chk.checkAllOtherValvesAreOff(others3, 3));

	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(2, 0));
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(3, 0));
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(4, valveRunTimeMs));
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(5, valveRunTimeMs));
	BOOST_REQUIRE(chk.mainValveRunTimeShouldEqual(chk.getTotalRunTime()));

	chk.advanceTime(10000);
	
	av = controller.getActiveValve();
	BOOST_REQUIRE(av->pinNumber == 5);
	BOOST_REQUIRE(av->isOn);
	int others4[] = { 2,3,4 };
	BOOST_REQUIRE(chk.mainValveIsOn());
	BOOST_REQUIRE(chk.checkAllOtherValvesAreOff(others4, 3));
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(2, 0));
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(3, 0));
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(4, 0));
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(5, valveRunTimeMs));
	BOOST_REQUIRE(chk.mainValveRunTimeShouldEqual(chk.getTotalRunTime()));
	
	
	//advance through the end, and verify everything is turned off.
	chk.advanceTime(10000);
	BOOST_REQUIRE(chk.checkAllValvesAreOff());
	BOOST_REQUIRE(chk.mainValveIsOff());
	BOOST_REQUIRE(chk.mainValveRunTimeShouldEqual(0));

}




BOOST_AUTO_TEST_CASE(test_multiple_executions)
{
	flow::FlowController controller(2, 3, 4, 5, 6);
	Checker chk(&controller);

	controller.scheduleValveToRun(2, 1000);
	controller.scheduleValveToRun(3, 1000);

	chk.advanceTime(10);
	chk.advanceTime(10);
	
	BOOST_REQUIRE(chk.mainValveIsOn());
	BOOST_REQUIRE(chk.valveIsOn(2));
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(2, 990));
	BOOST_REQUIRE(chk.valveIsOff(3));
	BOOST_REQUIRE(chk.valveIsScheduled(3));

	chk.advanceTime(990);

	int others7[] = { 2,4,5 };
	for (int i = 0; i < 9; ++i) {
		chk.advanceTime(100);
		BOOST_REQUIRE(chk.mainValveIsOn());
		BOOST_REQUIRE(chk.valveIsOn(3));
		BOOST_REQUIRE(controller.getActiveValve()->pinNumber == 3);
		BOOST_REQUIRE(chk.checkAllOtherValvesAreOff(others7, 3));
		BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(3, (1000 - (100 * (i + 1)))));
		BOOST_REQUIRE(chk.mainValveRunTimeShouldEqual(chk.getTotalRunTime()));
	}
	//advance the last 100 ms, which will turn everything off.
	chk.advanceTime(100);

	//check everything's reset and valid
	BOOST_REQUIRE(chk.checkForReset());
	

	controller.scheduleValveToRun(2, 1000);
	controller.scheduleValveToRun(5, 1000);

	//first advance will turn everything on
	chk.advanceTime(10);
	chk.resetRunTime();
	//next advance is the time elapsed from when things started.
	chk.advanceTime(10);

	BOOST_REQUIRE(chk.mainValveIsOn());
	BOOST_REQUIRE(chk.valveIsOn(2));
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(2, 990));
	int others4[] = { 3,4,5 };
	BOOST_REQUIRE(chk.mainValveIsOn());
	BOOST_REQUIRE(chk.checkAllOtherValvesAreOff(others4, 3));
	BOOST_REQUIRE(chk.valveIsScheduled(5));
	BOOST_REQUIRE(!chk.valveIsScheduled(3));

	chk.advanceTime(990);
	BOOST_REQUIRE(chk.valveIsOn(5));
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(5, 1000));
	int others5[] = { 2,3,4 };
	BOOST_REQUIRE(chk.mainValveIsOn());
	BOOST_REQUIRE(chk.checkAllOtherValvesAreOff(others5, 3));


	for (int i = 0; i < 9; ++i) {
		chk.advanceTime(100);

		BOOST_REQUIRE(chk.mainValveIsOn());
		BOOST_REQUIRE(chk.valveIsOn(5));
		BOOST_REQUIRE(controller.getActiveValve()->pinNumber == 5);
		BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(5, (1000 - (100*(i+1)))));
		BOOST_REQUIRE(chk.mainValveRunTimeShouldEqual(chk.getTotalRunTime()));
	}

	chk.advanceTime(100);

	BOOST_REQUIRE(chk.checkForReset());

}

BOOST_AUTO_TEST_CASE(test_main_valve_timeout)
{
	flow::FlowController ctrl(2, 3, 4, 5, 6);
	Checker chk(&ctrl);

	ctrl.scheduleValveToRun(2,10000000000000);
	chk.advanceTime(0);
	chk.advanceTime(17000000);
	BOOST_REQUIRE(chk.mainValveIsOn());
	chk.advanceTime(100000000);
	BOOST_REQUIRE(chk.mainValveIsOff());
	BOOST_REQUIRE(chk.valveIsOff(2));
	BOOST_REQUIRE(chk.checkAllValvesAreOff());
	BOOST_REQUIRE(chk.checkForReset());
	chk.advanceTime(100000000000);
	BOOST_REQUIRE(chk.checkAllValvesAreOff());
	BOOST_REQUIRE(chk.checkForReset());
}


BOOST_AUTO_TEST_CASE(test_schedule_during_execution)
{
	flow::FlowController ctrl(2,3,4,5,6);
	Checker chk(&ctrl);

	ctrl.scheduleValveToRun(2, 1000);
	chk.advanceTime(10);
	chk.advanceTime(500);

	BOOST_REQUIRE(ctrl.getActiveValve()->pinNumber == 2);

	ctrl.scheduleValveToRun(3, 1000);
	chk.advanceTime(0);
	chk.advanceTime(10);

	BOOST_REQUIRE(chk.valveIsScheduled(3));
	BOOST_REQUIRE(chk.valveIsOff(3));
	BOOST_REQUIRE(chk.valveIsOn(2));
	BOOST_REQUIRE(ctrl.getActiveValve()->pinNumber == 2);
	BOOST_REQUIRE(chk.mainValveIsOn());
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(2, 490));
	BOOST_REQUIRE(chk.mainValveRunTimeShouldEqual(chk.getTotalRunTime()));

	chk.advanceTime(100);
	BOOST_REQUIRE(chk.valveIsScheduled(3));
	BOOST_REQUIRE(chk.valveIsOff(3));
	BOOST_REQUIRE(ctrl.getActiveValve()->pinNumber == 2);
	BOOST_REQUIRE(chk.valveIsOn(2));
	BOOST_REQUIRE(chk.mainValveIsOn());
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(2, 390));
	BOOST_REQUIRE(chk.mainValveRunTimeShouldEqual(chk.getTotalRunTime()));


	chk.advanceTime(400);
	BOOST_REQUIRE(chk.valveIsOff(2));
	BOOST_REQUIRE(chk.valveIsScheduled(3));
	BOOST_REQUIRE(chk.valveIsOn(3));
	BOOST_REQUIRE(ctrl.getActiveValve()->pinNumber == 3);
	BOOST_REQUIRE(chk.mainValveIsOn());
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(3, 1000));
	BOOST_REQUIRE(chk.mainValveRunTimeShouldEqual(chk.getTotalRunTime()));


	chk.advanceTime(100000);
	BOOST_REQUIRE(chk.checkAllValvesAreOff());
	BOOST_REQUIRE(chk.mainValveIsOff());
	BOOST_REQUIRE(chk.checkForReset());
	BOOST_REQUIRE(ctrl.getActiveValve() == 0);
}


BOOST_AUTO_TEST_CASE(test_cancel)
{
	flow::FlowController ctrl(2, 3, 4, 5, 6);
	Checker chk(&ctrl);

	ctrl.scheduleValveToRun(2, 1000);
	ctrl.scheduleValveToRun(3, 1000);
	chk.advanceTime(100);
	chk.advanceTime(100);

	ctrl.cancelRun(2);
	chk.advanceTime(100);
	chk.advanceTime(100);
	BOOST_REQUIRE(chk.valveIsOff(2));
	BOOST_REQUIRE(chk.valveIsScheduled(3));
	BOOST_REQUIRE(chk.valveIsOn(3));
	BOOST_REQUIRE(ctrl.getActiveValve()->pinNumber == 3);
	BOOST_REQUIRE(chk.mainValveIsOn());
	BOOST_REQUIRE(chk.mainValveRunTimeShouldEqual(chk.getTotalRunTime()));

	ctrl.cancelRun(3);
	chk.advanceTime(10);
	BOOST_REQUIRE(chk.valveIsOff(2));
	BOOST_REQUIRE(chk.valveIsOff(3));
	BOOST_REQUIRE(ctrl.getActiveValve() == 0);
	BOOST_REQUIRE(chk.mainValveIsOff());
	BOOST_REQUIRE(chk.mainValveRunTimeShouldEqual(0));



}


BOOST_AUTO_TEST_CASE(new_execution) {

	flow::FlowController ctrl(2, 3, 4, 5, 6);
	Checker chk(&ctrl);

	ctrl.scheduleValveToRun(2, 5000);
	ctrl.scheduleValveToRun(3, 5000);
	ctrl.scheduleValveToRun(4, 5000);
	ctrl.scheduleValveToRun(5, 5000);

	for (int i = 0; i < 15000; ++i) {
		chk.advanceTime(1);
	}


	BOOST_REQUIRE(chk.valveIsOn(4));
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(4, 1));

	ctrl.scheduleValveToRun(2, 5000);
	ctrl.scheduleValveToRun(4, 5000);

	//valve 4 is currently running, has 1ms left.  It's just been reset to 5000ms
	BOOST_REQUIRE(chk.valveIsOn(4));
	BOOST_REQUIRE(chk.valveHasMsRunTimeRemaining(4, 5000));
	int otherValves[] = { 2, 3, 5 };
	BOOST_REQUIRE(chk.checkAllOtherValvesAreOff(otherValves, 3));
	BOOST_REQUIRE(ctrl.getActiveValve()->pinNumber == 4);
	BOOST_REQUIRE(chk.valveIsScheduled(2));
	BOOST_REQUIRE(chk.valveIsScheduled(5));

	for (int i = 0; i < 4000; ++i) {
		chk.advanceTime(1);
	}

	//make sure there's only one valve on.
	int v[] = { 2, 3, 5 };
	BOOST_REQUIRE(chk.checkAllOtherValvesAreOff(v, 3));

	for (int i = 0; i < 5000; ++i) {
		chk.advanceTime(1);
	}


	int v2[] = { 2, 3, 4 };
	BOOST_REQUIRE(chk.checkAllOtherValvesAreOff(v2, 3));
	//add additional checks here.
	BOOST_REQUIRE(chk.valveIsOn(5));
	BOOST_REQUIRE(ctrl.getActiveValve()->pinNumber == 5);
	BOOST_REQUIRE(chk.checkAllOtherValvesAreOff(v2, 3));
	BOOST_REQUIRE(chk.valveIsScheduled(2));


}


BOOST_AUTO_TEST_CASE(test_for_change_alerts) {

	flow::FlowController ctrl(2, 3, 4, 5, 6);
	Checker chk(&ctrl);

	chk.advanceTime(1);
	ctrl.scheduleValveToRun(2, 5000);
	ctrl.scheduleValveToRun(4, 5000);

	chk.advanceTime(1);

	//there should be an alert here
	BOOST_REQUIRE(ctrl.wasThereAChange());

	//now it should be cleared.

	for (int i = 0; i < 2000; ++i) {
		chk.advanceTime(1);
		BOOST_REQUIRE(!ctrl.wasThereAChange());
	}

	//schedule another run
	ctrl.scheduleValveToRun(3, 5000);

	for (int i = 0; i < 2999; ++i) {
		chk.advanceTime(1);
		BOOST_REQUIRE(!ctrl.wasThereAChange());
	}

	//should be ready for a change now
	chk.advanceTime(1);
	BOOST_REQUIRE(ctrl.wasThereAChange());


}



BOOST_AUTO_TEST_SUITE_END()
