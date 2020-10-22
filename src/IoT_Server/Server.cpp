#include "Server.h"

using namespace tinyxml2;

std::string ReplaceAll(std::string &str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
	return str;
}

Server::Server() {}

void Server::start() {

	int cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
	
	this->server_run_thread = new pthread_t[cpu_num];
	for (int i = 0; i < cpu_num; i++) 
		pthread_create(&this->server_run_thread[i], NULL, run, NULL);

	void* ret;
	for (int i = 0; i < cpu_num; i++)
		pthread_join(this->server_run_thread[i], &ret);
}

Server::~Server() {
	delete this->server_run_thread;
}

void * Server::run(void *) {

	while (1) {
		std::pair<int, std::string> msg;
		msg = LocalDB::getInstance()->pop_recv_msg();

		int fd = msg.first;
		XMLDocument doc;
		if (doc.Parse(msg.second.c_str(), msg.second.size()) != XMLError::XML_SUCCESS) {
			std::cout << "xml Read Error!!" << std::endl << "msg: " << msg.second << std::endl;
			continue;
		}
		
		int msg_type = MSG_TYPE::ERROR;
		XMLNode* node = doc.FirstChild();

		while (node != nullptr && !(std::string(node->Value()) == "IoTPacket")) {
			node = node->NextSibling();
		}

		if (node != nullptr) {
			const XMLAttribute* packetType = node->ToElement()->FirstAttribute();
			while (packetType != nullptr) {
				if (std::string(packetType->Name()) == "PacketType")
					msg_type = packetType->IntValue();
				packetType = packetType->Next();
			}
			node = node->FirstChild();
		}

		std::cout << msg_type << std::endl;
		switch (msg_type) {
		case MSG_TYPE::LOGIN:
			login(fd, node);
			break;
		case MSG_TYPE::LOGOUT:
			logout(fd, node);
			break;
		case MSG_TYPE::REG_USER:
			regist_new_user(fd, node);
			break;
		case MSG_TYPE::REG_DEV:
			regist_new_device(fd, node);
			break;
		case MSG_TYPE::ENROLL_DEV:
			enroll_user_device(fd, node);
			break;
		case MSG_TYPE::RMV_USR_DEV:
			remove_user_device(fd, node);
			break;
		case MSG_TYPE::SEND_DEV_LIST:
			send_device_list(fd, node);
			break;
		case MSG_TYPE::SEL_DEV:
			select_device(fd, node);
			break;
		case MSG_TYPE::CTRL_DEV:
			control_device(fd, node);
			break;
		case MSG_TYPE::CHNG_USR_INFO:
			change_user_info(fd, node);
			break;
		case MSG_TYPE::DEV_DATA:
			get_dev_data(node);
		default:
			send_task_result_msg(fd, MSG_TYPE::ERROR, PROCESS_RESULT::FAIL);
			break;
		}
	}

	return nullptr;
}

void Server::login(int fd, const XMLNode * msg) {
	std::string id = "", pw = "";

	while (msg != nullptr && !(std::string(msg->Value()) == "LoginClient"))
		msg = msg->NextSibling();

	if (msg != nullptr) {
		for (auto attr = msg->ToElement()->FirstAttribute(); attr != nullptr; attr = attr->Next()) {
			if (std::string(attr->Name()) == "ID")
				id = attr->Value();
			else if (std::string(attr->Name()) == "PW")
				pw = attr->Value();
		}
	}

	if (msg != nullptr && ODBC_Manager::getInstance()->login(id, pw)) {
		std::vector<Device> dev_list;
		ODBC_Manager::getInstance()->GetDevice(id, &dev_list);
		LocalDB::getInstance()->add_user(User(fd, id, dev_list));

		send_task_result_msg(fd, MSG_TYPE::LOGIN, PROCESS_RESULT::SUCCESS);
		return;
	}

	send_task_result_msg(fd, MSG_TYPE::LOGIN, PROCESS_RESULT::FAIL);
}

void Server::logout(int fd, const XMLNode * msg) {
	std::string id = "";

	while (msg != nullptr && !(std::string(msg->Value()) == "LogoutClient"))
		msg = msg->NextSibling();

	if (msg != nullptr) {
		for (auto attr = msg->ToElement()->FirstAttribute(); attr != nullptr; attr = attr->Next()) {
			if (std::string(attr->Name()) == "ID")
				id = attr->Value();
		}
	}

	LocalDB::getInstance()->del_user(id);
	send_task_result_msg(fd, MSG_TYPE::LOGOUT, PROCESS_RESULT::SUCCESS);
}

