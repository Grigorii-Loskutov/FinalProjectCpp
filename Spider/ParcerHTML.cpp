#include "ParcerHTML.h"

#include <vector>
#include <string>
#include <iostream>
#include <iterator>
#include <regex>

#define STRINGIZE(x) #x
#define TO_STRING(x) STRINGIZE(x)

//ParcerHTML::ParcerHTML(std::vector<std::string> HTML_strings) {
ParcerHTML::ParcerHTML(std::string HTML_strings) {
	if (HTML_strings.size() == 0) { throw std::domain_error(std::string(__FILE__) + ": no strings in input vector: " + std::string(TO_STRING(HTML_strings))); }

	// –егул€рное выражение дл€ поиска ссылок в HTML
	//std::regex LINKpattern("<a href=[^>]+(?=\/?>|$)");  // href= - начало, не включет >, заканчиваетс€ на /> или >
	//std::regex LINKpattern("<a href=[^>]+(?=\/?>)");
	std::regex LINKpattern("<a href=[^>]*>");
	//std::regex LINKpattern("<a href=\"(.*?)\"");

	// –егул€рное выражение дл€ поиска текста
	//std::regex WORDpattern(">([^<>]+)<");
	std::regex WORDpattern(">([^<]+)<");
	//std::regex notWORDpattern("<.*>");
	/*std::string superString;
	for (const auto& line : HTML_strings) {
		superString.append(line);
	}*/
	//std::cout << superString;
	//for (const auto& line : HTML_strings) {
		// »тераторы дл€ поиска совпадений
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
	std::regex tagRegex(R"((<[^>]*>|<!--[^>]*-->))");
	Words = std::regex_replace(HTML_strings, tagRegex, "");
}

std::set<std::string> ParcerHTML::getLinks() {
	return Links;
}

//std::vector<std::string> ParcerHTML::getWords() {
std::string ParcerHTML::getWords() {

	return Words;
}

