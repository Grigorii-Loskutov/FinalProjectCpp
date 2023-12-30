#include "finder.h"
#include <iostream>
#include <string.h>
#include <exception>
#include "../Spider/database.h"

std::vector<std::string> finder(std::string inSeachString) {
	std::vector<std::string> seachResults;

	// Создадим подключение к базе данных
	database DB;
	try {
		DB.SetConnection("localhost", "indexator", "postgres", "cfhvf810", 5432);
		try {
			seachResults = DB.seachRequest(inSeachString);
		}
		catch (const std::exception& ex) {
			std::cout << __FILE__ << ", line: " << __LINE__ << std::endl;
			std::cout << "\n Try to find <" + inSeachString << "> in database: ";
			std::string except = ex.what();
			std::cout << "\n" << except;
		}
	}
	catch (const std::exception& ex) {
		std::cout << "Try to create tables in databse\n";
		std::string except = ex.what();
		std::cout << "\n" << except;
	}
	return seachResults;
}