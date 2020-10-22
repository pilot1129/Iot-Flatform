#include "ODBC_Manager.h"

ODBC_Manager* ODBC_Manager::Instance = nullptr;

ODBC_Manager * ODBC_Manager::getInstance() {
	if (Instance == nullptr)
		Instance = new ODBC_Manager();
	return Instance;
}


bool ODBC_Manager::DBConnect() {
	/* Env Handle */
	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

	/* Tibero Connect */
	rc = SQLConnect(hdbc,
		(SQLCHAR *)"tibero6", SQL_NTS, //Data Source Name or DB Name
					(SQLCHAR *)"HSDJ", SQL_NTS, // User
					(SQLCHAR *)"tibero", SQL_NTS); // Password

	if (rc != SQL_SUCCESS) {
		fprintf(stderr, "Connection failed! \n");
		exit(1);
	}

	/* Statements Handle */
	SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
}

void ODBC_Manager::DBDisConnect() {
	/* Release Handle and Close Connection */
	SQLFreeStmt(hstmt, SQL_DROP);
	SQLDisconnect(hdbc);
	SQLFreeConnect(hdbc);
	SQLFreeEnv(henv);
}

bool ODBC_Manager::DBExecuteSQL() {
	if (SQLExecute(hstmt) != SQL_SUCCESS)
		return false;
	return true;
}

bool ODBC_Manager::DBPrepare(SQLCHAR* qry) {
	if (SQLPrepareA(hstmt, qry, SQL_NTS) != SQL_SUCCESS)
		return false;
	return true;
}

bool ODBC_Manager::DBExecuteSQL(SQLCHAR* qry) {
	if (SQLExecDirect(hstmt, qry, SQL_NTS) != SQL_SUCCESS)
		return false;
	return true;
}

bool ODBC_Manager::Connect_DB() {
	return DBConnect();
}

void ODBC_Manager::DisConnect_DB() {
	DBDisConnect();
}

bool ODBC_Manager::login(std::string id, std::string pwd) {

	if (Connect_DB() != SQL_SUCCESS) {
		printf("Connection failed!\n");
		return false;
	}

	bool check = false;
	SQLCHAR Name[10];
	SQLLEN i_name;

	CHAR* ID = new char[id.size() + 1];
	std::copy(id.begin(), id.end(), ID);
	ID[id.size()] = '\0';

	char* PW = new char[pwd.size() + 1];
	std::copy(pwd.begin(), pwd.end(), PW);
	PW[pwd.size()] = '\0';

	// DBConnect(); // connect Function already call
	DBPrepare((SQLCHAR*)"Select name from users where id = ? and password = ?");

	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)ID, 0, NULL);
	SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)PW, 0, NULL);

	if (!DBExecuteSQL()) {
		printf("Login Execute Error!\n");
		DisConnect_DB();
		delete[] ID, PW;
		return false;
	}

	SQLBindCol(hstmt, 1, SQL_CHAR, Name, sizeof(Name), &i_name); // find user name(exist/ no exist)
	while (SQLFetch(hstmt) != SQL_NO_DATA) {
		check = true; break;
	}
	if (hstmt)
		SQLCloseCursor(hstmt); // solve cursur ????

	//DBDisConnect();
	delete[] ID, PW;
	DisConnect_DB();
	if (check) // if find
		return true;
	else // if find failed
		return false;
}

bool ODBC_Manager::GetDevice(std::string id, std::vector<Device> *DeviceList) {

	if (Connect_DB() != SQL_SUCCESS) {
		printf("Connection failed!\n");
		return false;
	}

	bool check = false;
	int i = 0;
	SQLCHAR Serial_Number[30];
	SQLCHAR Device_Nickname[30];
	SQLLEN i_S_Number, i_D_Nickname;

	CHAR* ID = new char[id.size() + 1];
	std::copy(id.begin(), id.end(), ID);
	ID[id.size()] = '\0';

	DBPrepare((SQLCHAR*)"Select device_nickname, serial_number from device where user_id = ?");
	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)ID, 0, NULL);

	if (!DBExecuteSQL()) {
		printf("Device Link Execute Error!\n");
		delete[] ID;
		DisConnect_DB();
		return false;
	}

	SQLBindCol(hstmt, 1, SQL_CHAR, Device_Nickname, sizeof(Device_Nickname), &i_D_Nickname);
	SQLBindCol(hstmt, 2, SQL_CHAR, Serial_Number, sizeof(Serial_Number), &i_S_Number);

	while (SQLFetch(hstmt) != SQL_NO_DATA) {
		check = true;
		Device DevProduct; // copy data(Device)
		std::string DevN((const char*)Device_Nickname);
		std::string SerN((const char*)Serial_Number);

		// input data to map
		DevProduct.set_id(SerN);
		DevProduct.set_nickname(DevN);
		
		DeviceList->push_back(DevProduct);
	}

	if (hstmt)
		SQLCloseCursor(hstmt); // solve cursur

	if (!check) {
		printf("No Data\n");
		delete[] ID;
		DisConnect_DB();
		return false;
	}
	delete[] ID;
	DisConnect_DB();
	return true;
}

