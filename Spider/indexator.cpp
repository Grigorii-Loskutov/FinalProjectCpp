#include "HTTPclient.h"
#include "ParcerHTML.h"
#include "indexator.h"
#include <iostream>
#include <string.h>
#include <set>
#include <map>
#include <exception>
#include <regex>
#include <iterator>


std::tuple <std::string, std::set<std::string>, std::map<std::string, int>> indexator(std::string inLink) {

	std::set<std::string> Links; // Набор ссылок, найденных на странице
	std::map<std::string, int> Frequencies; // Частоты слов, найденных на странице
	int link_id; // Идентификатор страницы, которую индексируем
	std::map<std::string, int> WordIdPair; // Идентификаторы и соответсвующие слова в таблице
	std::set<std::string> wordsInDB;		// Слова, полученные из базы данных
	std::set<std::string> wordsInPage;	    // Слова, наденные на странице (для проверки наличия их в базе данных)
	std::string host;						// Адрес хоста
	std::string target;						// Ресурс на хосте
	bool isHTTPS = false;					// Поддерживает ли хост https

	// Функция возвращает кортеж: (адрес индексируемой страницы, set новых ссылок, набор: (слово, частота))
	std::tuple <std::string, std::set<std::string>, std::map<std::string, int>> indexatorResult;

	//Определим тип сервера: http или https
	const std::string http_pref = "http://";
	const std::string https_pref = "https://";
	if (inLink.compare(0, https_pref.length(), https_pref) == 0) {
		isHTTPS = true;
		//std::regex pattern_https(https_pref);
		//host = std::regex_replace(inLink, pattern_https, "");
	}
	else if (inLink.compare(0, http_pref.length(), http_pref) == 0) {
		isHTTPS = false;
		//std::regex pattern_http(http_pref);
		//host = std::regex_replace(inLink, pattern_http, "");
	}
	else {
		host = inLink;
	}
	std::regex pattern_https(https_pref);
	std::regex pattern_http(http_pref);
	host = (isHTTPS) ? std::regex_replace(inLink, pattern_https, "") : std::regex_replace(inLink, pattern_http, "");

	// Разделим адрес на host и target
	size_t slashPos = host.find("/");
	if (slashPos != std::string::npos) {
		std::string temp_str = host;
		host = host.substr(0, slashPos);
		target = temp_str.substr(slashPos);
	}
	else {
		host = host;
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