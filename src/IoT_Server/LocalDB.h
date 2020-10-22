#ifndef LOCAL_DB
#define LOCAL_DB

#include "stdafx.h"
#include "User.h"
#include "Device.h"

class LocalDB {
public:
	static LocalDB* getInstance();

	void add_user(User user);
	void del_user(std::string id);

	void add_device(Device dev);

	User* get_user(std::string user_id);
	Device* get_device(std::string dev_id);

	void push_recv_msg(std::pair<int, std::string> msg);
	std::pair<int, std::string> pop_recv_msg();

	void push_send_msg(std::pair<int, std::string> msg);
	std::pair<int, std::string> pop_send_msg();

	void disconnect_client(int* fd);

private:
	static LocalDB* Instance;
	LocalDB();

	pthread_mutex_t devices_mutex;
	pthread_mutex_t users_mutex;
	pthread_mutex_t recv_msg_mutex;
	pthread_mutex_t send_msg_mutex;
	sem_t recv_msg_semaphore;
	sem_t send_msg_semaphore;
	int server_ready;

	std::queue<std::pair<int, std::string>> recv_msg;
	std::queue<std::pair<int, std::string>> send_msg;

	std::map<std::string, User> users;
	std::map<std::string, Device> devices;
	
};

#endif

/*
��Ʈ��ũ -> ����DB(fd, msg) -> ����ó�� -> ����DB(fd, msg) -> ��Ʈ��ũ
����ó��:
1. ��Ŷ Ÿ�� �˻�
2-1. �α���: ODBC�� ��ġ �� ���� DB�� ��� => ���� ����� ����̽��� �� �ش� ������ ����̽����� ������ ��ũ
2-2. �α׾ƿ�: ����DB�� ��ϵ� �ش� ���� ����(��������� ODBC�� ����)
2-3. ���� ���: ODBC�� �ߺ� ID�˻� �� ���� �߰�
2-4. ����̽� ���: ODBC�� �ߺ� serial�˻� �� ������ �߰�, �ش� ����̽� ����DB�� �޸� ���(���� ���)
2-5. ������ ����̽� ��� ��û: �ش� ������ ����̽� ��� �� ����(��Ʈ��ũ ���� ��) ��ȯ
2-6. ������ Ư�� ����̽� ����: �������� ���� ������ �ִ��� Ȯ���ϰ� �ش� ����̽��� ��ũ�� ������ ����(ODBC�̿�)
2-7. ������ Ư�� ����̽� ����: �������� ���� ������ �ִ��� Ȯ���ϰ� �ش� ����̽��� ���� ��� ����
2-8. ����̽��κ��� ������ ����: �����͸� ������ ����̽����� ����. ���� �ش� ����̽��� �̿��ϴ� �������� �˾Ƽ� ������Ʈ ������ ������ ����(MQTTó�� ��������)
2-9. ���� ���� ����: ���� ���� �Ľ� �� ODBC�� ����
3. ���� ���� �� ������ ����̽� ��� �ڵ����� �α׾ƿ� �� �޸� ��ε�
*/