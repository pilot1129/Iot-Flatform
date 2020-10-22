#ifndef DEVICE
#define DEVICE

#include "Client.h"
class Device :
	public Client {
public:
	Device();
	Device(std::string id, std::string nickname);
	Device(int fd, std::string id, std::string nickname);
	void set_fd(int fd);
	void set_id(std::string id);
	void set_nickname(std::string nickname);
	void set_status(std::string status);

	int get_fd() const;
	std::string get_id() const;
	std::string get_nickname() const;
	std::string get_status() const;

private:
	std::string nickname;
	std::string device_status;
	
};

#endif