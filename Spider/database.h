#pragma once

#include <iostream>
#include <pqxx/pqxx>
#include <exception>
#include <tuple>
#include <windows.h>

class database {
private:
	std::unique_ptr <pqxx::connection> c;

	std::string str_creation = {
			"CREATE TABLE IF NOT EXISTS links ("
			"id SERIAL PRIMARY KEY, "
			"link VARCHAR(100) UNIQUE NOT NULL); "
			"CREATE TABLE IF NOT EXISTS words ("
			"id SERIAL PRIMARY KEY, "
			"word VARCHAR(40) UNIQUE NOT NULL); "
			"CREATE TABLE IF NOT EXISTS frequencies ("
			"links_id INTEGER REFERENCES links(id), "
			"words_id INTEGER REFERENCES words(id), "
			"frequency INTEGER, "
			"CONSTRAINT pk PRIMARY KEY(links_id, words_id));"	
	};

public:
	database();

	void SetConnection(std::string DataBaseHostName,
		std::string DataBaseName,
		std::string DataBaseUserName,
		std::string DataBasePassword,
		int DataBasePort);

	void table_create();


	database(const database&) = delete; // Запретим копирование

	database& operator=(const database&) = delete; // Запретим копирование

	void word_add(const std::string newWord);
	
	void link_add(const std::string newLink);
};