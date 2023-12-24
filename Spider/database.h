#pragma once

#include <pqxx/pqxx>
#include <string>

class database {
private:
	std::unique_ptr <pqxx::connection> connection;

private:
	database();

	void SetConnection(std::unique_ptr<pqxx::connection> in_c);

	database(const database&) = delete; // Запретим копирование

	database& operator=(const database&) = delete; // Запретим копирование

	void table_create(const std::string str_creation);



};