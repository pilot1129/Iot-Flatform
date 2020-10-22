#ifndef ODBC_MNG
#define ODBC_MNG

#include "stdafx.h"
#include "Device.h"

#include <stdio.h>
#include <stdlib.h>

class ODBC_Manager {
public:
	static ODBC_Manager* getInstance();

	bool Connect_DB();// DBConnect
	void DisConnect_DB(); // DBDisConnect
	bool login(std::string id, std::string pwd); //match 여부만 체크

	bool GetDevice(std::string id, std::vector<Device> *DeviceList); // get DeviceList for map
	bool Add_User(std::string id, std::string pw, std::string name, std::string email, std::string pn); // add user
	bool Add_Device(std::string serial_id, std::string maker_code, std::string product_code, std::string nickname);// add device
	bool Add_Device_ownership(std::string user_id, std::string device_serial); // change ownership Device
	bool Remove_Device_ownership(std::string user_id, std::string device_serial); // remove ownership Device
	std::string Get_ScreenData(std::string device_serial); // get screendata
	bool Change_User_info(std::string id, std::string pw, std::string name, std::string email, std::string pn); // change user's info

private:
	SQLRETURN rc = SQL_SUCCESS;
	SQLUINTEGER len;
	SQLHANDLE henv, hdbc, hstmt;
	std::vector<std::string> DeviceList;
	char buf[128];

	ODBC_Manager();
	static ODBC_Manager* Instance;

	bool DBConnect();
	void DBDisConnect();
	bool DBExecuteSQL(); // use with prepare
	bool DBPrepare(SQLCHAR* qry);
	bool DBExecuteSQL(SQLCHAR* qry);

	bool ExistData(SQLCHAR* qry);
	std::string Varchar_Maker(std::string param);
	void ExecuteQuery(SQLCHAR* qry);
};

#endif