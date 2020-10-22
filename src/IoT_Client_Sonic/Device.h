#ifndef DEVICE
#define DEVICE

#include "stdafx.h"
#include "Network.h"

#define TRIG 3
#define ECHO 4

#define MAKER_CODE "HSSON"
#define PRODUCT_CODE "SONIC0001"
#define SERIAL "qwerasdfzxcv"
#define DEFAULT_NICKNAME "SONIC_DEVICE"

class Device {
public:
	Device();
	~Device();

	void run();

private:
	
};

#endif