void Server::regist_new_user(int fd, const XMLNode * msg) {
	std::string id = "", pw = "", name = "", email = "", phone_number = "";
	
	while (msg != nullptr && !(std::string(msg->Value()) == "RegUserClient"))
		msg = msg->NextSibling();

	if (msg != nullptr) {
		for (auto attr = msg->ToElement()->FirstAttribute(); attr != nullptr; attr = attr->Next()) {
			if (std::string(attr->Name()) == "ID")
				id = attr->Value();
			else if (std::string(attr->Name()) == "PW")
				pw = attr->Value();
			else if (std::string(attr->Name()) == "Name")
				name = attr->Value();
			else if (std::string(attr->Name()) == "Mobile")
				phone_number = attr->Value();
			else if (std::string(attr->Name()) == "Email")
				email = attr->Value();
		}
	}

	if(ODBC_Manager::getInstance()->Add_User(id, pw, name, email, phone_number))
		send_task_result_msg(fd, MSG_TYPE::REG_USER, PROCESS_RESULT::SUCCESS);
	else
		send_task_result_msg(fd, MSG_TYPE::REG_USER, PROCESS_RESULT::FAIL);
}

void Server::regist_new_device(int fd, const XMLNode * msg) {
	std::string id = "", maker_code = "", product_code = "", nickname = "";
	
	/*
	Xml_Parsing
	*/
	while (msg != nullptr && !(std::string(msg->Value()) == "Device"))
		msg = msg->NextSibling();

	if (msg != nullptr) {
		for (auto attr = msg->ToElement()->FirstAttribute(); attr != nullptr; attr = attr->Next()) {
			if (std::string(attr->Name()) == "ID")
				id = attr->Value();
			else if (std::string(attr->Name()) == "MakerCode")
				maker_code = attr->Value();
			else if (std::string(attr->Name()) == "ProductCode")
				product_code = attr->Value();
			else if (std::string(attr->Name()) == "nickName")
				nickname = attr->Value();
		}
	}

	/*
	ODBC_access => 중복 감지?
	*///
	if (!ODBC_Manager::getInstance()->Add_Device(id, maker_code, product_code, nickname)) {
		send_task_result_msg(fd, MSG_TYPE::REG_DEV, PROCESS_RESULT::FAIL);
		return;
	}

	LocalDB::getInstance()->add_device(Device(fd, id, nickname));
	send_task_result_msg(fd, MSG_TYPE::REG_DEV, PROCESS_RESULT::SUCCESS);
}

void Server::enroll_user_device(int fd, const XMLNode * msg) {
	std::string user_id = "", dev_id = "", nickname = "";

	while (msg != nullptr && !(std::string(msg->Value()) == "RegDeviceClient"))
		msg = msg->NextSibling();

	if (msg != nullptr) {
		for (auto attr = msg->ToElement()->FirstAttribute(); attr != nullptr; attr = attr->Next()) {
			if (std::string(attr->Name()) == "ID")
				user_id = attr->Value();
			else if (std::string(attr->Name()) == "Serial")
				dev_id = attr->Value();
			else if (std::string(attr->Name()) == "Alias")
				nickname = attr->Value();
		}
	}

	//user check//
		//ODBC device check//
	User* user = LocalDB::getInstance()->get_user(user_id);
	if (user != nullptr && user->get_fd() == fd && ODBC_Manager::getInstance()->Add_Device_ownership(user_id, dev_id)) {
		//user device_regist//
		user->add_device(Device(dev_id, nickname));
		send_task_result_msg(fd, MSG_TYPE::ENROLL_DEV, PROCESS_RESULT::SUCCESS);
		return;
	}
	send_task_result_msg(fd, MSG_TYPE::ENROLL_DEV, PROCESS_RESULT::FAIL);
}

void Server::remove_user_device(int fd, const XMLNode * msg) {
	std::string user_id = "", dev_id = "";
	
	while (msg != nullptr && !(std::string(msg->Value()) == "RmvDeviceClient"))
		msg = msg->NextSibling();

	if (msg != nullptr) {
		for (auto attr = msg->ToElement()->FirstAttribute(); attr != nullptr; attr = attr->Next()) {
			if (std::string(attr->Name()) == "ID")
				user_id = attr->Value();
			else if (std::string(attr->Name()) == "Serial")
				dev_id = attr->Value();
		}
	}

	/*
	user check
	*///
	User* user = LocalDB::getInstance()->get_user(user_id);
	if (user == nullptr || user->find_device(dev_id) == nullptr) {
		send_task_result_msg(fd, MSG_TYPE::RMV_USR_DEV, PROCESS_RESULT::FAIL);
		return;
	}

	/*
	ODBC_access &
			device del
	*///
	if(ODBC_Manager::getInstance()->Remove_Device_ownership(user_id, dev_id))
		user->delete_device(dev_id);
}

