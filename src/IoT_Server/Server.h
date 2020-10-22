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

	//전체 메시지 최상위에는 하위 메시지 타입이 필요. 
	//또한 모든 메시지는 처리 후 accept packet 넘겨줄 것임. 해당 메시지 받기 전에는 application wait상태로 대기할 것.
	enum MSG_TYPE {ERROR, LOGIN, LOGOUT, REG_USER, REG_DEV, ENROLL_DEV, RMV_USR_DEV, SEND_DEV_LIST, SEL_DEV, CTRL_DEV, CHNG_USR_INFO, DEV_DATA}; //add DEV_DATA -> not need ack_data--------------------------------------
	enum PROCESS_RESULT {FAIL, SUCCESS};

	static void login(int fd, const tinyxml2::XMLNode* msg); //로그인 기능 <user_id, pw 데이터 필요>
	static void logout(int fd, const tinyxml2::XMLNode* msg); //로그아웃 기능 <user_id 필요>
	static void regist_new_user(int fd, const tinyxml2::XMLNode* msg); //회원가입 <user_id, pw, user_name, user_email, phone_number 필요>
	static void regist_new_device(int fd, const tinyxml2::XMLNode* msg); //새 기기 등록 <손희승 파트>
	static void enroll_user_device(int fd, const tinyxml2::XMLNode* msg); //유저 리스트에 특정 디바이스 등록 <user_id, dev_serial 필요>
	static void remove_user_device(int fd, const tinyxml2::XMLNode* msg); //유저 리스트에서 특정 디바이스 삭제 <user_id, dev_serial 필요>
	static void send_device_list(int fd, const tinyxml2::XMLNode* msg); //유저가 소유하고 있는 디바이스 리스트 전송 <user_id 필요>
														//앱에서 필요한 데이터 명시해 주세요.(카톡으로)

	static void select_device(int fd, const tinyxml2::XMLNode* msg); //특정 디바이스 선택시 필요한 데이터 전송 <user_id, dev_serial 필요>
	static void control_device(int fd, const tinyxml2::XMLNode* msg); //특정 디바이스 컨트롤명령 전송 <user_id, dev_serial 필요>
	static void change_user_info(int fd, const tinyxml2::XMLNode* msg); //유저 정보 변경 <pw, name, email, phone_number필요. 변경사항 없을경우 ""(empty string)로 표시>
	static void get_dev_data(const tinyxml2::XMLNode* msg);

	static void send_task_result_msg(int fd, MSG_TYPE packet_type, PROCESS_RESULT result);
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

/*
DB에서는 등록되지 않은 Device이면서 등록 및 연결된 지 3개월 이상 된 디바이스는 삭제한다.
*/