#include "indexator.h"
#include <iostream>
#include <string.h>
#include <set>
#include <map>
#include <exception>
#include "database.h"
#include "HTTPclient.h"
#include "ParcerHTML.h"


std::set<std::string> indexator(database& DB, std::string inLink) {

	std::set<std::string> Links; // Набор ссылок, найденных на странице
	std::map<std::string, int> Frequencies; // Частоты слов, найденных на странице
	int link_id; // Идентификатор страницы, которую индексируем
	std::map<std::string, int> WordIdPair; // Идентификаторы и соответсвующие слова в таблице
	std::set<std::string> wordsInDB;		// Слова, полученные из базы данных
	std::set<std::string> wordsInPage;	    // Слова, наденные на странице (для проверки наличия их в базе данных)
	std::string host;
	std::string target;

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

		if (client.performGetRequest(host, "80", target, 5) == 0) // Запрос выполнен без ошибки
		{
			response = client.getData();
			try
			{
				// Пробуем парсить страницу
				ParcerHTML parcerHTML(response, inLink);
				Links = parcerHTML.getLinks();
				Frequencies = parcerHTML.getFrequencies();
				
				for (const auto& line : Links) {
					//std::cout << line << std::endl;
				}
				try
				{
					// Добавим в базу адресс страницы, которую проиндексировали (если такого адреса там ещё нет)
					DB.link_add(inLink);

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

			}
			catch (const std::exception& ex) {
				std::cout << "\n Fail to parce page " + inLink << ": ";
				std::string except = ex.what();
				std::cout << "\n" << except;
				return Links;
			}
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