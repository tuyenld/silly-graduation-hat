
Install MQTT broker and client on the VPS server

```bash
# Install Mosquitto MQTT broker
sudo apt-get --yes install mosquitto mosquitto-clients
sudo systemctl start mosquitto
systemctl status mosquitto
mosquitto -h # mosquitto version 1.5.7; MQTT v3.1.1

```

Install MQTT client on Raspberry PI Zero

```bash
sudo apt-get --yes install mosquitto-clients
```

Clone **Practical-Python-Programming-for-IoT** repository

```bash
git clone https://github.com/PacktPublishing/Practical-Python-Programming-for-IoT.git pyiot
```

Configure mqtt broker. You need to change `http_dir` path. The content of file `mosquitto_pyiot.conf` is as follows. You may need to allow port `1883` and `8083` to pass through your sever firewall. If your server is using `ufw`, the command below can be used.

```bash
sudo ufw allow 1883
sudo ufw allow 8083
```

For an unknown reason, mosquitto did not work with my user directory, so I need to move `mosquitto_www` into a new directory.

```
sudo mv /home/tuyenld/mqtt-python/pyiot/chapter04/mosquitto_www /etc/mosquitto/conf.d/mosquitto_www
```

Add username/password configuration.

```bash
cd /etc/mosquitto/conf.d/
mosquitto_passwd -c pass piZero # password is pihat :)
```


```bash
# sudo vi /etc/mosquitto/conf.d/mosquitto_pyiot.conf
persistence true

# MQTT Protocol
listener 1883 0.0.0.0
# password file created above
password_file /etc/mosquitto/conf.d/pass
protocol mqtt

# Web Sockets Protocol
listener 8083 0.0.0.0
protocol websockets
http_dir /etc/mosquitto/conf.d/mosquitto_www
```

Restart mosquitto broker and test connection.

```bash
sudo systemctl restart mosquitto
# make sure that the new configuration is applied
systemctl status mosquitto

# Test connention
wget http://localhost:8083
Remember that MQTT uses ports 1883 and 8883 (for SSL).
```

## Test

```bash
# [subscriber] Terminal1 of Raspberry Pi 
# Using username and password created
mosquitto_sub -u piZero -P pihat -v -h 104.248.243.162 -t 'hat'

# [publisher] Terminal2 of VPS server (MQTT broker)
# the message 'hello' should be received at Terminal1
mosquitto_pub -u piZero -P pihat -h 104.248.243.162 -t 'hat' -m 'hello'
```

## Debug

```bash
sudo cat /var/log/mosquitto/mosquitto.log

systemctl status mosquitto

# Or alternatively, execute the following command to start Mosquitto manually
# which will display any startup or configurations errors in the Terminal
sudo mosquitto -v -c /etc/mosquitto/mosquitto.conf

# clear log file
sudo truncate -s 0  /var/log/mosquitto/mosquitto.log
```

# cpp

## Install on Raspberry Pi
```bash

# https://mosquitto.org/blog/2013/01/mosquitto-debian-repository/
sudo apt-get install mosquitto

# https://jpinjpblog.wordpress.com/2017/12/18/installing-mqtt-for-c-on-raspberry-pi/
sudo apt-get install libmosquitto-dev libmosquittopp-dev libssl-dev

sudo vi /etc/mosquitto/conf.d/websocket.conf
listener 1883
listener 1884
protocol websockets

sudo service mosquitto restart

# As long as you make your code with the following flags.
# -l mosquittopp
```

## Libs

- [mosquittopp.h](https://github.com/eclipse/mosquitto/blob/master/lib/cpp/mosquittopp.h)
- [mosquitto.h](https://mosquitto.org/api/files/mosquitto-h.html)

# Python programming

## Raspberry Pi configuration

```bash
sudo apt install python3-pip
sudo apt-get install python3-venv
python3 -m venv hat-env
source hat-env/bin/activate

pip install paho-mqtt
```

- `-q2` QOS 2: The message will be delivered exactly once.
- `-c` --disable-clean-session: keep messages while I am offline.

```bash
# subscriber
mosquitto_sub -c -q2 -u piZero -P pihat -v -h 104.248.243.162 -t 'hat'

# publisher
mosquitto_pub -u piZero -P pihat -h 104.248.243.162 -t 'hat' -m 'hello'



mosquitto_pub -h test.mosquitto.org -t "example/temperature" -m 'hello'

```

The MQTT client is single [threaded](https://stackoverflow.com/a/54572567), it will only receive and process one message at a time

