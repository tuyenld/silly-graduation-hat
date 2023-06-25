# CFLAGS=-Wall -ggdb -I../../lib -I../../lib/cpp
# LDFLAGS=-L../../lib ../../lib/cpp/libmosquittopp.so.1 ../../lib/libmosquitto.so.1
LDFLAGS=-lmosquittopp

.PHONY: all clean

all : mqtt_control

mqtt_control : main.o mqtt_control.o
	${CXX} $^ -o $@ ${LDFLAGS}

main.o : main.cpp
	${CXX} -c $^ -o $@ ${CFLAGS}

mqtt_control.o : mqtt_control.cpp
	${CXX} -c $^ -o $@ ${CFLAGS}

clean :
	-rm -f *.o mqtt_control
