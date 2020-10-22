#include "Device.h"

Device::Device() {
	wiringPiSetupGpio();
	pinMode(LED_PORT, OUTPUT);
	Network::getInstance()->set_device(MAKER_CODE, PRODUCT_CODE, SERIAL, DEFAULT_NICKNAME);
}


Device::~Device() {
}

void Device::run() {
	while (1) {
		std::string msg = Network::getInstance()->socket_recv();

		std::cout << msg << std::endl;
		if (msg == "LED_ON") {
			digitalWrite(LED_PORT, HIGH);
		}
		else if (msg == "LED_OFF") {
			digitalWrite(LED_PORT, LOW);
		}
	}
}
