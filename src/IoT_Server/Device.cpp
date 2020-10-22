#include "Device.h"


Device::Device() : Client(-1, "") {
	this->nickname = "";
	this->device_status = "";
}

Device::Device(std::string id, std::string nickname) : Client(-1, id) {
	this->nickname = nickname;
	this->device_status = "";
}

Device::Device(int fd, std::string id, std::string nickname) : Client(fd, id) {
	this->nickname = nickname;
	this->device_status = "";
}

void Device::set_fd(int fd) {
	this->fd = fd;
}

void Device::set_id(std::string id) {
	this->id = id;
}

void Device::set_nickname(std::string nickname) {
	this->nickname = nickname;
}

void Device::set_status(std::string status) {
	this->device_status = status;
}

int Device::get_fd() const {
	return this->fd;	
}

std::string Device::get_id() const {
	return this->id;
}

std::string Device::get_nickname() const {
	return this->nickname;
}

std::string Device::get_status() const {
	return this->device_status;
}
