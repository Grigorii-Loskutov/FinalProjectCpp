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

	// Удаление знаков препинани¤ и скобок
	std::regex pattern_punctuation(R"([[:punct:]()])");
	inSeachString = std::regex_replace(inSeachString, pattern_punctuation, " ");

	// Удалени¤ кавычек ("), одинарных (') и дефисов (-)
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
	std::locale loc = gen(""); // спользуем локаль по умолчанию
	inSeachString = boost::locale::to_lower(inSeachString, loc);

	// Заполним set для хранения слов поиска
	std::set<std::string> setInWords;
	unsigned int cut_end_pos{ 0 };
	unsigned int cut_start_pos{ 0 };
	unsigned int stringLength = inSeachString.length();
	for (unsigned int iter = 0; iter < stringLength; ++iter) {
		if (inSeachString[iter] == '_') {
			cut_end_pos = iter;
			std::string word = inSeachString.substr(cut_start_pos, cut_end_pos - cut_start_pos);
			if (word.length() >= 4) {
				setInWords.insert(word);
			}
			cut_start_pos = iter + 1;
		}
		else if (iter == (stringLength - 1)) {
			cut_end_pos = stringLength;
			std::string word = inSeachString.substr(cut_start_pos, cut_end_pos - cut_start_pos);
			if (word.length() >= 4) {
				setInWords.insert(word);
			}
		}
	}
	std::cout << "\nСлова для поиска: \n";
	for (const auto& word : setInWords) {
		std::cout << word << std::endl;
	}
	// Хранение результатов запроса для каждого слова (ссылка, частота)
	std::vector <std::map<std::string, int>> resultsPerWord;

	// Вектор для хранения слов запроса, порядок слов соответсвует resultsPerWord
	// Это проще, чем городить ещё один set
	std::vector<std::string> wordsInOrder; 
															
	try {
		DB.SetConnection("localhost", "indexator", "postgres", "cfhvf810", 5432);
		for (const auto& word : setInWords)
		{
			try {
				resultsPerWord.push_back(DB.seachRequest(word));
				wordsInOrder.push_back(word);
			}
			catch (const std::exception& ex) {
				std::cout << __FILE__ << ", line: " << __LINE__ << std::endl;
				std::cout << "\n Try to find <" + word << "> in database: ";
				std::string except = ex.what();
				std::cout << "\n" << except;
			}
		}
	}
	catch (const std::exception& ex) {
		std::cout << "Try to connect to database\n";
		std::string except = ex.what();
		std::cout << "\n" << except;
	}
	std::cout << "\nРезультаты поиска для слов: \n";
	unsigned int wordIter = 0;
	for (const auto& vectors : resultsPerWord) {
		std::cout << wordsInOrder[wordIter] << ": \n";
		for (const auto& pair : vectors) {
			std::cout << pair.first << ": ";
			std::cout << pair.second << std::endl;
			seachResults.push_back(pair.first);
		}
		++wordIter;
	}
	return seachResults;
}