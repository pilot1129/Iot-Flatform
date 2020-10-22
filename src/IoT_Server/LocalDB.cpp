#include "LocalDB.h"

LocalDB* LocalDB::Instance = nullptr;

LocalDB * LocalDB::getInstance() {
	if (Instance == nullptr)
		Instance = new LocalDB();
	return Instance;
}

void LocalDB::add_user(User user) {
	pthread_mutex_lock(&this->users_mutex);
	auto is_exist_iter = this->users.find(user.get_id());
	if (is_exist_iter != users.end())
		is_exist_iter->second = user;
	else
		this->users[user.get_id()] = user;
	pthread_mutex_unlock(&this->users_mutex);
}

void LocalDB::del_user(std::string id) {
	pthread_mutex_lock(&this->users_mutex);
	this->users.erase(id);
	pthread_mutex_unlock(&this->users_mutex);
}

void LocalDB::add_device(Device dev) {
	pthread_mutex_lock(&this->devices_mutex);
	auto is_exist_iter = this->devices.find(dev.get_id());
	if (is_exist_iter != devices.end())
		is_exist_iter->second = dev;
	else
		this->devices[dev.get_id()] = dev;
	pthread_mutex_unlock(&this->devices_mutex);
}

User * LocalDB::get_user(std::string user_id) {
	return &this->users[user_id];
}

Device * LocalDB::get_device(std::string dev_id) {
	return &this->devices[dev_id];
}

void LocalDB::push_recv_msg(std::pair<int, std::string> msg) {
	if (server_ready)
		pthread_yield();

	pthread_mutex_lock(&this->recv_msg_mutex);
	this->recv_msg.push(msg);
	sem_post(&recv_msg_semaphore);
	pthread_mutex_unlock(&this->recv_msg_mutex);
}

std::pair<int, std::string> LocalDB::pop_recv_msg() {
	std::pair<int, std::string> msg;
	sem_wait(&this->recv_msg_semaphore);
	this->server_ready++;

	pthread_mutex_lock(&this->recv_msg_mutex);
	msg = this->recv_msg.front();
	this->recv_msg.pop();
	pthread_mutex_unlock(&this->recv_msg_mutex);

	this->server_ready--;
	
	return msg;
}

void LocalDB::push_send_msg(std::pair<int, std::string> msg) {
	pthread_mutex_lock(&this->send_msg_mutex);
	this->send_msg.push(msg);
	sem_post(&send_msg_semaphore);
	pthread_mutex_unlock(&this->send_msg_mutex);
}

std::pair<int, std::string> LocalDB::pop_send_msg() {
	std::pair<int, std::string> msg;
	sem_wait(&this->send_msg_semaphore);

	pthread_mutex_lock(&this->send_msg_mutex);
	msg = this->send_msg.front();
	this->send_msg.pop();
	pthread_mutex_unlock(&this->send_msg_mutex);

	return msg;
}

void LocalDB::disconnect_client(int * fd) {//TODO
	pthread_mutex_lock(&this->devices_mutex);
	for (auto iter = this->devices.begin(); iter != this->devices.end(); iter++) {
		if (iter->second.get_fd() == (*fd)) {
			this->devices.erase(iter);
			pthread_mutex_unlock(&this->devices_mutex);
			delete fd;
			return;
		}
	}
	pthread_mutex_unlock(&this->devices_mutex);
	
	pthread_mutex_lock(&this->users_mutex);
	for (auto iter = this->users.begin(); iter != this->users.end(); iter++) {
		if (iter->second.get_fd() == (*fd)) {
			this->users.erase(iter);
			pthread_mutex_unlock(&this->users_mutex);
			delete fd;
			return;
		}
	}
	pthread_mutex_unlock(&this->users_mutex);

	delete fd;
}

LocalDB::LocalDB() {
	this->devices_mutex = PTHREAD_MUTEX_INITIALIZER;
	this->users_mutex = PTHREAD_MUTEX_INITIALIZER;
	this->recv_msg_mutex = PTHREAD_MUTEX_INITIALIZER;
	this->send_msg_mutex = PTHREAD_MUTEX_INITIALIZER;

	sem_init(&this->recv_msg_semaphore, 0, 0);
	sem_init(&this->send_msg_semaphore, 0, 0);

	server_ready = 0;
}