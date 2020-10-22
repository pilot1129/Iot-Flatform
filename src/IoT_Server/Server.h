#ifndef SERVER
#define SERVER

#include "stdafx.h"
#include "LocalDB.h"
#include "tinyxml2.h"
#include "ODBC_Manager.h"

class Server {
public:
	Server();
	~Server();
	void start();

private:
	static void* run(void*); // TODO
	pthread_t * server_run_thread;

	//��ü �޽��� �ֻ������� ���� �޽��� Ÿ���� �ʿ�. 
	//���� ��� �޽����� ó�� �� accept packet �Ѱ��� ����. �ش� �޽��� �ޱ� ������ application wait���·� ����� ��.
	enum MSG_TYPE {ERROR, LOGIN, LOGOUT, REG_USER, REG_DEV, ENROLL_DEV, RMV_USR_DEV, SEND_DEV_LIST, SEL_DEV, CTRL_DEV, CHNG_USR_INFO, DEV_DATA}; //add DEV_DATA -> not need ack_data--------------------------------------
	enum PROCESS_RESULT {FAIL, SUCCESS};

	static void login(int fd, const tinyxml2::XMLNode* msg); //�α��� ��� <user_id, pw ������ �ʿ�>
	static void logout(int fd, const tinyxml2::XMLNode* msg); //�α׾ƿ� ��� <user_id �ʿ�>
	static void regist_new_user(int fd, const tinyxml2::XMLNode* msg); //ȸ������ <user_id, pw, user_name, user_email, phone_number �ʿ�>
	static void regist_new_device(int fd, const tinyxml2::XMLNode* msg); //�� ��� ��� <����� ��Ʈ>
	static void enroll_user_device(int fd, const tinyxml2::XMLNode* msg); //���� ����Ʈ�� Ư�� ����̽� ��� <user_id, dev_serial �ʿ�>
	static void remove_user_device(int fd, const tinyxml2::XMLNode* msg); //���� ����Ʈ���� Ư�� ����̽� ���� <user_id, dev_serial �ʿ�>
	static void send_device_list(int fd, const tinyxml2::XMLNode* msg); //������ �����ϰ� �ִ� ����̽� ����Ʈ ���� <user_id �ʿ�>
														//�ۿ��� �ʿ��� ������ ����� �ּ���.(ī������)

	static void select_device(int fd, const tinyxml2::XMLNode* msg); //Ư�� ����̽� ���ý� �ʿ��� ������ ���� <user_id, dev_serial �ʿ�>
	static void control_device(int fd, const tinyxml2::XMLNode* msg); //Ư�� ����̽� ��Ʈ�Ѹ�� ���� <user_id, dev_serial �ʿ�>
	static void change_user_info(int fd, const tinyxml2::XMLNode* msg); //���� ���� ���� <pw, name, email, phone_number�ʿ�. ������� ������� ""(empty string)�� ǥ��>
	static void get_dev_data(const tinyxml2::XMLNode* msg);

	static void send_task_result_msg(int fd, MSG_TYPE packet_type, PROCESS_RESULT result);
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

/*
DB������ ��ϵ��� ���� Device�̸鼭 ��� �� ����� �� 3���� �̻� �� ����̽��� �����Ѵ�.
*/