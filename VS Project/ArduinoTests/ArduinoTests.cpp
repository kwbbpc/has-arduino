// ArduinoTests.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <boost/test/unit_test.hpp>
#include <pb.h>
#include <pb_encode.h>
#include <pb_decode.h>

#include "weather.pb.h"
#include "NanoPbCodec.h"

/*
int main()
{

	WeatherMessage msg = WeatherMessage_init_zero;
	msg.temperatureF = 99.999;
	msg.humidity = 77.777;
	/* REQUIRED: MARK Optional Fields as included or not included! */
/*	msg.has_temperatureF = true;
	msg.has_humidity = true;

	messaging::Message* m = messaging::nanopb::encodeMessage<WeatherMessage>(messaging::WEATHER_MESSAGE_ID, &msg, WeatherMessage_fields);

	WeatherMessage decoded = WeatherMessage_init_zero;

	bool success = messaging::nanopb::decodeMessage < WeatherMessage>(m->payload, m->message_length, WeatherMessage_fields, decoded);



}


*/