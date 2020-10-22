#ifndef NETWORK
#define NETWORK

#include "stdafx.h"

#define SERVER_IP "13.125.221.145"
#define SERVER_PORT 33265

#define BUF_SIZE 1024

enum MSG_TYPE { REG_DEV = 4, CTRL_DEV = 9, DEV_DATA = 11 };

class Network {
public:
	static Network* getInstance();
	void socket_send(std::string data, MSG_TYPE type = MSG_TYPE::DEV_DATA);
	std::string socket_recv();
	void set_device(std::string maker_code, std::string product_code, std::string serial, std::string nickname);
	
private:
	typedef struct Device_Info {
		std::string serial;
		std::string maker_code;
		std::string product_code;
		std::string nickname;
	} Device_Info;

	static Network* Instance;

	int sock_fd;
	struct sockaddr_in sock_addr;
	bool dev_reg;
	Device_Info dev_info;

	Network();
	~Network();

	void initConnect();
	std::string makeXML(std::string data, MSG_TYPE type);
	std::string parseXML(std::string XMLdata);
};

#endif