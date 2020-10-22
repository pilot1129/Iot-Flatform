### 사용법
* IoT_Server - Linux ubuntu 환경기반으로 동작한다.

네트워크 config정보는 Network.h내에 define으로 존재한다.

	1)iodbc를 설치한다.
	2)iodbc가 설치된 폴더 위의 .odbc.ini를 연다.
	3).odbc.ini를 아래와 같이 설정한다
		[ODBC Data Sources]
  		2 tibero6 = Tibero6 ODBC driver
  		3
  		4 [ODBC]
  		5 Trace = 1
  		6 TraceFile = /home/유저이름/iodbc/tmp/odbc.trace
  		7
  		8 [tibero6]
  		9 Driver  = /home/ubuntu/tibero6/client/lib/libtbodbc.so
 		10 Description = Tibero6 ODBC Datasource
 		11
 		12 server = 121.165.2.17 #DB 컴퓨터의 외부 IP 번호
		13 port = 32323 #DB 컴퓨터의 포트번호
 		14 database = tibero
	4) /etc 의 profile을 연다.
	5) 아래와 같이 설정한다.
		#tibero setting
 		32 export TB_HOME=/home/유저이름/tibero6
		33 export TB_SID=tibero
 		34 export LD_LIBRARY_PATH=TB_HOME/lib
 		35 export PATH=$PATH:$TB_HOME/bin
 		36
 		37 #iodbc setting
 		38 export IODBC_HOME=$home/ 유저이름 /iodbc
 		39 export LD_LIBRARY_PATH=$IODBC_HOME/lib:$LD_LIBRARY_PATH
		40 export PATH=$PATH:$IODBC_HOME/bin:$PATHh"

	6)소스 디렉토리 내의 CMakeList.txt를 연 후 [LINK_DIRECTORIES]의 경로를 현재 자신이 설치한 iodbc의 라이브러리 경로로 설정한다.
	7)"cmake CMakeList.txt"명령어를 실행한다 -> Makefile 생성
	8)"make"명령어를 실행하여 out파일을 빌드한다.

* IoT_Client(LED, Sonic)

네트워크 config정보는 Network.h내에 define으로 존재한다.

	1) "cmake CMakeList.txt"명령어를 실행한다 -> Makefile 생성
	2) "make"명령어를 실행하여 out파일을 빌드한다.

