#include <cstdio>
#include <cstring>

#include "mqtt_control.h"
#include <mosquittopp.h>

mqtt_control::mqtt_control(const char *id, const char *user_name, const char *pass_word, 
						const char *host, int port, std::string topics_sub) 
	: mosquittopp(id)
{
	int keepalive = 60;
	size_t pos = 0;
	std::string delimiter = ":";
	while ((pos = topics_sub.find(delimiter)) != std::string::npos) {
		topics.push_back(topics_sub.substr(0, pos));
		topics_sub.erase(0, pos + delimiter.length());
	}
	// last topic
	topics.push_back(topics_sub.substr(0, pos));

	username_pw_set(user_name, pass_word);
	/* Connect immediately. This could also be done by calling
	 * mqtt_control->connect(). */
	connect_async(host, port, keepalive);
};

mqtt_control::~mqtt_control()
{
}

void mqtt_control::on_connect(int rc)
{
	// printf("Connected with code %d.\n", rc);
	if(rc == 0){
		/* Only attempt to subscribe on a successful connect. */
		int reason_code = 0;
		for (std::string topic : topics)
		{
			reason_code = subscribe(NULL, topic.c_str(), 2);
			if(reason_code != MOSQ_ERR_SUCCESS){
				fprintf(stderr, "Error during subscribing: %s\n", mosqpp::strerror(reason_code));
				/* We might as well disconnect if we were unable to subscribe */
				disconnect();
			}
			else
			{
				printf("Subscribed [%s] successfully! \n", topic.c_str());
			}
		}
	}
	else
	{
		fprintf(stderr, "Error during connecting: %s\n", mosqpp::connack_string(rc));
	}
}

void mqtt_control::on_message(const struct mosquitto_message *message)
{
	char buf[51];

	if(!strcmp(message->topic, "hat")){
		memset(buf, 0, 51*sizeof(char));
		/* Copy N-1 bytes to ensure always 0 terminated. */
		memcpy(buf, message->payload, 50*sizeof(char));
		printf("[hat] Received %s \n", buf);
		disp_two_lines new_disp((const char*)message->payload);
		led_config_cur->set_disp(new_disp);
	}

	if(!strcmp(message->topic, "ctrl")){
		memset(buf, 0, 51*sizeof(char));
		/* Copy N-1 bytes to ensure always 0 terminated. */
		memcpy(buf, message->payload, 50*sizeof(char));
		printf("[ctrl] Received %s \n", buf);
	}
}

void mqtt_control::on_subscribe(int mid, int qos_count, const int *granted_qos)
{
	int i;
	bool have_subscription = false;

	/* In this example we only subscribe to a single topic at once, but a
	 * SUBSCRIBE can contain many topics at once, so this is one way to check
	 * them all. */
	for(i=0; i<qos_count; i++){
		printf("on_subscribe: %d :granted qos = %d\n", i, granted_qos[i]);
		if(granted_qos[i] <= 2){
			have_subscription = true;
		}
	}
	if(have_subscription == false){
		/* The broker rejected all of our subscriptions, we know we only sent
		 * the one SUBSCRIBE, so there is no point remaining connected. */
		fprintf(stderr, "Error: All subscriptions rejected.\n");
		disconnect();
	}
}

