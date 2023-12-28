#include "indexator.h"
#include <iostream>
#include <string.h>
#include <set>
#include <map>
#include <exception>
#include "database.h"
#include "HTTPclient.h"
#include "ParcerHTML.h"

std::set<std::string> Links; // Набор ссылок, найденных на странице
std::map<std::string, int> Frequencies; // Частоты слов, найденных на странице
int link_id; // Идентификатор страницы, которую индексируем
std::map<std::string, int> WordIdPair; // Идентификаторы и соответсвующие слова в таблице

std::set<std::string> indexator(database& DB, std::string inLink) {

	try {
		HTTPclient client; // Клиент для скачивания страницы
		std::string response = ""; // Строка с ответом
		if (client.performGetRequest(inLink, "80", "/", 5) == 0)
		{
			response = client.getData();
		}
		ParcerHTML parcerHTML(response, inLink);
		Links = parcerHTML.getLinks();
		Frequencies = parcerHTML.getFrequencies();

		// Выведем найденные ссылки
		for (const auto& line : Links) {
			std::cout << line << std::endl;
		}


		// Запишем адресс страницы, которую проиндексировали
		try
		{
			DB.link_add(inLink);
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
				std::cout << "Try to add new word in database\n";
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
	//unsigned int counter = 0;
	//for (const auto& pair : WordIdPair) {
	//	std::cout << counter << ". " << pair.first << ": " << pair.second << std::endl;
	//	++counter;
	//}

	// Получим id страницы, которую индексируем
	try {
		link_id = DB.getLinkId(inLink);
		std::cout << inLink << " id = " << link_id << std::endl;
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
			//std::cout << "Try to add frequency\n";
			std::string except = ex.what();
			//std::cout << "\n" << except;
		}
	}
	std::cout << "\n Индексатор отработал и не упал \n" << std::endl;
	return Links;
};