bool ODBC_Manager::Add_User(std::string id, std::string pw, std::string name, std::string email, std::string pn) {

	if (Connect_DB() != SQL_SUCCESS) {
		printf("Connection failed!\n");
		return false;
	}

	SQLCHAR RetID[40];
	SQLLEN i_RetID;
	bool check = false;
	//ID
	CHAR* ID = new char[id.size() + 1];
	std::copy(id.begin(), id.end(), ID);
	ID[id.size()] = '\0';
	//PW
	CHAR* PW = new char[pw.size() + 1];
	std::copy(pw.begin(), pw.end(), PW);
	PW[pw.size()] = '\0';

	CHAR* Name = new char[name.size() + 1];
	std::copy(name.begin(), name.end(), Name);
	Name[name.size()] = '\0';

	CHAR* EM = new char[email.size() + 1];
	std::copy(email.begin(), email.end(), EM);
	EM[email.size()] = '\0';

	CHAR* PN = new char[pn.size() + 1];
	std::copy(pn.begin(), pn.end(), PN);
	PN[pn.size()] = '\0';

	// Check User
	DBPrepare((SQLCHAR*)"Select id from users where id = ?");
	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)ID, 0, NULL);

	if (!DBExecuteSQL()) {
		printf("Add_User Check Execute Error!\n");
		DisConnect_DB();
		delete[] ID, PW, Name, EM, PN;
		return false;
	}

	SQLBindCol(hstmt, 1, SQL_CHAR, RetID, sizeof(RetID), &i_RetID);
	while (SQLFetch(hstmt) != SQL_NO_DATA) {
		check = true;
	}
	if (hstmt)
		SQLCloseCursor(hstmt);

	if (check) {
		printf("Already ID Exist!\n");
		DisConnect_DB();
		delete[] ID, PW, Name, EM, PN;
		return false;
	}

	// Add User
	DBPrepare((SQLCHAR*)"insert into users values(?,?,?,?,?)");
	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)ID, 0, NULL);
	SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)PW, 0, NULL);
	SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)Name, 0, NULL);
	SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)EM, 0, NULL);
	SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_INTEGER, SQL_INTEGER, 0, 0, (SQLCHAR*)PN, 0, NULL);

	if (!DBExecuteSQL()) {
		printf("Add User Execute Error!\n");
		DisConnect_DB();
		delete[] ID, PW, Name, EM, PN;
		return false;
	}
	if (hstmt)
		SQLCloseCursor(hstmt);

	DisConnect_DB();
	delete[] ID, PW, Name, EM, PN;
	return true;
}

