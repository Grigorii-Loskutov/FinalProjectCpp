#include <vector>
#include <string>
#include <iostream>
#include <iterator>
#include <regex>
#include <boost/locale.hpp>
#include <boost/locale/conversion.hpp>
#include "../Spider/database.h"

std::vector<std::string> finder(std::string inSeachString) {
	std::vector<std::string> seachResults;

	// Создадим подключение к базе данных
	database DB;

	// Удаление знаков препинания и скобок
	std::regex pattern_punctuation(R"([[:punct:]()])");
	inSeachString = std::regex_replace(inSeachString, pattern_punctuation, " ");

	// Удаления кавычек ("), одинарных (') и дефисов (-)
	std::regex pattern_remove_quotes_and_dashes(R"([\"'-])");
	inSeachString = std::regex_replace(inSeachString, pattern_remove_quotes_and_dashes, "");

	// Удаление чисел (всех слов с числами)
	std::regex pattern_numbers("\\b\\w*\\d+\\w*\\b");
	inSeachString = std::regex_replace(inSeachString, pattern_numbers, " ");

	// Удаление лишних пробелов
	std::regex SPACEpattern(R"(\s+)");
	inSeachString = std::regex_replace(inSeachString, SPACEpattern, "_");

	// Переведем в нижний регистр
	boost::locale::generator gen;
	std::locale loc = gen(""); // Используем локаль по умолчанию
	inSeachString = boost::locale::to_lower(inSeachString, loc);

	// Заполним set для хранения слов для поиска
	std::set<std::string> setInWords;
	unsigned int cut_end_pos{ 0 };
	unsigned int cut_start_pos{ 0 };
	for (unsigned int iter = 0; iter < inSeachString.length(); ++iter) {
		if (inSeachString[iter] == '_') {
			cut_end_pos = iter;
			std::string word = inSeachString.substr(cut_start_pos, cut_end_pos - cut_start_pos);
			if (word.length() >= 4) {
				setInWords.insert(word);
			}
			cut_start_pos = iter + 1;
		}
	}
	std::cout << "\nСлова для поиска: \n";
	for (const auto& word : setInWords) {
		std::cout << word << std::endl;
	}
	
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