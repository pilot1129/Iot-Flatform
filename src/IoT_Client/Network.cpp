#include "Network.h"

using namespace tinyxml2;

Network* Network::Instance = nullptr;

Network * Network::getInstance() {
	if (Instance == nullptr)
		Instance = new Network;
	return Instance;
}

void Network::socket_send(std::string data, MSG_TYPE type) {
	if (dev_reg) {
		std::string msg = makeXML(data, type);

		if (write(this->sock_fd, msg.c_str(), msg.size()) < 0)
			this->initConnect();
	}
}

std::string Network::socket_recv() {
	std::string msg = "";
	if (dev_reg) {
		char buf[BUF_SIZE];

		memset(buf, 0, sizeof(buf));
		while (read(this->sock_fd, buf, sizeof(buf)) < 0)
			this->initConnect();

		msg = parseXML(buf);
	}
	return msg;
}

void Network::set_device(std::string maker_code, std::string product_code, std::string serial, std::string nickname) {
	this->dev_info.maker_code = maker_code;
	this->dev_info.product_code = product_code;
	this->dev_info.serial = serial;
	this->dev_info.nickname = nickname;
	this->dev_reg = true;
	initConnect();
}

Network::Network() {
	this->dev_reg = false;
	while (true) {
		if ((this->sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			std::cout << "Can't open stream socket" << std::endl;
			continue;
		}
		break;
	}
	memset(&this->sock_addr, 0, sizeof(this->sock_addr));

	this->sock_addr.sin_family = AF_INET;
	this->sock_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	this->sock_addr.sin_port = htons(SERVER_PORT);
}


Network::~Network() {
	close(this->sock_fd);
}

void Network::initConnect() {
	while (true) {
		if (connect(sock_fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr))) {
			std::cout << "can't connect to Server." << std::endl;
			std::cout << "try reconnect..." << std::endl;
			delay(1000);
			continue;
		}
		socket_send(this->dev_info.serial, MSG_TYPE::REG_DEV);
		if (socket_recv() == "done") { //add timeout
			std::cout << "Server connect success!!" << std::endl << std::endl;
			break;
		}
	}
}

std::string Network::makeXML(std::string data, MSG_TYPE type) { // add error
	XMLDocument doc;
	XMLDeclaration* dec = doc.NewDeclaration();
	doc.LinkEndChild(dec);

	XMLElement* root = doc.NewElement("IoTPacket");
	doc.LinkEndChild(root);
	root->SetAttribute("PacketType", type);

	if (type == MSG_TYPE::REG_DEV) {
		XMLElement* reg_data = doc.NewElement("Device");
		root->LinkEndChild(reg_data);
		reg_data->SetAttribute("ID", this->dev_info.serial.c_str());
		reg_data->SetAttribute("MakerCode", this->dev_info.maker_code.c_str());
		reg_data->SetAttribute("ProductCode", this->dev_info.product_code.c_str());
		reg_data->SetAttribute("nickName", this->dev_info.nickname.c_str());
	}
	else {
		XMLElement* status = doc.NewElement("Status");
		root->LinkEndChild(status);
		status->SetAttribute("Data", data.c_str());
	}

	XMLPrinter printer;
	std::string msg;
	doc.Accept(&printer);
	msg = printer.CStr();

	doc.Clear();
	return msg;
}

std::string Network::parseXML(std::string XMLdata) {
	XMLDocument doc;
	int msg_type = 0;
	if (doc.Parse(XMLdata.c_str(), XMLdata.size()) != XMLError::XML_SUCCESS) {
		std::cout << "xml Read Error!!" << std::endl << "msg: " << XMLdata << std::endl;
		return "";
	}

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
	while (node != nullptr && !(std::string(node->Value()) == "ServerMsg"))
		node = node->NextSibling();

	switch (msg_type) {
		case MSG_TYPE::REG_DEV:
			if (node != nullptr) {
				for (auto attr = node->ToElement()->FirstAttribute(); attr != nullptr; attr = attr->Next()) {
					if (std::string(attr->Name()) == "MSG")
						return attr->Value();
				}
			}
			break;

		case MSG_TYPE::CTRL_DEV:
			if (node != nullptr) {
				for (auto attr = node->ToElement()->FirstAttribute(); attr != nullptr; attr = attr->Next()) {
					if (std::string(attr->Name()) == "Command")
						return attr->Value();
				}
			}
			break;

		default:
			break;
	}

	return "";
}
