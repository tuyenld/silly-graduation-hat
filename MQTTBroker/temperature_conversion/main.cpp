#include "mqtt_control.h"

int main(int argc, char *argv[])
{
	class mqtt_control *led_control;
	mosqpp::lib_init();

	led_control = new mqtt_control("led_control", "piZero", "pihat", "104.248.243.162", 1883, "hat:ctrl");
	led_control->loop_start();

	while(true)
	{
		sleep(1);
	}
	mosqpp::lib_cleanup();

	return 0;
}

