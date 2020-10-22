#ifndef CLIENT
#define CLIENT
#include "stdafx.h"

class Client {
public:
	Client(int fd, std::string id);
	virtual void set_fd(int fd) = 0;
	virtual void set_id(std::string id) = 0;

	virtual int get_fd() const = 0;
	virtual std::string get_id() const = 0;

protected:
	int fd;
	std::string id;
};

#endif