#ifndef USER
#define USER

#include "Client.h"
#include "Device.h"

class User :
	public Client {
public:
	User();
	User(int fd, std::string id);
	User(int fd, std::string id, std::vector<Device> DeviceList);

	void set_fd(int fd);
	void set_id(std::string id);

	int get_fd() const;
	std::string get_id() const;

	void add_device(Device dev);
	bool delete_device(std::string id);
	const std::vector<Device> * get_device_list();
	const Device* find_device(std::string id);

private:
	std::vector<Device> device_list;
};

#endif