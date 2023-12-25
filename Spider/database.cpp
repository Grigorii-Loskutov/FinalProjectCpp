#include "database.h"
#include <iostream>
#include <pqxx/pqxx>
#include <exception>
#include <tuple>
#include <windows.h>

void database::SetConnection(std::string DataBaseHostName,
	std::string DataBaseName,
	std::string DataBaseUserName,
	std::string DataBasePassword,
	int DataBasePort) {
	std::string str_connection = "";

	// Составим строку для подключения
	str_connection.append("host=" + DataBaseHostName + " ");
	str_connection.append("port=" + std::to_string(DataBasePort) + " ");
	str_connection.append("dbname=" + DataBaseName + " ");
	str_connection.append("user=" + DataBaseUserName + " ");
	str_connection.append("password=" + DataBasePassword);

	// Создадим подключение
	std::unique_ptr<pqxx::connection> c1 = std::make_unique<pqxx::connection>(str_connection);
	c = std::move(c1);
	(*c).set_client_encoding("UTF8");
}

database::database() {
	;
}

void database::table_create() {
	pqxx::work tx{ *c };
	tx.exec(str_creation);
	tx.commit();
}

void database::word_add(const std::string newWord) {
	pqxx::work tx{ *c };
	std::string str_word_add = "INSERT INTO Words (word) VALUES ('" + tx.esc(newWord) + "')";
	tx.exec(str_word_add);
	tx.commit();
}

void database::link_add(const std::string newLink) {
	pqxx::work tx{ *c };
	std::string str_link_add = "INSERT INTO Links (link) VALUES ('" + tx.esc(newLink) + "')";
	tx.exec(str_link_add);
	tx.commit();
}
