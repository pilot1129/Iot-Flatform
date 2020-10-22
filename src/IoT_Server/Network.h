#ifndef NETWORK
#define NETWORK

#include "stdafx.h"
#include "LocalDB.h"

#define PORT 33265
#define PACKET_BUFFER_LEN 1024

class Network {
public:
	static Network* getInstance();
	static void destructInstance();

private:
	Network();
	~Network();
	static Network* Instance;
	static int server_fd;
	static std::vector<pthread_t> client_sess;
	static std::queue<pthread_t> garbage_thread;
	static bool Server_run;
	static sem_t garbage_num;
	static pthread_mutex_t garbage_mutex;
	static pthread_mutex_t client_sess_mutex;

	struct sockaddr_in server_addr;
	pthread_t listen_thread;
	pthread_t write_thread;
	pthread_t thread_garbage_collector;

	static void * write_packet(void *);
	static void * accept_packet(void *);
	static void * listen_packet(void * client_fd);
	static void * garbage_collector(void *);
};

#endif
//가비지 콜렉터 변경할 것
