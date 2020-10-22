#include "Device.h"

Device::Device() {
	wiringPiSetupGpio();
	pinMode(TRIG, OUTPUT);
	pinMode(ECHO, INPUT);
	Network::getInstance()->set_device(MAKER_CODE, PRODUCT_CODE, SERIAL, DEFAULT_NICKNAME);
}


Device::~Device() {
}

void Device::run() {
	while (1) {
		digitalWrite(TRIG, LOW);
		digitalWrite(TRIG, LOW);
		delayMicroseconds(2);
		digitalWrite(TRIG, HIGH);
		delayMicroseconds(10);
		digitalWrite(TRIG, LOW);

		int start_time, end_time;
		while (digitalRead(ECHO) == 0)
			start_time = micros();
		while (digitalRead(ECHO) == 1)
			end_time = micros();

		unsigned long duration = end_time - start_time;

		float distance = ((float)(340 * duration) / 10000) / 2;

		std::cout << distance << std::endl;
		delay(100);
		Network::getInstance()->socket_send(std::to_string(distance));
	}
}