void Server::send_device_list(int fd, const XMLNode * msg) {
	std::string user_id = "";
	
	while (msg != nullptr && !(std::string(msg->Value()) == "ReqDevListClient"))
		msg = msg->NextSibling();

	if (msg != nullptr) {
		for (auto attr = msg->ToElement()->FirstAttribute(); attr != nullptr; attr = attr->Next()) {
			if (std::string(attr->Name()) == "ID")
				user_id = attr->Value();
		}
	}

	User* user = LocalDB::getInstance()->get_user(user_id);
	if (user == nullptr) {
		send_task_result_msg(fd, MSG_TYPE::SEND_DEV_LIST, PROCESS_RESULT::FAIL);
		return;
	}

	//send dev_list//
	XMLDocument doc;
	XMLDeclaration* dec = doc.NewDeclaration();
	doc.LinkEndChild(dec);

	XMLElement* root = doc.NewElement("IoTPacket");
	doc.LinkEndChild(root);
	root->SetAttribute("PacketType", MSG_TYPE::SEND_DEV_LIST);

	XMLElement* ack = doc.NewElement("ServerMsg");
	root->LinkEndChild(ack);

	XMLElement* body = doc.NewElement("Data");
	ack->LinkEndChild(body);

	const std::vector<Device> * dev_list = user->get_device_list();
	for (int i = 0; i < user->get_device_list()->size(); i++) {
		XMLElement* list;
		list = doc.NewElement(std::string("Device").c_str());
		body->LinkEndChild(list);
		list->SetAttribute("Serial", (*dev_list)[i].get_id().c_str());
		list->SetAttribute("Alias", (*dev_list)[i].get_nickname().c_str());
		if(LocalDB::getInstance()->get_device((*dev_list)[i].get_id()) != nullptr)
			list->SetAttribute("Connect", 1);
		else
			list->SetAttribute("Connect", 0);
	}

	XMLPrinter printer;
	std::string data;
	doc.Accept(&printer);
	data = printer.CStr();

	doc.Clear();
	LocalDB::getInstance()->push_send_msg(std::pair<int, std::string>(fd, data));
}

void Server::select_device(int fd, const XMLNode * msg) {
	std::string user_id = "", dev_id = "", screen_data = "";
	bool need_screen_data = false;
	
	while (msg != nullptr && !(std::string(msg->Value()) == "SelDeviceClient"))
		msg = msg->NextSibling();

	if (msg != nullptr) {
		for (auto attr = msg->ToElement()->FirstAttribute(); attr != nullptr; attr = attr->Next()) {
			if (std::string(attr->Name()) == "ID")
				user_id = attr->Value();
			else if (std::string(attr->Name()) == "Serial")
				dev_id = attr->Value();///////////////////////////////////////////////////
			else if (std::string(attr->Name()) == "ScreenInfo")
				need_screen_data = attr->IntValue();
		}
	}

	//check user_permission//
	User* user = LocalDB::getInstance()->get_user(user_id);
	if (user == nullptr || user->find_device(dev_id) == nullptr) {
		send_task_result_msg(fd, MSG_TYPE::SEL_DEV, PROCESS_RESULT::FAIL);
		return;
	}

	/*
	if need screen_data, then ODBC access -> get screen data
	and send device_data
	*/

	//send data
	//send dev_list//
	XMLDocument doc;
	XMLDeclaration* dec = doc.NewDeclaration();
	doc.LinkEndChild(dec);

	XMLElement* root = doc.NewElement("IoTPacket");
	doc.LinkEndChild(root);
	root->SetAttribute("PacketType", MSG_TYPE::SEL_DEV);

	XMLElement* ack = doc.NewElement("ServerMsg");
	root->LinkEndChild(ack);

	XMLElement* body = doc.NewElement("Data");
	ack->LinkEndChild(body);

	if (need_screen_data) {
		std::string screen_data;
		XMLElement* screen_element = doc.NewElement("ScreenInfo");
		body->LinkEndChild(screen_element);
		screen_data = ODBC_Manager::getInstance()->Get_ScreenData(dev_id);
		screen_element->SetAttribute("data", screen_data.c_str());
	}

	XMLElement* dev_status;
	dev_status = doc.NewElement("Status");
	body->LinkEndChild(dev_status);
	std::string status = LocalDB::getInstance()->get_device(dev_id)->get_status();
	dev_status->SetAttribute("data", status.c_str());

	XMLPrinter printer;
	std::string data;
	doc.Accept(&printer);
	data = printer.CStr();

	doc.Clear();
	std::cout << data << std::endl;
	LocalDB::getInstance()->push_send_msg(std::pair<int, std::string>(fd, data));
}

