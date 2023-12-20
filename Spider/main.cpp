#include <iostream>
//#include <pqxx/pqxx>
#include <windows.h>
#include <exception>
#include <string.h>
#include <thread>
#include "ParcerINI.h"
#include "HTTPclient.h"
#include "HTTPserverSync.h"

std::string DataBaseHostName;
std::string DataBaseName;
std::string DataBaseUserName;
std::string DataBasePassword;
int DataBasePort;

std::string SpiderStarPageURL;
int SpiderDepth;

std::string FinderAddress;
int FinderPort;


int main()
{
	setlocale(LC_ALL, "Russian");
	SetConsoleCP(65001);
	SetConsoleOutputCP(65001);
	// Прочитаем конфигурацию в файле configuration.ini
	try {
		char buffer[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, buffer);
		std::string filePath = std::string(buffer) + "\\configuration.ini"; // определим путь к исполняемому файлу
		std::cout << filePath;
		ParcerINI parser = ParcerINI::ParcerINI(filePath);

		DataBaseHostName = parser.get_value<std::string>("DataBase.HostName");
		DataBaseName = parser.get_value<std::string>("DataBase.DatabaseName");
		DataBaseUserName = parser.get_value<std::string>("DataBase.UserName");
		DataBasePassword = parser.get_value<std::string>("DataBase.Password");
		DataBasePort = parser.get_value<int>("DataBase.Port");

		SpiderStarPageURL = parser.get_value<std::string>("Spider.StartPageURL");
		SpiderDepth = parser.get_value<int>("Spider.Depth");

		FinderAddress = parser.get_value<std::string>("Finder.Address");
		FinderPort = parser.get_value<int>("Finder.Port");


		std::cout << "DataBaseHostName: " << DataBaseHostName << std::endl;
		std::cout << "DataBaseName: " << DataBaseName << std::endl;
		std::cout << "DataBaseUserName: " << DataBaseUserName << std::endl;
		std::cout << "DataBasePassword: " << DataBasePassword << std::endl;
		std::cout << "DataBasePort: " << DataBasePort << std::endl;
		std::cout << "SpiderStarPageURL: " << SpiderStarPageURL << std::endl;
		std::cout << "SpiderDepth: " << SpiderDepth << std::endl;
		std::cout << "FinderAddress: " << FinderAddress << std::endl;
		std::cout << "FinderPort: " << FinderPort << std::endl;


	}
	catch (const std::exception& ex) {
		std::string except = ex.what();
		std::cout << "\n" << except;
		//std::cout << ex.what() << std::endl;
	}

	//Отправим запрос на HTTP, указанный в конфигурации
	HTTPserver server;
	char* addrServer = { "127.0.0.1" };
	char* portServer = { "8080" };
	char* docRootServer = { "." };
	std::thread serverThread(&HTTPserver::MakeHTTPserver, &server, FinderAddress, FinderPort, docRootServer);
	//server.MakeHTTPserver(4, addrServer, portServer, docRootServer);

	HTTPclient client;
	client.performGetRequest(SpiderStarPageURL, "80", "/", 5);
	std::vector<std::string> response = client.getData();
	for (const auto& line : response) {
		std::cout << line << std::endl;
	}
	serverThread.join(); // ToDo: Как завершить работу сервера?
}

