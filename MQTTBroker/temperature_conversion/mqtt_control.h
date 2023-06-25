#ifndef TEMPERATURE_CONVERSION_H
#define TEMPERATURE_CONVERSION_H

#include <mosquittopp.h>
#include <string>
#include <vector>

class mqtt_control : public mosqpp::mosquittopp
{
	public:
		mqtt_control(const char *id, const char *user_name, const char *pass_word, 
						const char *host, int port, std::string topics_sub);
		
		std::vector<std::string> topics;
		~mqtt_control();

		void on_connect(int rc);
		void on_message(const struct mosquitto_message *message);
		void on_subscribe(int mid, int qos_count, const int *granted_qos);
};

#endif
