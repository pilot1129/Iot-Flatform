#include "Client.h"

Client::Client(int fd, std::string id) {
	this->fd = fd;
	this->id = id;
}
