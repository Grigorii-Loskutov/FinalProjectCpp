#include "ParcerHTML.h"

#include <vector>
#include <string>
#include <iostream>
#include <iterator>
#include <regex>
#include <boost/locale.hpp>
#include <boost/locale/conversion.hpp>


#define STRINGIZE(x) #x
#define TO_STRING(x) STRINGIZE(x)

//ParcerHTML::ParcerHTML(std::vector<std::string> HTML_strings) {
ParcerHTML::ParcerHTML(std::string HTML_strings, std::string SourceLink) {
	if (HTML_strings.size() == 0) { throw std::domain_error(std::string(__FILE__) + ": no strings in input vector: " + std::string(TO_STRING(HTML_strings))); }

	// Регулярное выражение для поиска ссылок в HTML
	std::regex LINKpattern(R"(<a href=\"[^">]*\">)");
	//std::regex LINKpattern(R"(<a href=[^>]*>)");

	// Итераторы для поиска совпадений
	std::sregex_iterator it_link(HTML_strings.begin(), HTML_strings.end(), LINKpattern);
	std::sregex_iterator end_link;
	while (it_link != end_link) {
		std::smatch match = *it_link;
		std::string match_str = match.str();
		match_str = match_str.substr(8);
		match_str = match_str.substr(0, match_str.size() - 1);
		//std::cout << "Found link: " << match_str << std::endl;

		// Сссылки внутри сайта начинаются не с "http://", а с "/" или с просто текста ссылки,
		// поэтому нужно внутренние сслыки дополнить полным адресом
		std::string const http_pref = "http://";
		if (match_str.length() >= http_pref.length() &&
			match_str.compare(0, http_pref.length(), http_pref) != 0) {
			if (match_str[0] == '/') {
				match_str = SourceLink + match_str;
			}
			else {
				match_str = SourceLink + "/" + match_str;
			}
		}
		else {
			match_str = match_str.substr(http_pref.length(), match_str.length() - 1); // Удалим префикс "http://"
		}
		if (match_str[match_str.length() - 1] == '/') {
			match_str = match_str.substr(0, match_str.length() - 2); // Удалим лишний "/" в конце
		}
		// Желательно проверить, что SourceLink не содержится на странице, чтобы бесконечно ее не добавлять
		Links.insert(match_str);
		++it_link;
	}

	//std::regex tagRegex("<[^>]*>");
	// Удаление html информации в <>
	std::regex tagRegex(R"((<[^>]*>|<!--[^>]*-->))");
	Line = std::regex_replace(HTML_strings, tagRegex, " ");

	// Удаление строки вида &nbsp;
	std::regex pattern_nbsp(R"((&nbsp;\s*)+)");
	Line = std::regex_replace(Line, pattern_nbsp, " ");

	// Удаление знаков препинания и скобок
	std::regex pattern_punctuation(R"([[:punct:]()])");
	Line = std::regex_replace(Line, pattern_punctuation, " ");

	// Удаление чисел
	std::regex pattern_numbers(R"(\b\d+\b)");
	Line = std::regex_replace(Line, pattern_numbers, " ");

	// Удаление лишних пробелов
	std::regex SPACEpattern(R"(\s+)");
	Line = std::regex_replace(Line, SPACEpattern, "_");

	// Переведем в нижний регистр
	for (char& c : Line) {
		c = std::tolower(c);
	}

	// Заполним набор для хранения частот

	unsigned int cut_end_pos{ 0 };
	unsigned int cut_start_pos{ 0 };
	for (unsigned int iter = 0; iter < Line.length(); ++iter) {
		if (Line[iter] == '_') {
			cut_end_pos = iter;
			std::string word = Line.substr(cut_start_pos, cut_end_pos - cut_start_pos);
			if (word.length() >= 4) {
				Frequencies[word]++;
			}
			cut_start_pos = iter + 1;
		}
	}
}

std::set<std::string> ParcerHTML::getLinks() {
	return Links;
}


std::string ParcerHTML::getLine() {
	return Line;
}

std::vector<std::string> ParcerHTML::getWords() {
	return Words;
}

std::map<std::string, int> ParcerHTML::getFrequencies() {
	return Frequencies;
}
