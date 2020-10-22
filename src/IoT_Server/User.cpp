#include "User.h"

User::User() : Client(-1, "") {}

User::User(int fd, std::string id) : Client(fd, id) {}

User::User(int fd, std::string id, std::vector<Device> device_list) : Client(fd, id) {
	this->device_list = device_list;
}

void User::set_fd(int fd) {
	this->fd = fd;
}

void User::set_id(std::string id) {
	this->id = id;
}

int User::get_fd() const {
	return this->fd;
}

std::string User::get_id() const {
	return this->id;
}

void User::add_device(Device dev) {
	for (int i = 0; i < this->device_list.size(); i++) {
		if (device_list[i].get_id() == dev.get_id())
			return;
	}

	this->device_list.push_back(dev);
}

bool User::delete_device(std::string id) {
	for (int i = 0; i < this->device_list.size(); i++) {
		if (this->device_list[i].get_id() == id) {
			this->device_list.erase(this->device_list.begin() + i);
			return true;
		}
	}
	return false;
}

const std::vector<Device> * User::get_device_list() {
	return &this->device_list;
}

const Device* User::find_device(std::string id) {
	for (int i = 0; i < this->device_list.size(); i++) {
		if (this->device_list[i].get_id() == id)
			return &this->device_list[i];
	}
	return nullptr;
}