bool ODBC_Manager::Add_Device(std::string serial_id, std::string maker_code, std::string product_code, std::string nickname) {

	if (Connect_DB() != SQL_SUCCESS) {
		printf("Connection failed!\n");
		return false;
	}

	// if nickname == "", please input NULL
	SQLCHAR RetID[40];
	SQLLEN i_RetID;
	SQLLEN ival;
	ival = SQL_NULL_DATA;
	bool check = false;

	//serial_ID
	CHAR* SID = new char[serial_id.size() + 1];
	std::copy(serial_id.begin(), serial_id.end(), SID);
	SID[serial_id.size()] = '\0';
	//Maker_code
	CHAR* M_code = new char[maker_code.size() + 1];
	std::copy(maker_code.begin(), maker_code.end(), M_code);
	M_code[maker_code.size()] = '\0';
	//product_code
	CHAR* P_code = new char[product_code.size() + 1];
	std::copy(product_code.begin(), product_code.end(), P_code);
	P_code[product_code.size()] = '\0';
	//nickname
	CHAR* Nick = new char[nickname.size() + 1];
	std::copy(nickname.begin(), nickname.end(), Nick);
	Nick[nickname.size()] = '\0';

	// Check Device
	DBPrepare((SQLCHAR*)"Select serial_number from device where serial_number = ?");
	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)SID, 0, NULL);

	if (!DBExecuteSQL()) {
		printf("Add_Device Check Execute Error!\n");
		DisConnect_DB();
		delete[] SID, M_code, P_code, Nick;
		return false;
	}

	SQLBindCol(hstmt, 1, SQL_CHAR, RetID, sizeof(RetID), &i_RetID);
	while (SQLFetch(hstmt) != SQL_NO_DATA) {
		check = true;
	}
	if (hstmt)
		SQLCloseCursor(hstmt);

	if (check) {
		printf("Already Device Exist!\n");
		DisConnect_DB();
		delete[] SID, M_code, P_code, Nick;
		return true;
	}

	// Add Device
	DBPrepare((SQLCHAR*)"insert into Device values(?,?,?,?,?)");
	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)SID, 0, NULL);
	SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 40, 0, NULL, 0, &ival);
	SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)M_code, 0, NULL);
	SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)P_code, 0, NULL);
	SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)Nick, 0, NULL);

	if (!DBExecuteSQL()) {
		printf("Add Device Execute Error!\n");
		DisConnect_DB();
		delete[] SID, M_code, P_code, Nick;
		return false;
	}
	if (hstmt)
		SQLCloseCursor(hstmt);

	delete[] SID, M_code, P_code, Nick;
	DisConnect_DB();
	return true;
}
bool ODBC_Manager::Add_Device_ownership(std::string user_id, std::string device_serial) {

	if (Connect_DB() != SQL_SUCCESS) {
		printf("Connection failed!\n");
		return false;
	}

	SQLCHAR RetID[40];
	SQLLEN i_RetID;
	bool check = false;
	//serial
	CHAR* SID = new char[device_serial.size() + 1];
	std::copy(device_serial.begin(), device_serial.end(), SID);
	SID[device_serial.size()] = '\0';
	//ID
	CHAR* ID = new char[user_id.size() + 1];
	std::copy(user_id.begin(), user_id.end(), ID);
	ID[user_id.size()] = '\0';

	// Check Device
	DBPrepare((SQLCHAR*)"Select serial_number from device where serial_number = ?");
	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)SID, 0, NULL);

	if (!DBExecuteSQL()) {
		printf("regist_ownership Check Execute Error!\n");
		DisConnect_DB();
		delete[] SID, ID;
		return false;
	}

	SQLBindCol(hstmt, 1, SQL_CHAR, RetID, sizeof(RetID), &i_RetID);
	while (SQLFetch(hstmt) != SQL_NO_DATA) {
		check = true;
	}
	if (hstmt)
		SQLCloseCursor(hstmt);

	if (!check) {
		printf("No device exist!\n");
		DisConnect_DB();
		delete[] SID, ID;
		return false;
	}

	// Add ownership
	DBPrepare((SQLCHAR*)"update device set user_id = ? where serial_number = ?");
	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)ID, 0, NULL);
	SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)SID, 0, NULL);


	if (!DBExecuteSQL()) {
		printf("regist_ownership Execute Error!\n");
		DisConnect_DB();
		delete[] SID, ID;
		return false;
	}

	if (hstmt)
		SQLCloseCursor(hstmt);

	delete[] SID, ID;
	return true;
}

bool ODBC_Manager::Remove_Device_ownership(std::string user_id, std::string device_serial) {

	if (Connect_DB() != SQL_SUCCESS) {
		printf("Connection failed!\n");
		return false;
	}

	SQLCHAR RetID[40];
	SQLLEN i_RetID;
	SQLLEN ival;
	ival = SQL_NULL_DATA;
	bool check = false;
	//serial
	CHAR* SID = new char[device_serial.size() + 1];
	std::copy(device_serial.begin(), device_serial.end(), SID);
	SID[device_serial.size()] = '\0';

	// Check Device
	DBPrepare((SQLCHAR*)"Select serial_number from device where serial_number = ?");
	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)SID, 0, NULL);

	if (!DBExecuteSQL()) {
		printf("remove_ownership Check Execute Error!\n");
		DisConnect_DB();
		delete[] SID;
		return false;
	}

	SQLBindCol(hstmt, 1, SQL_CHAR, RetID, sizeof(RetID), &i_RetID);
	while (SQLFetch(hstmt) != SQL_NO_DATA) {
		check = true;
	}
	if (hstmt)
		SQLCloseCursor(hstmt);

	if (!check) {
		printf("No device exist!\n");
		DisConnect_DB();
		delete[] SID;
		return false;
	}

	// Add ownership
	DBPrepare((SQLCHAR*)"update device set user_id = ? where serial_number = ?");
	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 40, 0, NULL, 0, &ival);
	SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)SID, 0, NULL);


	if (!DBExecuteSQL()) {
		printf("regist_ownership Execute Error!\n");
		DisConnect_DB();
		delete[] SID;
		return false;
	}

	if (hstmt)
		SQLCloseCursor(hstmt);

	delete[] SID;
	return true;
}

