#ifndef WEATHER_H
#define WEATHER_H

#include<weather.pb.h>

namespace messaging {
	namespace weather {

		WeatherMessage getWeatherMessage(float temperature, float humidity) {
			WeatherMessage msg = WeatherMessage_init_zero;

			msg.has_humidity = true;
			msg.has_temperatureF = true;
			msg.temperatureF = temperature;
			msg.humidity = humidity;

			return msg;
		}
		
	}
}


#endif // !WEATHER_H
