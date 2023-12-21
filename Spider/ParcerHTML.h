#pragma once
#include <vector>
#include <string>
#include <map>
#include <set>

class ParcerHTML {
private:
	std::string Words; // ¬ектор дл€ хранени€ строк исходной страницы, очищенной от метаинформации (только текст)
	std::set<std::string> Links; // —опоставление дл€ хранени€ ссылок, найденных на странице
	std::map<std::string, int> Frequencies; // Ќабор дл€ хранени€ частот отдельных слов (пара: слово, количество в векторе)
public:
	//ParcerHTML(std::vector<std::string> HTML_strings);
	ParcerHTML(std::string HTML_strings);
	std::set<std::string> getLinks();
	std::string getWords();
};