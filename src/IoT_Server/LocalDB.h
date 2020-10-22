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
네트워크 -> 로컬DB(fd, msg) -> 서버처리 -> 로컬DB(fd, msg) -> 네트워크
서버처리:
1. 패킷 타입 검사
2-1. 로그인: ODBC에 매치 후 로컬 DB에 등록 => 현재 연결된 디바이스들 중 해당 유저의 디바이스들이 있으면 링크
2-2. 로그아웃: 로컬DB에 등록된 해당 유저 제거(변경사항은 ODBC에 저장)
2-3. 유저 등록: ODBC에 중복 ID검사 후 유저 추가
2-4. 디바이스 등록: ODBC에 중복 serial검사 후 없으면 추가, 해당 디바이스 로컬DB에 메모리 등록(연결 등록)
2-5. 유저가 디바이스 목록 요청: 해당 유저의 디바이스 목록 및 상태(네트워크 상태 등) 반환
2-6. 유저가 특정 디바이스 선택: 유저에게 제어 권한이 있는지 확인하고 해당 디바이스의 스크린 데이터 전송(ODBC이용)
2-7. 유저가 특정 디바이스 제어: 유저에게 제어 권한이 있는지 확인하고 해당 디바이스에 제어 명령 전송
2-8. 디바이스로부터 데이터 전송: 데이터를 받으면 디바이스에만 갱신. 이후 해당 디바이스를 이용하는 유저들이 알아서 업데이트 사항이 있으면 전송(MQTT처럼 제어하자)
2-9. 유저 정보 변경: 유저 정보 파싱 후 ODBC에 갱신
3. 연결 끊김 시 유저와 디바이스 모두 자동으로 로그아웃 및 메모리 언로드
*/