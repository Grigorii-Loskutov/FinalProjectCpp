#include "indexator.h"
#include <iostream>
#include <string.h>
#include <set>
#include <map>
#include <exception>
<<<<<<< Updated upstream
#include "database.h"
=======
#include <regex>
#include <iterator>
#include <vector>
>>>>>>> Stashed changes
#include "HTTPclient.h"
#include "ParcerHTML.h"


std::set<std::string> indexator(database& DB, std::string inLink) {

	std::set<std::string> Links; // Набор ссылок, найденных на странице
	std::map<std::string, int> Frequencies; // Частоты слов, найденных на странице
	int link_id; // Идентификатор страницы, которую индексируем
	std::map<std::string, int> WordIdPair; // Идентификаторы и соответсвующие слова в таблице
	std::set<std::string> wordsInDB;		// Слова, полученные из базы данных
	std::set<std::string> wordsInPage;	    // Слова, наденные на странице (для проверки наличия их в базе данных)
	std::string host;						// Адрес хоста
	std::string target;						// Ресурс на хосте
	bool isHTTPS = false;					// Поддерживает ли хост https

<<<<<<< Updated upstream
	// Разделим адрес на host и target
	size_t slashPos = inLink.find("/");
	if (slashPos != std::string::npos) {
		host = inLink.substr(0, slashPos);
		target = inLink.substr(slashPos);
=======
	// Функция возвращает кортеж: (адрес индексируемой страницы, set новых ссылок, набор: (слово, частота))
	std::tuple <std::string, std::set<std::string>, std::map<std::string, int>> indexatorResult;

	//Определим тип сервера: http или https
	const std::string const http_pref = "http://";
	const std::string const https_pref = "https://";
	if (inLink.compare(0, https_pref.length(), https_pref) == 0) {
		isHTTPS = true;
		std::regex pattern_https(https_pref);
		host = std::regex_replace(inLink, pattern_https, "");
	}
	else if (inLink.compare(0, http_pref.length(), http_pref) == 0) {
		isHTTPS = false;
		std::regex pattern_http(http_pref);
		host = std::regex_replace(inLink, pattern_http, "");
>>>>>>> Stashed changes
	}
	else {
		host = inLink;
	}

	// Разделим адрес на host и target
	size_t slashPos = host.find("/");
	if (slashPos != std::string::npos) {
		host = host.substr(0, slashPos);
		target = host.substr(slashPos);
	}
	else {
		host = host;
		target = "/";
	}

	try {
		HTTPclient client; // Клиент для скачивания страницы
		std::string response = ""; // Строка с ответом
<<<<<<< Updated upstream

		client.performGetRequest(host, "80", target, 5);

=======
		if (isHTTPS)
		{
			client.performGetRequest(host, "443", target, 11);
		}
		else
		{
			client.performGetRequest(host, "80", target, 11);
		}
>>>>>>> Stashed changes
		response = client.getData();
		try
		{
			// Пробуем парсить страницу
			ParcerHTML parcerHTML(response, inLink);
			Links = parcerHTML.getLinks();
			Frequencies = parcerHTML.getFrequencies();
<<<<<<< Updated upstream

			for (const auto& line : Links) {
				//std::cout << line << std::endl;
			}
			try
			{
				// Добавим в базу адресс страницы, которую проиндексировали (если такого адреса там ещё нет)
				DB.link_add(inLink); // Просто бросим исключения, если адресс уже проиндексирован

				// Вносим в базу данных частоты слов для определенной страницы
				// Получим id страницы, которую индексируем
				try {
					link_id = DB.getLinkId(inLink);

					// Получим таблицу слов с id целиком
					try {
						WordIdPair = DB.getWordId();
						bool isNewWordAdd = false;
						// Добавим новые слова, если такие есть
						for (const auto& pair : Frequencies) {
							std::string wordInPage = pair.first;
							if (WordIdPair[wordInPage] == 0) {
								// Добавим в базу данных новое слово
								isNewWordAdd = true;
								try {
									DB.word_add(wordInPage);
								}
								catch (const std::exception& ex) {
									std::cout << __FILE__ << ", line: " << __LINE__ << std::endl;
									std::cout << "\n Fail to add new word in database: ";
									std::string except = ex.what();
									std::cout << "\n" << except;
									DB.CloseConnection();
									return Links;
								}
							}
						}

						// Если были добавления слов, то загрузим заново таблицу из  базы данных
						if (isNewWordAdd) {
							try {
								WordIdPair = DB.getWordId();
							}
							catch (const std::exception& ex) {
								std::cout << __FILE__ << ", line: " << __LINE__ << std::endl;
								std::cout << "\n Fail to get words ID from database: ";
								std::string except = ex.what();
								std::cout << "\n" << except;
								DB.CloseConnection();
								return Links;
							}
						}

						//Заполним таблицу частот, если успешно получили ID страницы
						for (const auto& pair : Frequencies) {
							//std::cout << counter << ". " << pair.first << ": " << pair.second << std::endl;
							//++counter;
							int wordFrequency = pair.second;
							int wordId = WordIdPair[pair.first];
							try {
								DB.frequency_add(link_id, wordId, wordFrequency);
							}
							catch (const std::exception& ex) {
								std::cout << __FILE__ << ", line: " << __LINE__ << std::endl;
								std::cout << "\nTry to add frequency for word " << pair.first << ": ";
								std::string except = ex.what();
								std::cout << "\n" << except;
								DB.CloseConnection();
								return Links;
							}
						}
					}
					catch (const std::exception& ex) {
						std::cout << __FILE__ << ", line: " << __LINE__ << std::endl;
						std::cout << "\n Fail to get words ID from database: ";
						std::string except = ex.what();
						std::cout << "\n" << except;
						DB.CloseConnection();
						return Links;
					}

				}
				catch (const std::exception& ex) {
					std::cout << __FILE__ << ", line: " << __LINE__ << std::endl;
					std::cout << "\n Fail to get address ID from database " + inLink << ": ";
					std::string except = ex.what();
					std::cout << "\n" << except;
					DB.CloseConnection();
					return Links;
				}


			}
			catch (const std::exception& ex) {
				std::cout << __FILE__ << ", line: " << __LINE__ << std::endl;
				std::cout << "\n Fail to add new address " << inLink << ": ";
				std::string except = ex.what();
				std::cout << "\n" << except;
				DB.CloseConnection();
				return Links;
			}

=======
			indexatorResult = std::make_tuple(inLink, Links, Frequencies);
>>>>>>> Stashed changes
		}
		catch (const std::exception& ex) {
			std::cout << "\n Fail to parce page " + inLink << ": ";
			std::string except = ex.what();
			std::cout << "\n" << except;
			return Links;
		}
	}
	catch (const std::exception& ex) {
		std::cout << __FILE__ << ", line: " << __LINE__ << std::endl;
		std::cout << "\n Fail to load page: ";
		std::string except = ex.what();
		std::cout << "\n" << except;
		return Links;

	}
	//std::cout << "\nИндексатор отработал и не упал" << std::endl;
	return Links;
};