std::string ODBC_Manager::Get_ScreenData(std::string device_serial) {

	if (Connect_DB() != SQL_SUCCESS) {
		printf("Connection failed!\n");
		return NULL;
	}

	SQLCHAR Merchandise_code[30];
	SQLCHAR Screen_info[100];
	SQLLEN i_M_code, i_S_info;
	std::string SInfo;
	bool check = false;
	//serial
	CHAR* SID = new char[device_serial.size() + 1];
	std::copy(device_serial.begin(), device_serial.end(), SID);
	SID[device_serial.size()] = '\0';

	// Check SerialNumber to get merchandise_code
	DBPrepare((SQLCHAR*)"Select merchandise_code from device where serial_number = ?");
	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)SID, 0, NULL);

	if (!DBExecuteSQL()) {
		printf("ScreenData Device Check Execute Error!\n");
		DisConnect_DB();
		delete[] SID;
		return NULL;
	}

	SQLBindCol(hstmt, 1, SQL_CHAR, Merchandise_code, sizeof(Merchandise_code), &i_M_code);
	while (SQLFetch(hstmt) != SQL_NO_DATA) {
		check = true;
	}
	if (hstmt)
		SQLCloseCursor(hstmt);

	if (!check) {
		printf("No device exist!\n");
		DisConnect_DB();
		delete[] SID;
		return NULL;
	}

	check = false;
	// Add ownership
	DBPrepare((SQLCHAR*)"select screen_info from merchandise where merchandise_code = ?");
	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, Merchandise_code, 0, &i_M_code);

	if (!DBExecuteSQL()) {
		printf("Get ScreenData Execute Error!\n");
		DisConnect_DB();
		delete[] SID;
		return NULL;
	}

	SQLBindCol(hstmt, 1, SQL_CHAR, Screen_info, sizeof(Screen_info), &i_S_info);
	while (SQLFetch(hstmt) != SQL_NO_DATA) {
		check = true;
		SInfo = (const char*)Screen_info;
	}

	if (hstmt)
		SQLCloseCursor(hstmt);

	if (!check) {
		printf("No Data in ScreenData \n");
		DisConnect_DB();
		delete[] SID;
		return NULL;
	}
	DisConnect_DB();
	delete[] SID;
	return SInfo;
}

bool ODBC_Manager::Change_User_info(std::string id, std::string pw, std::string name, std::string email, std::string pn) {

	if (Connect_DB() != SQL_SUCCESS) {
		printf("Connection failed!\n");
		return false;
	}

	//ID
	CHAR* ID = new char[id.size() + 1];
	std::copy(id.begin(), id.end(), ID);
	ID[id.size()] = '\0';

	//Check change
	//pw part
	if (pw != "") {
		CHAR* PW = new char[pw.size() + 1];
		std::copy(pw.begin(), pw.end(), PW);
		PW[pw.size()] = '\0';

		DBPrepare((SQLCHAR*)"update users set password = ? where id = ?");
		SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)PW, 0, NULL);
		SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)ID, 0, NULL);

		if (!DBExecuteSQL()) {
			printf("Change User password Error!\n");
			DisConnect_DB();
			delete[] PW;
			return false;
		}
		if (hstmt)
			SQLCloseCursor(hstmt);
		delete[] PW;
	}

	//Name part
	if (name != "") {
		CHAR* Name = new char[name.size() + 1];
		std::copy(name.begin(), name.end(), Name);
		Name[name.size()] = '\0';

		DBPrepare((SQLCHAR*)"update users set name = ? where id = ?");
		SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)Name, 0, NULL);
		SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)ID, 0, NULL);

		if (!DBExecuteSQL()) {
			printf("Change User name Error!\n");
			DisConnect_DB();
			delete[] Name;
			return false;
		}
		if (hstmt)
			SQLCloseCursor(hstmt);
		delete[] Name;
	}

	//Email part
	if (email != "") {
		CHAR* EM = new char[email.size() + 1];
		std::copy(email.begin(), email.end(), EM);
		EM[email.size()] = '\0';

		DBPrepare((SQLCHAR*)"update users set email = ? where id = ?");
		SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)EM, 0, NULL);
		SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)ID, 0, NULL);

		if (!DBExecuteSQL()) {
			printf("Change User email Error!\n");
			DisConnect_DB();
			delete[] EM;
			return false;
		}
		if (hstmt)
			SQLCloseCursor(hstmt);
		delete[] EM;
	}

	//Phone number part
	if (pn != "") {
		CHAR* PN = new char[pn.size() + 1];
		std::copy(pn.begin(), pn.end(), PN);
		PN[pn.size()] = '\0';

		DBPrepare((SQLCHAR*)"update users set phone_number = ? where id = ?");
		SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)PN, 0, NULL);
		SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_CHAR, SQL_CHAR, 0, 0, (SQLCHAR*)ID, 0, NULL);

		if (!DBExecuteSQL()) {
			printf("Change User phone number Error!\n");
			DisConnect_DB();
			delete[] PN;
			return false;
		}
		if (hstmt)
			SQLCloseCursor(hstmt);
		delete[] PN;
	}

	DisConnect_DB();
	delete[] ID;
	return true;
}

