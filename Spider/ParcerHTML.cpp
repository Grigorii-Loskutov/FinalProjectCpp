#include "ParcerHTML.h"

#include <vector>
#include <string>
#include <iostream>
#include <iterator>
#include <regex>

#define STRINGIZE(x) #x
#define TO_STRING(x) STRINGIZE(x)

ParcerHTML::ParcerHTML(std::vector<std::string> HTML_strings) {
	if (HTML_strings.size() == 0) { throw std::domain_error(std::string(__FILE__) + ": no strings in input vector: " + std::string(TO_STRING(HTML_strings))); }
	
	// –егул€рное выражение дл€ поиска ссылок в HTML
	std::regex LINKpattern("href=[^>]+(?=\/?>|$)");  // href= - начало, не включет >, заканчиваетс€ на /> или >

	// –егул€рное выражение дл€ поиска текста
	//std::regex WORDpattern(">([^<>]+)<");
	std::regex WORDpattern(">([^<]+)<");
	//std::regex notWORDpattern("<.*>");
	for (const auto& line : HTML_strings) {
		// »тераторы дл€ поиска совпадений
		std::sregex_iterator it_link(line.begin(), line.end(), LINKpattern);
		std::sregex_iterator end_link;
		while (it_link != end_link) {
			std::smatch match = *it_link;
			std::string match_str = match.str();
			match_str = match_str.substr(0, match_str.size() - 1);
			//std::cout << "Found link: " << match_str << std::endl;
			Links.insert(match_str);
			++it_link;
		}

		std::sregex_iterator it_word(line.begin(), line.end(), WORDpattern);
		std::sregex_iterator end_word;
		std::string temp_str = line;
		while (it_word != end_word) {
			std::smatch match = *it_word;
			std::string match_str = match.str();
			/*match_str = match_str.substr(1);
			match_str = match_str.substr(0, match_str.size() - 1);*/
			std::cout << "Found text string: " << match_str << std::endl;
			temp_str.append(match_str);
			++it_word;
		}
		if (temp_str.size() != 0) {
			Words.push_back(temp_str);
		}
	}
	
}

std::set<std::string> ParcerHTML::getLinks() {
	return Links;
}

std::vector<std::string> ParcerHTML::getWords() {
	return { "" "" };
}

