#include <iostream>
#include <tuple>
#include <pqxx/pqxx>
#include <windows.h>
#include <exception>
#include <string.h>
#include <thread>
#include "ParcerINI.h"
#include "HTTPclient.h"
#include "ParcerHTML.h"
#include "database.h"


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
	//setlocale(LC_ALL, "Russian");
	SetConsoleCP(65001);
	SetConsoleOutputCP(65001); //UTF-8
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
	}

	// Создадим подключение к базе данных
	database DB;
	try {
		DB.SetConnection(DataBaseHostName, DataBaseName, DataBaseUserName, DataBasePassword, DataBasePort);
		DB.table_create();
	}
	catch (const std::exception& ex) {
		std::cout << "Try to create tables in databse\n";
		std::string except = ex.what();
		std::cout << "\n" << except;
	}

	// Создадим HTTP client
	HTTPclient client;
	try {

		client.performGetRequest(SpiderStarPageURL, "80", "/", 5);
		std::string response = client.getData();
		ParcerHTML parcerHTML(response, SpiderStarPageURL);
		std::set<std::string> Links = parcerHTML.getLinks();
		std::map<std::string, int> Frequencies = parcerHTML.getFrequencies();


		for (const auto& line : Links) {
			std::cout << line << std::endl;
		}
		unsigned int counter = 0;
		for (const auto& pair : Frequencies) {
			std::cout << counter << ". " << pair.first << ": " << pair.second << std::endl;
			++counter;
			// Запись в database под отдельный try
			try {
				DB.word_add(pair.first);
			}
			catch (const std::exception& ex) {
				std::string except = ex.what();
				std::cout << "\n" << except;
			}

		}
	}
	catch (const std::exception& ex) {
		std::cout << "Try to load HTML and parce it\n";
		std::string except = ex.what();
		std::cout << "\n" << except;
		std::cout << "Паук отработал и не упал" << std::endl;

	}
}