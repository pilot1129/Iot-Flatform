#ifndef DEVICE
#define DEVICE

#include "stdafx.h"
#include "Network.h"

#define LED_PORT 2
#define COMMAND "LED_ON"

#define MAKER_CODE "HSSON"
#define PRODUCT_CODE "LED0001"
#define SERIAL "1q2w3e4r"
#define DEFAULT_NICKNAME "LED_DEVICE"

class Device {
public:
	Device();
	~Device();

	void run();

private:
	
};

#endif