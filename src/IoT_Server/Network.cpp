#include "Network.h"

Network* Network::Instance = nullptr;
bool Network::Server_run = true;
int Network::server_fd = -1;
std::vector<pthread_t> Network::client_sess;
std::queue<pthread_t> Network::garbage_thread;
sem_t Network::garbage_num;
pthread_mutex_t Network::garbage_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Network::client_sess_mutex = PTHREAD_MUTEX_INITIALIZER;

Network * Network::getInstance() {
	if (Instance == nullptr)
		Instance = new Network();
	return Instance;
}

void Network::destructInstance() {
	delete Instance;
}

Network::Network() {
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		std::cout << "Server: Can't open server socket" << std::endl;
		exit(0);
	}

	memset(&this->server_addr, 0, sizeof(server_addr));

	this->server_addr.sin_family = AF_INET;
	this->server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->server_addr.sin_port = htons(PORT);

	int err;
	err = bind(server_fd, (struct sockaddr*) &server_addr, sizeof(server_addr));
	if (err < 0) {
		std::cout << "Server: Can't bind socket address" << std::endl;
		exit(0);
	}

	err = listen(server_fd, 5);
	if (err < 0) {
		std::cout << "Server: Can't listening" << std::endl;
		exit(0);
	}

	sem_init(&garbage_num, 0, 0);
	pthread_create(&this->listen_thread, NULL, accept_packet, NULL);
	pthread_create(&this->write_thread, NULL, write_packet, NULL);
	pthread_create(&this->thread_garbage_collector, NULL, garbage_collector, NULL);
}

Network::~Network() {
	pthread_cancel(listen_thread);
	pthread_cancel(write_thread);
	pthread_cancel(thread_garbage_collector);

	for (pthread_t th : client_sess) {
		void* result;
		pthread_cancel(th);
		pthread_join(th, &result);
	}
}

void * Network::write_packet(void *) {
	std::pair<int, std::string> msg;

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	while (1) {
		msg = LocalDB::getInstance()->pop_send_msg();
		write(msg.first, msg.second.c_str(), msg.second.size());
	}

	return nullptr;
}

void * Network::accept_packet(void *) {
	int* client_fd = nullptr;
	struct sockaddr_in client_addr;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_cleanup_push([](void * server_fd) {
								close(*((int*)server_fd));
							}, 
						 &server_fd);

	while (1) {
		socklen_t length = sizeof(client_addr);
		client_fd = new int;
		*client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &length);
		if (*client_fd < 0)
			std::cout << "Server: accept failed" << std::endl;
		else {
			pthread_t listen_thread;
			pthread_create(&listen_thread, NULL, listen_packet, client_fd);

			pthread_mutex_lock(&client_sess_mutex);
			client_sess.push_back(listen_thread);
			pthread_mutex_unlock(&client_sess_mutex);
		}
	}

	pthread_cleanup_pop(1);
	return nullptr;
}

void * Network::listen_packet(void * client_fd) {
	int sock_fd = *((int*)client_fd);
	char buffer[PACKET_BUFFER_LEN];

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_cleanup_push([](void* client_fd) {
								close(*((int*)client_fd));
							},
						client_fd);

	while (1) {
		memset(buffer, 0, sizeof(buffer));
		if (read(sock_fd, buffer, PACKET_BUFFER_LEN) <= 0)
			break;

		LocalDB::getInstance()->push_recv_msg(std::pair<int, std::string>(sock_fd, std::string(buffer)));
	}

	pthread_cleanup_pop(1);

	pthread_mutex_lock(&garbage_mutex);
	garbage_thread.push(pthread_self());
	pthread_mutex_unlock(&garbage_mutex);
	sem_post(&garbage_num);

	return client_fd;
}

void * Network::garbage_collector(void *) {
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	void* ret;
	while (1) {
		sem_wait(&garbage_num);
		pthread_t garbage = garbage_thread.front();
		for (int i = 0; i < client_sess.size(); i++) {
			if (client_sess[i] == garbage) {
				std::cout << "delete" << std::endl; //test
				pthread_mutex_lock(&client_sess_mutex);
				pthread_join(client_sess[i], &ret);
				LocalDB::getInstance()->disconnect_client((int*)ret);
				client_sess.erase(client_sess.begin() + i);
				pthread_mutex_unlock(&client_sess_mutex);
			}
		}
		pthread_mutex_lock(&garbage_mutex);
		garbage_thread.pop();
		pthread_mutex_unlock(&garbage_mutex);
	}
	return nullptr;
}