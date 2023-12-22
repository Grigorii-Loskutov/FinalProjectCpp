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

	// –егул¤рное выражение дл¤ поиска ссылок в HTML
	//std::regex LINKpattern("<a href=[^>]+(?=\/?>|$)");  // href= - начало, не включет >, заканчиваетс¤ на /> или >
	//std::regex LINKpattern("<a href=[^>]+(?=\/?>)");
	std::regex LINKpattern("<a href=[^>]*>");
	//std::regex LINKpattern("<a href=\"(.*?)\"");

	// –егул¤рное выражение дл¤ поиска текста
	//std::regex WORDpattern(">([^<>]+)<");
	std::regex WORDpattern(">([^<]+)<");
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
	// ”даление html информации в <>
	std::regex tagRegex(R"((<[^>]*>|<!--[^>]*-->))");
	Words = std::regex_replace(HTML_strings, tagRegex, " ");

	// ”даление строки вида &nbsp;
	std::regex pattern_nbsp(R"((&nbsp;\s*)+)");
	Words = std::regex_replace(Words, pattern_nbsp, " ");
	
	// ”даление знаков препинани¤ и скобок
	std::regex pattern_punctuation(R"([[:punct:]()])");
	Words = std::regex_replace(Words, pattern_punctuation, " ");

	// ”даление чисел
	std::regex pattern_numbers(R"(\b\d+\b)");
	Words = std::regex_replace(Words, pattern_numbers, " ");

	// ”даление слов длиной менее 3х символов
	std::regex pattern_short_words(R"(\b\w{1,2}\b)");
	Words = std::regex_replace(Words, pattern_short_words, " ");

	// Переведем в нижний регистр

	//boost::locale::generator gen;
	//std::locale loc = gen("ru_RU.UTF-8"); //  акое-то шаманство с генератором
	//std::locale::global(loc);


	//std::wstring_convert<boost::locale::converter<wchar_t>, wchar_t> converter;
	//std::wstring wstr = converter.from_bytes(Words);


	//boost::locale::to_lower(wstr, loc);
	//boost::locale::to_lower(Words, loc);

	//Words = converter.to_bytes(wstr);
	//Words = boost::locale::to_lower(Words, loc);


}

std::set<std::string> ParcerHTML::getLinks() {
	return Links;
}

//std::vector<std::string> ParcerHTML::getWords() {
std::string ParcerHTML::getWords() {

	return Words;
}