void Server::control_device(int fd, const XMLNode * msg) {
	std::string user_id = "", dev_id = "", func = "";

	while (msg != nullptr && !(std::string(msg->Value()) == "CtrlDeviceClient"))
		msg = msg->NextSibling();

	if (msg != nullptr) {
		for (auto attr = msg->ToElement()->FirstAttribute(); attr != nullptr; attr = attr->Next()) {
			if (std::string(attr->Name()) == "ID")
				user_id = attr->Value();
			else if (std::string(attr->Name()) == "Serial")
				dev_id = attr->Value();
			else if (std::string(attr->Name()) == "Func")
				func = attr->Value();
		}
	}

	//check permission and valid//
	User* user = LocalDB::getInstance()->get_user(user_id);
	Device* dev = LocalDB::getInstance()->get_device(dev_id);
	if (user == nullptr || user->find_device(dev_id) == nullptr || dev == nullptr) {
		send_task_result_msg(fd, MSG_TYPE::CTRL_DEV, PROCESS_RESULT::FAIL);
		return;
	}

	//get device_fd, send_data//
	XMLDocument doc;
	XMLDeclaration* dec = doc.NewDeclaration();
	doc.LinkEndChild(dec);

	XMLElement* root = doc.NewElement("IoTPacket");
	doc.LinkEndChild(root);
	root->SetAttribute("PacketType", MSG_TYPE::CTRL_DEV);

	XMLElement* ack = doc.NewElement("ServerMsg");
	root->LinkEndChild(ack);
	ack->SetAttribute("Command", func.c_str());

	XMLPrinter printer;
	std::string data;
	doc.Accept(&printer);
	data = printer.CStr();

	doc.Clear();
	LocalDB::getInstance()->push_send_msg(std::pair<int, std::string>(dev->get_fd(), data));
	send_task_result_msg(fd, MSG_TYPE::CTRL_DEV, PROCESS_RESULT::SUCCESS);
}

void Server::change_user_info(int fd, const XMLNode * msg) {
	std::string id = "", pw = "", name = "", email = "", phone_number = ""; // if string is "", then not modify it

	while (msg != nullptr && !(std::string(msg->Value()) == "ChngUserInfoClient"))
		msg = msg->NextSibling();

	if (msg != nullptr) {
		for (auto attr = msg->ToElement()->FirstAttribute(); attr != nullptr; attr = attr->Next()) {
			if (std::string(attr->Name()) == "ID")
				id = attr->Value();
			else if (std::string(attr->Name()) == "PW")
				pw = attr->Value();
			else if (std::string(attr->Name()) == "Name")
				name = attr->Value();
			else if (std::string(attr->Name()) == "Mobile")
				phone_number = attr->Value();
			else if (std::string(attr->Name()) == "Email")
				email = attr->Value();
		}
	}

	/*
	ODBC_Access
	*///
	if (!ODBC_Manager::getInstance()->Change_User_info(id, pw, name, email, phone_number)) {
		send_task_result_msg(fd, MSG_TYPE::CHNG_USR_INFO, PROCESS_RESULT::FAIL);
		return;
	}
	send_task_result_msg(fd, MSG_TYPE::CHNG_USR_INFO, PROCESS_RESULT::SUCCESS);
}

void Server::get_dev_data(const tinyxml2::XMLNode * msg) {
	std::string id, status; // if string is "", then not modify it

	while (msg != nullptr && !(std::string(msg->Value()) == "Status"))
		msg = msg->NextSibling();

	if (msg != nullptr) {
		for (auto attr = msg->ToElement()->FirstAttribute(); attr != nullptr; attr = attr->Next()) {
			if (std::string(attr->Name()) == "ID")
				id = attr->Value();
			else if (std::string(attr->Name()) == "Data")
				status = attr->Value();
		}
	}

	Device* dev = LocalDB::getInstance()->get_device(id);
	if (dev != nullptr)
		dev->set_status(status);

	std::cout << status << std::endl;
}

void Server::send_task_result_msg(int fd, MSG_TYPE packet_type, PROCESS_RESULT result) {
	XMLDocument doc;
	XMLDeclaration* dec = doc.NewDeclaration();
	doc.LinkEndChild(dec);

	XMLElement* root = doc.NewElement("IoTPacket");
	doc.LinkEndChild(root);
	root->SetAttribute("PacketType", packet_type);

	XMLElement* ack = doc.NewElement("ServerMsg");
	root->LinkEndChild(ack);
	if (result == PROCESS_RESULT::FAIL)
		ack->SetAttribute("MSG", "fail");
	else if (result == PROCESS_RESULT::SUCCESS)
		ack->SetAttribute("MSG", "done");

	XMLPrinter printer;
	std::string msg;
	doc.Accept(&printer);
	msg = printer.CStr();

	doc.Clear();
	LocalDB::getInstance()->push_send_msg(std::pair<int, std::string>(fd, msg));
}