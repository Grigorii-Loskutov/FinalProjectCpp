#include "indexator.h"
#include <iostream>
#include <string.h>
#include <set>
#include <map>
#include <exception>
#include "HTTPclient.h"
#include "ParcerHTML.h"


std::tuple <std::string, std::set<std::string>, std::map<std::string, int>> indexator(std::string inLink) {

	std::set<std::string> Links; // Набор ссылок, найденных на странице
	std::map<std::string, int> Frequencies; // Частоты слов, найденных на странице
	int link_id; // Идентификатор страницы, которую индексируем
	std::map<std::string, int> WordIdPair; // Идентификаторы и соответсвующие слова в таблице
	std::set<std::string> wordsInDB;		// Слова, полученные из базы данных
	std::set<std::string> wordsInPage;	    // Слова, наденные на странице (для проверки наличия их в базе данных)
	std::string host;
	std::string target;

	// Функция возвращает кортеж: (адрес индексируемой страницы, set новых ссылок, набор: (слово, частота))
	std::tuple <std::string, std::set<std::string>, std::map<std::string, int>> indexatorResult;

	// Разделим адрес на host и target
	size_t slashPos = inLink.find("/");
	if (slashPos != std::string::npos) {
		host = inLink.substr(0, slashPos);
		target = inLink.substr(slashPos);
	}
	else {
		host = inLink;
		target = "/";
	}

	try {
		HTTPclient client; // Клиент для скачивания страницы
		std::string response = ""; // Строка с ответом

		client.performGetRequest(host, "80", target, 5);
		response = client.getData();
		try
		{
			// Пробуем парсить страницу
			ParcerHTML parcerHTML(response, inLink);
			Links = parcerHTML.getLinks();
			Frequencies = parcerHTML.getFrequencies();
			indexatorResult = std::make_tuple(inLink, Links, Frequencies);
			//std::cout << parcerHTML.getLine();
		}
		catch (const std::exception& ex) {
			std::cout << "\n\t" << "Fail to parce page " + inLink << ": ";
			std::string except = ex.what();
			std::cout << "\n" << except;
		}
	}
	catch (const std::exception& ex) {
		std::cout << "\n\t" << "Fail to load page " + inLink << ": ";
		std::string except = ex.what();
		std::cout << "\n" << except;
	}
	return indexatorResult;
};