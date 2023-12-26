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

	std::set<std::string> Links; // Набор ссылок, найденных на странице
	std::map<std::string, int> Frequencies; // Частоты слов, найденных на странице
	int link_id; // Идентификатор страницы, которую индексируем
	std::map<std::string, int> WordIdPair; // Идентификаторы и соответсвующие слова в таблице
	try {

		client.performGetRequest(SpiderStarPageURL, "80", "/", 5);
		std::string response = client.getData();
		ParcerHTML parcerHTML(response, SpiderStarPageURL);
		Links = parcerHTML.getLinks();
		Frequencies = parcerHTML.getFrequencies();


		for (const auto& line : Links) {
			std::cout << line << std::endl;
		}
		

		// Запишем адресс страницы, которую проиндексировали
		try
		{
			DB.link_add(SpiderStarPageURL);
		}
		catch (const std::exception& ex) {
			std::cout << "Try to add new URL in database\n";
			std::string except = ex.what();
			std::cout << "\n" << except;
		}

		unsigned int counter = 0;
		for (const auto& pair : Frequencies) {
			//std::cout << counter << ". " << pair.first << ": " << pair.second << std::endl;
			//++counter;
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
		
	}

	// Вносим в базу данных частоты слов для определенной страницы
	
	// Получим таблицу слов с id целиком
	try {
		WordIdPair = DB.getWordId();
	}
	catch (const std::exception& ex) {
		std::cout << "Try to get words id\n";
		std::string except = ex.what();
		std::cout << "\n" << except;
	}

	// Выведем таблицу
	unsigned int counter = 0;
	for (const auto& pair : WordIdPair) {
		std::cout << counter << ". " << pair.first << ": " << pair.second << std::endl;
		++counter;
	}

	// Получим id страницы, которую индексируем
	try {
		link_id = DB.getLinkId(SpiderStarPageURL);
		std::cout << SpiderStarPageURL << " id = " << link_id << std::endl;
	}
	catch (const std::exception& ex) {
		std::cout << "Try to get link id\n";
		std::string except = ex.what();
		std::cout << "\n" << except;
	}
	 //Заполним таблицу частот
	for (const auto& pair : Frequencies) {
		//std::cout << counter << ". " << pair.first << ": " << pair.second << std::endl;
		//++counter;
		int wordFrequency = pair.second;
		int wordId = WordIdPair[pair.first];
		try {
			DB.frequency_add(link_id, wordId, wordFrequency);
		}
		catch (const std::exception& ex) {
			std::string except = ex.what();
			std::cout << "\n" << except;
		}
	}
	std::cout << "Паук отработал и не упал" << std::endl;
}