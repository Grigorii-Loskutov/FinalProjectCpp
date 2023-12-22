#include "ParcerHTML.h"

#include <vector>
#include <string>
#include <iostream>
#include <iterator>
#include <regex>
#include <boost/locale.hpp>
#include <boost/locale/conversion.hpp>
//#include <wchar.h>

#define STRINGIZE(x) #x
#define TO_STRING(x) STRINGIZE(x)

//ParcerHTML::ParcerHTML(std::vector<std::string> HTML_strings) {
ParcerHTML::ParcerHTML(std::string HTML_strings) {
	if (HTML_strings.size() == 0) { throw std::domain_error(std::string(__FILE__) + ": no strings in input vector: " + std::string(TO_STRING(HTML_strings))); }

	// Регулярное выражение дл¤ поиска ссылок в HTML
	//std::regex LINKpattern("<a href=[^>]+(?=\/?>|$)");  // href= - начало, не включет >, заканчиваетс¤ на /> или >
	//std::regex LINKpattern("<a href=[^>]+(?=\/?>)");
	std::regex LINKpattern("<a href=[^>]*>");
	//std::regex LINKpattern("<a href=\"(.*?)\"");

	// Регулярное выражение для поиска текста
	//std::regex WORDpattern(">([^<>]+)<");
	//std::regex WORDpattern(">([^<]+)<");
	//std::regex notWORDpattern("<.*>");
	/*std::string superString;
	for (const auto& line : HTML_strings) {
		superString.append(line);
	}*/
	//std::cout << superString;
	//for (const auto& line : HTML_strings) {
		// »тераторы дл¤ поиска совпадений
	std::sregex_iterator it_link(HTML_strings.begin(), HTML_strings.end(), LINKpattern);
	std::sregex_iterator end_link;
	while (it_link != end_link) {
		std::smatch match = *it_link;
		std::string match_str = match.str();
		match_str = match_str.substr(8);
		match_str = match_str.substr(0, match_str.size() - 1);
		//std::cout << "Found link: " << match_str << std::endl;
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
	Line = std::regex_replace(Line, pattern_numbers," ");

	// Удаление лишних пробелов
	std::regex SPACEpattern(R"(\s+)");
	Line = std::regex_replace(Line, SPACEpattern, "_");

	//// Удаление слов длиной менее 3х символов  R"(\b\w{1,3}\b)"
	//std::regex pattern_short_words(R"(_.{1,3}_)");
	//Words = std::regex_replace(Words, pattern_short_words, "_");

	// Переведем в нижний регистр

	for (char& c : Line) {
		c = std::tolower(c);
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