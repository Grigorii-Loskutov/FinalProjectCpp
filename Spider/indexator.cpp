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

	std::string linkToDB; //ссылка для добавления в БД
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

		// Найдем символ "?" в temp_str
		size_t questionMarkPos = temp_str.find("?");
		if (questionMarkPos != std::string::npos) {
			// Если "?" найден, обрежем строку до этого символа
			temp_str = temp_str.substr(0, questionMarkPos);
		}
		target = temp_str.substr(slashPos);
	}
	else {
		host = host;
		target = "/";
	}

	try {
		HTTPclient client; // Клиент для скачивания страницы
		std::string response{ "" }; // Строка с ответом
		std::string redirectLink{ "" }; //Строка для ссылки в результате редиректа
		if (isHTTPS)
		{
			redirectLink = client.performGetRequest(host, "443", target, 11);
		}
		else
		{
			redirectLink = client.performGetRequest(host, "80", target, 11);
		}
		response = client.getData();

		try
		{
			//Если не было редиректа
			if(redirectLink.length()==0)
			{
				linkToDB = inLink; // Сссылка для добавления в БД соответствует входной ссылке
				if (isHTTPS)
				{
					host = https_pref + host;
				}
				else {
					host = http_pref + host;
				}
			}
			//Если был редирект
			else {
				linkToDB = redirectLink; //Сссылка для добавления в БД берется их редиректа
				// Выделим host
				size_t slashPos = linkToDB.find("/");
				if (slashPos != std::string::npos) {
					host = host.substr(0, slashPos);
				}
				else {
					host = linkToDB;
				}
			}
			// Пробуем парсить страницу
			ParcerHTML parcerHTML(response, host);
			Links = parcerHTML.getLinks();
			Frequencies = parcerHTML.getFrequencies();
			indexatorResult = std::make_tuple(linkToDB, Links, Frequencies);
			//std::cout << parcerHTML.getLine();
		}
		catch (const std::exception& ex) {
			std::cout << "\n\t" << "Fail to parce page " + linkToDB << ": ";
			std::string except = ex.what();
			std::cout << "\n" << except;
		}
	}
	catch (const std::exception& ex) {
		std::cout << "\n\t" << "Fail to load page " + linkToDB << ": ";
		std::string except = ex.what();
		std::cout << "\n" << except;
	}
	return indexatorResult;
};