void ODBC_Manager::ExecuteQuery(SQLCHAR * qry) {
	SQLRETURN rc = SQL_SUCCESS;
	SQLUINTEGER len;
	SQLHANDLE henv, hdbc, hstmt;
	SQLCHAR *sql = qry;
	//SQLCHAR *sql = (SQLCHAR *)"SELECT count(*) from USERS";
	char buf[128];

	/* Env Handle */
	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);


	/* Tibero Connect */

	rc = SQLConnect(hdbc,
		(SQLCHAR *)"tibero6", SQL_NTS, /* Data Source Name or DB NAME */
					(SQLCHAR *)"HSDJ", SQL_NTS, /* User */
					(SQLCHAR *)"tibero", SQL_NTS); /* Password */

	if (rc != SQL_SUCCESS) {
		fprintf(stderr, "Connection failed!!!\n");
		exit(1);
	}

	/* Statements Handle */
	SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	printf("Query: %s\n", sql);

	/* Execute Query */
	rc = SQLExecDirect(hstmt, sql, SQL_NTS);
	if (rc != SQL_SUCCESS) {
		fprintf(stderr, "SQLExecDirect failed!!!\n");
		exit(1);
	}

	/* Bind Result */
	SQLBindCol(hstmt, 1, SQL_C_CHAR, (SQLCHAR *)buf, 128, (long *)&len);

	/* Fetch Result */
	SQLFetch(hstmt);

	/* Release Handle and Close Connection */
	SQLFreeStmt(hstmt, SQL_DROP);
	SQLDisconnect(hdbc);
	SQLFreeConnect(hdbc);
	SQLFreeEnv(henv);
}

bool ODBC_Manager::ExistData(SQLCHAR * qry) {
	SQLRETURN rc = SQL_SUCCESS;
	SQLUINTEGER len;
	SQLHANDLE henv, hdbc, hstmt;
	SQLCHAR *sql = qry;
	//SQLCHAR *sql = (SQLCHAR *)"SELECT count(*) from USERS";
	char buf[128];

	/* Env Handle */
	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

	/* Tibero Connect */

	rc = SQLConnect(hdbc,
		(SQLCHAR *)"tibero6", SQL_NTS, /* Data Source Name or DB NAME */
					(SQLCHAR *)"HSDJ", SQL_NTS, /* User */
					(SQLCHAR *)"tibero", SQL_NTS); /* Password */

	if (rc != SQL_SUCCESS) {
		fprintf(stderr, "Connection failed!!!\n");
		exit(1);
	}

	/* Statements Handle */
	SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	printf("Query: %s\n", sql);

	/* Execute Query */
	rc = SQLExecDirect(hstmt, sql, SQL_NTS);
	if (rc != SQL_SUCCESS) {
		fprintf(stderr, "SQLExecDirect failed!!!\n");
		exit(1);
	}

	/* Bind Result */
	SQLBindCol(hstmt, 1, SQL_C_CHAR, (SQLCHAR *)buf, 128, (long *)&len);

	/* Fetch Result */
	if (SQLFetch(hstmt) == SQL_SUCCESS_WITH_INFO)
		return true;
	else
		return false;

	/* Release Handle and Close Connection */
	SQLFreeStmt(hstmt, SQL_DROP);
	SQLDisconnect(hdbc);
	SQLFreeConnect(hdbc);
	SQLFreeEnv(henv);
}

std::string ODBC_Manager::Varchar_Maker(std::string param) {
	return "\'" + param + "\'";
}

ODBC_Manager::ODBC_Manager() {

}