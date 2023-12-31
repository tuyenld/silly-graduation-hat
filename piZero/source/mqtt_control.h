#ifndef MQTT_CONTROL_H
#define MQTT_CONTROL_H

#include <mosquittopp.h>
#include <string>
#include <vector>
#include <unistd.h>
#include "led_config.h"

class mqtt_control : public mosqpp::mosquittopp
{
	public:
		mqtt_control(const char *id, const char *user_name, const char *pass_word, 
						const char *host, int port, std::string topics_sub);
		
		std::vector<std::string> topics;
		led_config *led_config_cur;

		~mqtt_control();

		void on_connect(int rc);
		void on_message(const struct mosquitto_message *message);
		void on_subscribe(int mid, int qos_count, const int *granted_qos);
};

#endif
